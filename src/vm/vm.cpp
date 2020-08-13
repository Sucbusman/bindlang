#include <iostream>
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
  pc = (this->rom).data();
  syscalls.clear();
  values.clear();
  standardSyscalls();
  base_ptr = 0;
  stack_ptr = 0;
}

void VM::init(vector<std::uint8_t> &&codes,
              vector<Value> &&constants){
  this->rom = codes; this->constants=constants;
  reset();
}

void VM::standardSyscalls(){
  syscalls.push_back(sys_print);
}

void VM::push(Value val){
  if(stack_ptr<values.size()){
    values[stack_ptr] = val;
    stack_ptr++;
  }else{
    values.push_back(val);
    stack_ptr = values.size();
  }
}

Value VM::pop(){
  stack_ptr--;
  return values[stack_ptr];
}

bool VM::run(){
#define NEXT(len)                            \
  do{                                        \
    pc+=len;                                 \
    inst = PtrCast<Inst>(pc);                \
    goto *instLables[inst->opc];}while(0)
  Inst *inst;
  const void* instLables[]={VM_INSTALL_ALL(VM_EXPAND_LABEL)};
  NEXT(0);
  VM_LABEL(SETL){
    values[inst->opr] = reg_val;
    NEXT(4);
  }
  VM_LABEL(GETL){
    reg_val = values[inst->opr];
    NEXT(4);
  }
  VM_LABEL(SETG){
    envs.top().set(*AS_CSTRING(constants[inst->opr]),reg_val);
    NEXT(4);
  }
  VM_LABEL(GETG){
    auto s = AS_CSTRING(constants[inst->opr]);
    auto val = envs.top().get(*s);
    if( val == nullptr)
      return error(*s,"Not found");
    reg_val = *val;
    NEXT(4);
  }
  VM_LABEL(FUN){
    NEXT(4);
  }
  VM_LABEL(CNST){
    DEBUG("come cns");
    reg_val = constants[inst->opr];
    NEXT(4);
  }
  VM_LABEL(PUSH){
    DEBUG("come push");
    push(reg_val);
    NEXT(1);
  }
  VM_LABEL(POP){
    DEBUG("come pop");
    reg_val = pop();
    NEXT(1);
  }
  VM_LABEL(RET){
    DEBUG("come ret");
    stack_ptr = base_ptr;
    base_ptr = pop().as.address;
    pc = (uint8_t*)pop().as.address;
    NEXT(1);
  }
  VM_LABEL(HALT){
    return true;
  }
  VM_LABEL(JUMP){
    DEBUG("come jump pc "<<hex<<setw(4)<<pc-rom.data());
    pc += inst->opr;
    DEBUG("| pc "<<hex<<setw(4)<<pc-rom.data()); 
    NEXT(0);
  }
  VM_LABEL(CALL){
    DEBUG("come call");
    auto func = AS_PROCEDURE(constants[inst->opr]);
    values.push_back( Value((size_t)pc+4) );
    values.push_back( Value(base_ptr));
    pc = rom.data()+func->offset;
    NEXT(0);
  }
  VM_LABEL(TCALL){
    NEXT(4);
  }
  VM_LABEL(SYSCALL){
    auto syscall = syscalls[inst->opr];
    if(not syscall(*this))
      return error("Syscall ",inst->opr,"failed!");
    else
      NEXT(4);
  }
  return true;
}

void VM::disassemble(){
  Inst *inst;
  const void* instLables[]={VM_INSTALL_ALL(VM_EXPAND_LABEL)};
  NEXT(0);
#define BYTE4(OP)                                              \
  VM_LABEL(OP){                                                \
    cout<<setw(4)<<pc-rom.data()                               \
        <<'['<<#OP<<' '<<std::hex<<(int)inst->opr<<']'<<endl;  \
    NEXT(4);                                                   \
  }
#define BYTE1(OP)                                              \
  VM_LABEL(OP){                                                \
    cout<<setw(4)<<pc-rom.data()<<'['<<#OP<<']'<<endl;         \
    NEXT(1);                                                   \
  }
  BYTE4(SETL);
  BYTE4(GETL);
  BYTE4(SETG);
  BYTE4(GETG);
  BYTE4(FUN);
  BYTE4(CNST);
  BYTE4(CALL);
  BYTE4(TCALL);
  BYTE4(SYSCALL);
  BYTE4(JUMP);
  BYTE1(PUSH);
  BYTE1(POP);
  BYTE1(RET);
  VM_LABEL(HALT){
    cout<<"HALT"<<endl;
    return;
  }

#undef BYTE4
#undef BYTE1
}

bool sys_print(VM& vm){
  printVal(vm.reg_val);
  return true;
}

#undef NEXT
} }
