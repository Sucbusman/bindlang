#ifndef __coder__
#define __coder__
#include "value.h"
#include "version.h"

namespace bindlang::vm{
  
class Coder{
 public:
  vector<uint8_t> codes;
  vector<Value>   constants;

  void pushInst(OpCode,uint32_t);
  void pushInst(OpCode);
  vector<uint8_t> genBytecode();
  void parseBytecode(vector<uint8_t>& buffer);

  bool compareVersion(uint32_t);
  bool readBinary(const char* fname);
  bool writeBinary(const char* fname);

  void CNST(Value);
  void PUSH();
  void POP();
  void SETL(uint32_t);
  void GETL(uint32_t);
  void RET();
  void HALT();
  void SYSCALL(uint32_t);
  void JUMP(uint32_t);
private:
  uint32_t header = 0xdeadbeef;
};



}
#endif
