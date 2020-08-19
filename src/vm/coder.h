#ifndef __coder__
#define __coder__
#include "version.h"
#include "vm/value.h"
#include <cstdint>

namespace bindlang::vm{
  
class Coder{
 public:
  vector<uint8_t> codes;
  vector<Value>   constants;

  void modify(size_t pc,uint32_t val);
  void push9(OpCode,uint64_t);
  void push5(OpCode,uint32_t);
  void push4(uint32_t);
  inline void push1(OpCode);
  vector<uint8_t> genBytecode();
  void parseBytecode(vector<uint8_t>& buffer);

  bool readBinary(const char* fname);
  bool writeBinary(const char* fname);

  size_t tellp();
  size_t tellcp();
  size_t addConst(Value);

  struct CapturedValue{
    uint32_t idx:30;
    //0:find in outer local env,1:find in outer captured values
    uint32_t iflocal:1;
    //0:end 1:use
    uint32_t flag:1;
  };

  // bytecodes
  size_t CNST(Value);
  size_t CNSH(Value);
  size_t CAPTURE(vector<CapturedValue>&);
  void CNST(uint32_t);
  void CNSH(uint64_t);
  void IMM(uint64_t);
  void PUSH();
  void POP();
  void ADD();
  void MINUS();
  void MULT();
  void DIVIDE();
  void GT();
  void LT();
  void EQ();
  void RET();
  void HALT();
  void CALL();
  void JNE(uint32_t);
  void JMP(uint32_t);
  void SETL(uint32_t);
  void GETL(uint32_t);
  void SETC(uint32_t);
  void GETC(uint32_t);
  void SYSCALL(uint32_t);
private:
  uint32_t header = 0xdeadbeef;
};



}
#endif
