#include "disassembler.h"
namespace bindlang::vm{

void Disassembler::run(){
#define NEXT(len)                            \
  do{                                        \
    pc+=len;                                 \
    inst = PtrCast<Inst>(pc);                \
    goto *instLables[inst->opc];}while(0)
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
  BYTE1(RET);
  BYTE1(JUMP);

#undef BYTE4
#undef BYTE1
#undef NEXT
}

}
