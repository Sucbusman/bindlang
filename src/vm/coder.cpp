#include <cstdio>
#include <fstream>
#include <iterator>
#include <memory>
#include <sstream>
#include "vm/coder.h"
#include "vm/value.h"

namespace bindlang::vm{

void Coder::parseBytecode(vector<uint8_t>& buffer){
#define READ(T) (*(T*)(buffer.data()+pos));pos+=sizeof(T)
#define PUSHV(EXP) constants.push_back(EXP)
  codes.clear();
  constants.clear();
  uint32_t len = 0;
  size_t pos = 0;
  // constants
  len = READ(uint32_t);
  len += 4;// len itself ocupy 4 bytes
  //cout<<"len plus 4:0x"<<hex<<len<<endl;
  while(pos < len){
    uint8_t byte = READ(uint8_t);
    //cout<<"pos:0x"<<(int)pos<<" type:"<<(int)byte<<endl;
    switch(byte){
      case VAL_NIL:
        PUSHV(Value());
        break;
      case VAL_BOOL:{
        bool b = READ(uint8_t);
        PUSHV(Value(b));
        break;
      }
      case VAL_NUMBER:{
        uint32_t n = READ(uint32_t);
        PUSHV(Value(n));
        break;
      }
      case VAL_String:{
        string  s;
        uint8_t b;
        while(true){
          b = READ(uint8_t);
          if(b == 0) break;
          s.push_back(b);
        }
        PUSHV(Value(make_obj(s)));
        break;
      }
      case VAL_Procedure:{
        uint8_t arity = READ(uint8_t);
        size_t offset = READ(size_t);
        PUSHV(Value(make_obj("anony",arity,offset)));
        break;
      }
      default:
        cerr<<"Unreachable!"<<endl;
        exit(1);
        break;
    }
  }

  // instructions
  while(pos<buffer.size()){
    codes.push_back(buffer[pos]);
    pos++;
  }
#undef PUSHV
#undef READ
}

vector<uint8_t> Coder::genBytecode(){
  std::ostringstream content;
  // constants
  uint32_t len = 0;
  auto len_pos = content.tellp();
  content.write(PtrCast<char>(&len),
                sizeof(len));
  for(auto const& val:constants){
    content.write((char*)&val.type,1);
    len+=1;
    switch(val.type){
      case VAL_NIL:
        break;
      case VAL_BOOL:
        content.write((char*)&val.as.boolean,1);
        len+=1;
        break;
      case VAL_NUMBER:
        content.write((char*)&val.as.number,4);
        len+=4;
        break;
      case VAL_String:{
        string *s = AS_CSTRING(val);
        content.write(s->c_str(),(*s).size()+1);
        len+=(*s).size()+1;
        break;
      }
      case VAL_Procedure:{
        auto proc = AS_PROCEDURE(val);
        content.write((char*)&proc->arity,1);
        content.write((char*)&proc->offset,sizeof(size_t));
        len+=1+sizeof(size_t);
        break;
      }
      default:
        cerr<<"Unreachable!"<<endl;
        exit(1);
        break;
    }
  }
  content.seekp(len_pos);
  content.write((char*)&len,sizeof(len));
  content.seekp(0,std::ios::end);

  // instructions
  content.write((char*)codes.data(),codes.size());
  auto s = content.str();
  return vector<uint8_t>(s.begin(),s.end());
}

bool Coder::readBinary(const char* fname){
  std::ifstream ifs(fname,std::ios::binary);
  if(ifs.fail()){
    cerr<<"Can not open file "<<fname<<endl;
    return false;
  }
  vector<uint8_t> buffer;
  uint32_t check;
  ifs.read((char*)&check,4);
  //cout<<"version:"<<hex<<check<<endl;
  if(ifs.fail() or check != header){
    cerr<<"Not a bindlang bytecode file!"<<hex<<check<<endl;
    return false;
  }
  ifs.read((char*)&check,4);
  if(ifs.fail() ){//TODO:compare version
    cerr<<"Different version with vm "<<hex<<check<<endl;
    return false;
  }
  while(true){
    uint8_t b = ifs.get();
    if(ifs.eof()) break;
    buffer.push_back(b);
  }
  parseBytecode(buffer);
  return true;
}

bool Coder::writeBinary(const char* fname){
  auto content = genBytecode();
  std::ofstream ofs(fname,std::ios::binary);
  if(ofs.fail()){
    cerr<<"Can not open file "<<fname<<endl;
    return false;
  }
  ofs.write(PtrCast<char>(&header),sizeof(header));
  uint32_t ver_info = ((1 & 0xfff)<<20)
                     |((1 & 0xff) <<12)
                     |( 1 & 0xfff);
  ofs.write(PtrCast<char>(&ver_info),sizeof(ver_info));
  ofs.write((char*)content.data(),content.size());
  return true;
}

// constants
size_t Coder::tellcp(){
  return constants.size();
}

size_t Coder::addConst(Value v){
  auto idx = constants.size();
  constants.push_back(v);
  return idx;
}

void Coder::addLine(size_t line_num){
  lines.push_back(pair(line_num,tellp()));
}

// bytecodes
template <typename T>
void Coder::modify(size_t pc,T bs){
  uint8_t *bytes = PtrCast<uint8_t>(&bs);
  int nbytes = sizeof(T);
  for(int i=0;i<nbytes;i++){
    codes[pc+i] = bytes[i];
  }
}

void Coder::modify16(size_t pc,uint16_t bs){
  modify(pc,(uint16_t)bs);
}

template <typename T>
void Coder::pushb(T bs){
  uint8_t *bytes = PtrCast<uint8_t>(&bs);
  int nbytes = sizeof(T);
  for(int i=0;i<nbytes;i++){
    codes.push_back(bytes[i]);
  }
}

template <typename T>
void Coder::pushi(OpCode opc,T opr){
  codes.push_back((uint8_t)opc);
  pushb(opr);
}

size_t Coder::tellp(){
  return codes.size();
}

size_t Coder::CNST(Value v){
  auto idx = (uint32_t)(constants.size());
  pushi(OpCode::CNST,idx);
  constants.push_back(v);
  return idx;
}

size_t Coder::CNSH(Value v){
  auto idx = constants.size();
  pushi(OpCode::CNST,idx);
  constants.push_back(v);
  return idx;
}

size_t Coder::CAPTURE(vector<CapturedValue>& value_infos){
  pushb((uint8_t)OpCode::CAPTURE);
  for(auto & info:value_infos){
    pushb(*reinterpret_cast<uint8_t*>(&info));
  }
  pushb((uint8_t)0);//indicate captured value encode end
  return tellp();
}

#define INST1(F)                                 \
  void Coder::F(){                               \
    pushb((uint8_t)OpCode::F);                   \
    last_op = OpCode::F;                         \
  }
#define INST2(F)                                 \
  void Coder::F(uint8_t n){                      \
    pushi(OpCode::F,n);                          \
    last_op = OpCode::F;                         \
  }
#define INST3(F)                                 \
  void Coder::F(uint16_t n){                     \
    pushi(OpCode::F,n);                          \
    last_op = OpCode::F;                         \
  }
#define INST5(F)                                 \
  void Coder::F(uint32_t idx){                   \
    pushi(OpCode::F,idx);                        \
    last_op = OpCode::F;                         \
  }
#define INST9(F)                                 \
  void Coder::F(uint64_t n){                     \
    pushi(OpCode::F,n);                          \
    last_op = OpCode::F;                         \
  }

void Coder::CALL(){
  pushb((uint8_t)OpCode::CALL);
  last_op = OpCode::CALL;
}

void Coder::RET(){
  if(last_op == OpCode::CALL){
    modify(codes.size()-1,(uint8_t)OpCode::TCALL);
    pushb((uint8_t)OpCode::RET);
  }else{
    pushb((uint8_t)OpCode::RET);
  }
  last_op = OpCode::RET;
}

INST1(START);
INST1(NOP);
INST1(HALT);
INST1(PUSH);
INST1(POP);
INST1(VCALL);
INST1(TCALL);
INST1(VRET);

// numeric
INST1(ADD);
INST1(MINUS);
INST1(MULT);
INST1(DIVIDE);
INST1(ADDN);
INST1(MINUSN);
INST1(MULTN);
INST1(DIVIDEN);

// boolean
INST1(EQ);
INST1(GT);
INST1(LT);
INST1(TRUE);
INST1(FALSE);
INST1(NOT);
// list
INST1(UNIT);
INST1(RCONS);
INST1(CONS);
INST1(HEAD);
INST1(TAIL);
INST1(EMPTYP);
// string
INST1(LEN);
INST1(TAKE);
INST1(CONCAT);

//convert
INST1(STR2INTS);
INST1(INT2STR);
INST1(INT2FILE);

INST1(COPY);
INST2(SYSCALL);
INST2(GETC);
INST2(SETC);
INST3(JNE);
INST3(JEQ);
INST3(JMP);
INST3(GETL);
INST3(SETL);
INST5(CNST);
INST9(IMM);
INST9(CNSH);

}
