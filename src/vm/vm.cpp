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
}

void VM::init(vector<std::uint8_t>&&codes,vector<Value>&&constants){
  this->rom = codes; this->constants=constants;
  reset();
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
    globals[*AS_STRING(constants[inst->opr])] = reg_val;
    NEXT(4);
  }
  VM_LABEL(GETG){
    auto s = AS_STRING(constants[inst->opr]);
    auto it = globals.find(*s);
    if( it == globals.end())
      return error(*s,"Not found");
    reg_val = it->second;
    NEXT(4);
  }
  VM_LABEL(FUN){
    
  }
  VM_LABEL(CNST){
    DEBUG("come cns");
    reg_val = constants[inst->opr];
    NEXT(4);
  }
  VM_LABEL(PUSH){
    DEBUG("come push");
    values.push_back(reg_val);
    NEXT(1);
  }
  VM_LABEL(POP){
    if(values.empty()) return error("pop from empty stack");
    reg_val = values.back();
    values.pop_back();
    NEXT(1);
  }
  VM_LABEL(PRINT){
    printVal(reg_val);
    NEXT(1);
  }
  VM_LABEL(RET){
    DEBUG("come ret");
    printVal(reg_val);
    return true;
  }
  VM_LABEL(JUMP){
    DEBUG("come jump pc "<<pc-rom.data());
    pc += inst->opr;
    DEBUG("| pc "<<pc-rom.data());
    NEXT(0);
  }
  return true;
}

void VM::disassemble(){
  Inst *inst;
  const void* instLables[]={VM_INSTALL_ALL(VM_EXPAND_LABEL)};
  NEXT(0);
#define BYTE4(OP)                                              \
  VM_LABEL(OP){                                                \
    cout<<'['<<#OP<<' '<<std::hex<<(int)inst->opr<<']'<<endl;  \
    NEXT(4);                                                   \
  }
#define BYTE1(OP)                                              \
  VM_LABEL(OP){                                                \
    cout<<'['<<#OP<<']'<<endl;  \
    NEXT(1);                                                   \
  }
  BYTE4(SETL);
  BYTE4(GETL);
  BYTE4(SETG);
  BYTE4(GETG);
  BYTE4(FUN);
  BYTE4(CNST);
  BYTE1(PUSH);
  BYTE1(POP);
  BYTE1(PRINT);
  VM_LABEL(RET){
    cout<<"RET"<<endl;
    return;
  }
  BYTE1(JUMP);

#undef BYTE4
#undef BYTE1
}

#undef NEXT
} }
