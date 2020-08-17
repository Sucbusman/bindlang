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
        uint32_t arity = READ(uint32_t);
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
        content.write((char*)&proc->arity,4);
        content.write((char*)&proc->offset,sizeof(size_t));
        len+=4+sizeof(size_t);
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
  if(ifs.fail() or not compareVersion(check)){
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

bool Coder::compareVersion(uint32_t check){
  uint32_t ver_info = ((1 & 0xfff)<<20)
                     |((1 & 0xff) <<12)
                     |( 1 & 0xfff);
  return check == ver_info;
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

// bytecodes
void Coder::insert(size_t pc,uint32_t opr){
  uint8_t *bytes = PtrCast<uint8_t>(&opr);
  codes[pc+0] = bytes[0];
  codes[pc+1] = bytes[1];
  codes[pc+2] = bytes[2];
  codes[pc+3] = bytes[3];
}

void Coder::push5(OpCode opc,uint32_t opr){
  codes.push_back((uint8_t)opc);
  uint8_t *bytes = PtrCast<uint8_t>(&opr);
  codes.push_back(bytes[0]);
  codes.push_back(bytes[1]);
  codes.push_back(bytes[2]);
  codes.push_back(bytes[3]);
}

void Coder::push9(OpCode opc,uint64_t opr){
  codes.push_back((uint8_t)opc);
  uint8_t *bytes = PtrCast<uint8_t>(&opr);
  for(auto i=0;i<8;i++){
    codes.push_back(bytes[i]);
  }
}

inline void Coder::push1(OpCode opc){
  codes.push_back((uint8_t)opc);
}

size_t Coder::tellp(){
  return codes.size();
}

size_t Coder::CNST(Value v){
  auto idx = (uint32_t)(constants.size());
  push5(OpCode::CNST,idx);
  constants.push_back(v);
  return idx;
}

size_t Coder::CNSH(Value v){
  auto idx = constants.size();
  push9(OpCode::CNST,idx);
  constants.push_back(v);
  return idx;
}


#define INST1(F) \
  void Coder::F(){                              \
    push1(OpCode::F);                        \
  }
#define INST5(F)\
  void Coder::F(uint32_t idx){                              \
    push5(OpCode::F,idx);                              \
  }
#define INST9(F) \
  void Coder::F(uint64_t n){ \
    push9(OpCode::F,n);   \
  }

INST1(HALT);
INST1(RET);
INST1(PUSH);
INST1(POP);
INST1(ADD);
INST1(MINUS);
INST1(MULT);
INST1(DIVIDE);
INST1(EQ);
INST1(GT);
INST1(LT);
INST1(CALL);
INST5(JNE);
INST5(SYSCALL);
INST5(JMP);
INST5(GETL);
INST5(FUN);
INST5(SETL);
INST5(CNST);
INST9(IMM);
INST9(CNSH);

}
