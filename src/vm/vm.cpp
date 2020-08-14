#include <bits/stdint-uintn.h>
#include <iostream>
#include "value.h"
#include "vm.h"
#include "coder.h"

namespace bindlang { namespace vm{

using std::cout;
using std::cerr;
using std::endl;

template <class... Args>
bool VM::error(Args...args){
  (cerr<< ... <<args)<<endl;
  return false;
}

void VM::reset(){
  ip = (this->rom).data();
  syscalls.clear();
  standardSyscalls();
  bp  = 0;
  sp  = 0;
}

void VM::init(vector<std::uint8_t> &&codes,
              vector<Value> &&constants){
  this->rom = codes;
  this->constants=constants;
  values.clear();
  bp = 0;
  sp = 0;
  reset();
}

void VM::standardSyscalls(){
  syscalls.push_back(sys_print);
}

void VM::push(Value val){
  if(sp<values.size()){
    values[sp] = val;
    sp++;
  }else{
    values.push_back(val);
    sp = values.size();
  }
}

Value VM::pop(){
  sp--;
  return values[sp];
}

void VM::dumpstack(){
  cout<<"===stack==="<<endl;
  for(auto it=values.rbegin();it!=values.rend();it++){
    inspectVal(*it);
  }
  cout<<"===end==="<<endl;
}

bool VM::run(){
#define EAT(T) (*(T*)ip);ip+=sizeof(T)
#define PEEK(T) (*(T*)ip)
#define bytep  ((uint8_t*)ip)
#define wordp  ((uint32_t*)ip)
#define dwordp ((uint64_t*)ip)
#define WHEN(OP) case (uint8_t)OpCode::OP
  uint8_t byte;
  uint32_t word;
  uint64_t dword;
  while(true){
    uint8_t opc = EAT(uint8_t);
    DEBUG("0x"<<left<<setw(4)
          <<hex<<ip-1-rom.data()<<" come "<<opcTable[opc]);
    switch(opc){
      WHEN(SETL):
        word = EAT(uint32_t);
        values[bp+word] = reg_val;
        break;
      WHEN(GETL):
        word = EAT(uint32_t);
        reg_val = values[bp+word];
        break;
      WHEN(SETG):
      WHEN(GETG):
        break;
      WHEN(SYSCALL):
        word = EAT(uint32_t);
        if(not syscalls[word](*this)){
          return false;
        }
        break;
      WHEN(FUN):
      WHEN(CNST):
        reg_val = constants[*wordp];
        ip+=4;
        break;
      WHEN(CNSH):
        reg_val = constants[*dwordp];
        ip+=8;
        break;
      WHEN(IMM):
        dword = EAT(uint64_t);
        reg_val = Value(dword);
        break;
      WHEN(ADD):{
        auto l = pop();auto r = pop();
        word = l.as.number+r.as.number;
        reg_val = Value(word);
        break;
      }
      WHEN(MINUS):{
        auto r = pop();auto l = pop();
        word = l.as.number-r.as.number;
        reg_val = Value(word);
        break;
      }
      WHEN(MULT):{
        auto r = pop();auto l = pop();
        word = l.as.number*r.as.number;
        reg_val = Value(word);
        break;
      }
      WHEN(DIVIDE):{
        auto r = pop();auto l = pop();
        word = l.as.number/r.as.number;
        reg_val = Value(word);
        break;
      }
      WHEN(PUSH):
        push(reg_val);
        break;
      WHEN(POP):
        reg_val = pop();
        break;
      WHEN(CALL):{
         auto func = AS_PROCEDURE(reg_val);
         uint32_t arity = *wordp;ip += 4;
         frames.push_back( callFrame{ip,bp,sp});
         bp = sp-arity;
         ip = rom.data()+func->offset;//jump
         break;
      }
      WHEN(TCALL):
      WHEN(JUMP):
        ip += *wordp;break;
      WHEN(RET):{
        sp = frames.back().sp;
        bp = frames.back().bp;
        ip = frames.back().ip;
        frames.pop_back();
        break;
      }
      WHEN(HALT):{
        return true;
      }
      default:{
        return false;
      }
    }
    //dumpstack();
  }
  return false;
}

void VM::disassemble(){
  while(ip-rom.data()<rom.size()){
    uint8_t opc = EAT(uint8_t);
    switch (opc){
      WHEN(SETL):
      WHEN(GETL):
      WHEN(SETG):
      WHEN(GETG):
      WHEN(SYSCALL):
      WHEN(FUN):
      WHEN(CALL):
      WHEN(CNST):{
        cout<<"0x"<<left<<setw(4)<<ip-1-rom.data()
            <<" ["<<opcTable[opc]<<' ';
        uint32_t n = EAT(uint32_t);
        n = n & VM_INST_IMM_MASK;
        cout<<hex<<n<<']'<<endl;
        break;
      }
      WHEN(CNSH):
      WHEN(IMM):{
        cout<<"0x"<<left<<setw(4)<<ip-1-rom.data()
            <<" ["<<opcTable[opc]<<' ';
        uint64_t n = EAT(uint64_t);
        cout<<hex<<n<<']'<<endl;
        break;
      }
      WHEN(ADD):
      WHEN(MINUS):
      WHEN(MULT):
      WHEN(DIVIDE):
      WHEN(PUSH):
      WHEN(POP):
      WHEN(TCALL):
      WHEN(HALT):
      WHEN(RET):{
        cout<<"0x"<<left<<setw(4)<<ip-1-rom.data()
            <<" ["<<opcTable[opc]<<']'<<endl;
        break;
      }
      default:{
        cout<<opcTable[opc]<<" disassemble unimplemented!"<<endl;
        break;
      }
    }
  }
#undef WHEN
}

bool sys_print(VM& vm){
  printVal(vm.reg_val);
  return true;
}

#undef NEXT
#undef EAT
} }
