#ifndef __coder__
#define __coder__
#include "value.h"

namespace bindlang::vm{
  
class Coder{
 public:
  vector<uint8_t> codes;
  vector<Value>   constants;

  void pushInst(OpCode,uint32_t);
  void pushInst(OpCode);

  void CNST(Value);
  void PUSH();
  void POP();
  void SETL(uint32_t);
  void GETL(uint32_t);
  void RET();
  void PRINT();
  void JUMP(uint32_t);
};



}
#endif
