#include "vm/value.h"
#include "vm/vm.h"
#include "vm/coder.h"
#include <bits/stdint-uintn.h>
#include <unistd.h>
#include <cstdio>
#include <fcntl.h>

namespace bindlang { namespace vm{

using std::cout;
using std::cerr;
using std::endl;

template <typename... Args>
inline bool VM::error(Args...args){
  (cerr<< ... <<args)<<endl;
  dumpRegs();
  dumpStack();
  exit(1);
}

template <typename... Args>
inline bool VM::warning(Args...args){
  (cerr<< ... <<args)<<endl;
  return false;
}

inline bool VM::hasError(){
  return error_num>0;
}

void VM::reset(){
  ip = (this->rom).data();
  syscalls.clear();
  standardSyscalls();
  frames.clear();
  frames.push_back(callFrame(0,0,nullptr));
  bp  = 0;
  sp  = 0;
  vsp = nullptr;
  GC_THRESHOLD = 0xffffffff;//16 MB
  bytesAllocated = 0;
}

void VM::init(vector<std::uint8_t> &&codes,
              vector<Value> &&constants){
  this->rom = codes;
  this->constants=constants;
  values.clear();
  reset();
}


void VM::standardSyscalls(){
#define EXPECT(T)                               \
  if(values[sp-1].type!=VAL_##T)                \
    {cerr<<"Expect " #T<<", but get ";          \
      inspectVal(values[sp-1]);                 \
      values[sp-1]=Value(false);return false;}
  //0 print(Value)
  syscalls.push_back([this](){
    printVal(values[sp-1]);
    return true;
  });
  //1 inspect(Value)
  syscalls.push_back([this](){
    inspectVal(values[sp-1]);
    return true;
  });
  //2 garbage colllect()
  syscalls.push_back([this](){
    cout<<"[gc] bytes allocated:"<<bytesAllocated<<endl;
    recycleMem();
    return true;
  });
  /* See /usr/include/asm-generic/fcntl.h */
  //3 open(String,NUMBER,NUMBER)
  syscalls.push_back([this](){
    EXPECT(NUMBER);
    uint16_t mode = pop().as.number;
    EXPECT(NUMBER);
    int      flag = pop().as.number;
    EXPECT(String);
    auto s = AS_CSTRING(values[sp-1]);
    int fd = open(s->c_str(),flag,mode);
    if(fd<0){
      warning("can not open file ",*s," with flag ",flag);
      values[sp-1] = Value(false);
      return false;
    }
    values[sp-1] = Value((uint16_t)fd);
    return true;
  });
  //4 close(FILE)
  syscalls.push_back([this](){
    EXPECT(FILE);
    int fd = values[sp-1].as.number;
    if(close(fd)==0){
      values[sp-1] = Value(true);
      return true;
    }else{
      values[sp-1] = Value(false);
      return false;
    }
  });
  //5 read(FILE,NUMBER)
  syscalls.push_back([this](){
    EXPECT(NUMBER);
    auto n = pop().as.number;
    EXPECT(FILE);
    auto fd = values[sp-1].as.number;
    #define BUFSIZE 128
    char buf[BUFSIZE]="\0";
    if(n>0){
      read(fd,buf,n%BUFSIZE);
      string s = string(buf);
      size_t times = n/BUFSIZE;
      while(times--){
        read(fd,buf,BUFSIZE);
        s+=buf;
      }
      values[sp-1] = Value(make_obj(s));
    }else{
      values[sp-1] = Value(make_obj(""));
    }
    return true;
    #undef BUFSIZE
  });
  // 6 write(FILE,STRING)
  syscalls.push_back([this](){
    EXPECT(String);
    auto s = AS_CSTRING(pop());
    EXPECT(FILE);
    uint16_t fd = values[sp-1].as.filedes;
    int n = write(fd,s->c_str(),s->size());
    if(n<0){
      warning("write to file ",fd," failed.");
      values[sp-1] = Value(false);
      return false;
    }else{
      values[sp-1] = Value((uint64_t)n);
      return true;
    }
  });
#undef EXPECT
}

inline void VM::push(Value const& val){
  if(sp<values.size()){
    values[sp] = val;
    sp++;
  }else{
    values.push_back(val);
    sp = values.size();
  }
}

inline Value VM::pop(){
  sp--;
  return values[sp];
}


bool VM::run(){
#define EAT(T) (*(T*)ip);ip+=sizeof(T)
#define PEEK(T) (*(T*)ip)
#define bytep  ((uint8_t*)ip)
#define wordp  ((uint32_t*)ip)
#define dwordp ((uint64_t*)ip)
#define WHEN(OP) case (uint8_t)OpCode::OP
#define BINARY_OP(type,op,rst_type)            \
  do{auto r = pop();auto l = values[sp-1];     \
    rst_type ans = (l.as.type op r.as.type);   \
    values[sp-1]=Value(ans);}while(0)
#define EXPECT(T)                              \
  if(values[sp-1].type != T){error("Expect " #T);break;}
  uint8_t  byte;
  uint16_t word;
  uint32_t dword;
  uint64_t qword;
  while(true){
    if(debugp){
      disas_inst(ip);
    }
    uint8_t opc = EAT(uint8_t);
    switch(opc){
      WHEN(START):{
        entry = sp;
        prepareGc();
        break;
      }
      WHEN(NOP):break;
      WHEN(SETC):{
        byte = EAT(uint8_t);
        if(vsp){
          (*vsp)[byte] = pop();
        }else{
          error("captured values ptr is null");
        }
        break;
      }
      WHEN(GETC):{
        byte = EAT(uint8_t);
        if(vsp){
          push((*vsp)[byte]);
        }else{
          error("captured values ptr is null");
        }
        break;
      }
      WHEN(SETL):{
        word = EAT(uint16_t);
        values[bp+word] = pop();
        break;
      }
      WHEN(GETL):{
        word = EAT(uint16_t);
        push(values[bp+word]);
        break;
      }
      WHEN(SYSCALL):
        byte = EAT(uint8_t);
        if(not (syscalls[byte])()){
          return false;
        }
        break;
      WHEN(CAPTURE):{
        auto proc = AS_PROCEDURE(values[sp-1]);
        auto val_info = EAT(uint8_t);
        while(val_info != 0u){
          bool localp = val_info & (1u<<6);
          auto idx = val_info & ~(3u<<6);
          if(localp){
            proc->captureds.push_back(values[bp+idx]);
          }else{
            proc->captureds.push_back((*vsp)[idx]);
          }
          val_info = EAT(uint8_t);
        }
        break;
      }
      WHEN(CNST):{
        dword = EAT(uint32_t);
        push(constants[dword]);
        break;
      }
      WHEN(CNSH):{
        qword = EAT(uint64_t);
        push(constants[qword]);
        break;
      }
      WHEN(COPY):
        ifGc();
        values[sp-1] = copy_val(values[sp-1]);
        break;
      WHEN(IMM):{
        uint64_t n = EAT(uint64_t);
        push(Value(n));
        break;
      }
      WHEN(ADD):    BINARY_OP(number,+,uint64_t);break;
      WHEN(MINUS):  BINARY_OP(number,-,uint64_t);break;
      WHEN(MULT):   BINARY_OP(number,*,uint64_t);break;
      WHEN(DIVIDE): BINARY_OP(number,/,uint64_t);break;
      WHEN(GT):     BINARY_OP(number,>,bool);break;
      WHEN(LT):     BINARY_OP(number,<,bool);break;
      WHEN(TRUE):   push(Value(true));break;
      WHEN(FALSE):  push(Value(false));break;
      WHEN(NOT):    push(Value(truthy(pop())?false:true));break;
      WHEN(EQ):{
        auto r = pop();
        auto l = pop();
        push(Value(r == l));
        break;
      }
      WHEN(UNIT):
        ifGc();
        push(Value(make_obj(Value(false),nullptr)));
        break;
      WHEN(RCONS):{
        ifGc();
        auto l = pop();
        auto r = pop();
        push(Value(make_obj(l,AS_LIST(r))));
        break;
      }
      WHEN(CONS):{
        ifGc();
        auto r = pop();
        auto l = pop();
        push(Value(make_obj(l,AS_LIST(r))));
        break;
      }
      WHEN(HEAD):{
        auto v = values[sp-1];
        EXPECT(VAL_List);
        auto lst = AS_LIST(v);
        values[sp-1] = lst->head;
        break;
      }
      WHEN(TAIL):{
        auto v = values[sp-1];
        EXPECT(VAL_List);
        auto lst = AS_LIST(v);
        values[sp-1] = Value(lst->tail);
        break;
      }
      WHEN(EMPTYP):{
        auto v = values[sp-1];
        EXPECT(VAL_List);
        auto lst = AS_LIST(v);
        if(lst->tail){
          values[sp-1] = Value(false);
        }else{
          values[sp-1] = Value(true);
        }
        break;
      }
      WHEN(LEN):{
        EXPECT(VAL_String);
        auto s = AS_CSTRING(values[sp-1]);
        values[sp-1] = Value(s->size());
        break;
      }
      WHEN(TAKE):{//take(String,NUMBER,NUMBER)
        EXPECT(VAL_NUMBER);
        auto end = pop().as.number;
        EXPECT(VAL_NUMBER);
        auto begin = pop().as.number;
        EXPECT(VAL_String);
        auto s = AS_CSTRING(values[sp-1]);
        if(begin>s->size()){
         error("index out of range.");
         values[sp-1] = Value(false);
        }else{
          string sub = s->substr(begin,end);
          values[sp-1] = Value(make_obj(sub));
        }
        break;
      }
      WHEN(CONCAT):{
        EXPECT(VAL_String);
        auto r = AS_CSTRING(pop());
        EXPECT(VAL_String);
        auto l = AS_CSTRING(values[sp-1]);
        auto a = *l+*r;
        values[sp-1] = Value(make_obj(a));
        break;
      }
      WHEN(STR2INTS):{
        EXPECT(VAL_String);
        auto s = AS_CSTRING(values[sp-1]);
        auto unit = make_obj(Value(false),nullptr);
        ObjList* last = unit;
        for(auto it=s->rbegin();it!=s->rend();++it){
          last = make_obj(Value((*it)),last);
        }
        values[sp-1] = Value(last);
        break;
      }
      WHEN(INT2STR):{
        EXPECT(VAL_NUMBER);
        auto n = values[sp-1].as.number;
        n %= 128;
        char cstr[2]={'\0','\0'};
        cstr[0] = (char)n;
        values[sp-1] = Value(make_obj(cstr));
        break; 
      }
      WHEN(INT2FILE):{
        EXPECT(VAL_NUMBER);
        uint16_t fd = values[sp-1].as.filedes;
        values[sp-1] = Value(fd);
        break;
      }
      WHEN(PUSH):
        push(values[sp-1]);
        break;
      WHEN(POP):
        pop();
        break;
      WHEN(CALL):{
        /* caller prepare all the aruguments, 
           align in the stack in the order.
           callee is respond to prepare one return value
           in the stack,so it doesn't need to record last sp
           bettween this switch
        */
        auto func = AS_PROCEDURE(pop());
        frames.push_back(callFrame(ip,bp,vsp));
        vsp = &func->captureds;
        bp = sp-func->arity;
        ip = rom.data()+func->offset;//jump
        break;
      }
      WHEN(VCALL):{
        vframes.push_back(sp);
        break;
      }
      WHEN(MCALL):{
        auto func = AS_PROCEDURE(pop());
        auto newbase = sp-func->arity;
        vector<Value> args;
        for(auto i=newbase;i<sp;i++){
          args.push_back(values[i]);
        }
        auto id = func->hash_call(args);
        auto it = func->cache.find(id);
        if(it!=func->cache.end()){
          // has cached answer,just return
          auto cache_args = it->second.second;
          if(args == cache_args){
            //cout<<"[ find cache "<<dec<<id<<" for proc ";
            //inspectObj(func);cout<<']'<<endl;
            values[newbase] = it->second.first;
            sp = newbase+1;
            break;
          }
        }else{
          // save call infomation
          cur_proc = func;
          cur_args = args;
          cur_callid = id;
        }
        frames.push_back(callFrame(ip,bp,vsp));
        vsp = &func->captureds;
        bp = sp-func->arity;
        ip = rom.data()+func->offset;//jump
        break;
      }
      WHEN(TCALL):{
        auto func = AS_PROCEDURE(pop());
        auto base = sp-func->arity;
        // move new arguments to old call frame
        for(auto i=0;i<func->arity;i++){
          values[bp+i] = values[base+i];
        }
        sp = bp + func->arity;
        vsp = &func->captureds;
        ip = rom.data()+func->offset;//jump
        break;
      }
      WHEN(RET):{
        values[bp] = values[sp-1];//only return one value
        sp = bp+1;//point to next slot
        //restore frame
        bp = frames.back().bp;
        vsp = frames.back().vsp;
        ip = frames.back().ip;
        frames.pop_back();
        break;
      }
      WHEN(VRET):{
        auto oldsp = vframes.back();
        vframes.pop_back();
        values[oldsp] = values[sp-1];//only return one value
        sp = oldsp+1;//point to next slot
        break;
      }
      WHEN(MRET):{
        (cur_proc->cache)[cur_callid] =
          CachePair(values[sp-1],cur_args);//add ans to cache
        values[bp] = values[sp-1];//only return one value
        sp = bp+1;//point to next slot
        //restore frame
        bp = frames.back().bp;
        vsp = frames.back().vsp;
        ip = frames.back().ip;
        frames.pop_back();
        break;
      }
      WHEN(JMP):
        ip += *(int16_t*)ip-1;break;
      WHEN(JEQ):{
        if(truthy(pop())){
          ip += *(int16_t*)ip-1;
        }else {
          EAT(uint16_t);
        }
        break;
      }
      WHEN(JNE):{
        if(not truthy(pop())){
          ip += *(int16_t*)ip-1;
        }else {
          EAT(uint16_t);
        }
        break;
      }
      WHEN(HALT):{
        return true;
      }
      default:{
        cerr<<opcTable[opc]<<" not implemented in vm run!"<<endl;
        return false;
      }
    }
    if(debugp){
      dumpRegs();
      dumpStack();
    }
  }
  return false;
#undef WHEN
#undef BINARY_OP
#undef PEEK
#undef EXPECT
#undef EAT
}

} }
