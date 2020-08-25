#ifndef __coder__
#define __coder__
#include "version.h"
#include "vm/value.h"
#include <bits/stdint-uintn.h>
#include <cstdint>

namespace bindlang::vm{
  
class Coder{
 public:
  vector<uint8_t> codes;
  vector<Value>   constants;

  template <typename T>
  void modify(size_t pc,T val);
  void modify16(size_t pc,uint16_t);

  template <typename T>
  void pushb(T opr);

  template <typename T>
  void pushi(OpCode opc,T opr);
  
  vector<uint8_t> genBytecode();
  void parseBytecode(vector<uint8_t>& buffer);

  bool readBinary(const char* fname);
  bool writeBinary(const char* fname);

  // helper
  size_t tellp();
  size_t tellcp();
  size_t addConst(Value);

  struct CapturedValue{
    uint8_t idx:6;
    //0:find in outer local env,1:find in outer captured values
    uint8_t iflocal:1;
    //0:end 1:use
    uint8_t flag:1;
  };
  // bytecodes
  size_t CNST(Value);
  size_t CNSH(Value);
  size_t CAPTURE(vector<CapturedValue>&);
  void CNST(uint32_t);
  void CNSH(uint64_t);
  void IMM(uint64_t);
  void START();
  void NOP();
  void PUSH();
  void POP();
  void ADD();
  void MINUS();
  void MULT();
  void DIVIDE();
  void GT();
  void LT();
  void EQ();
  void NOT();
  void TRUE();
  void FALSE();
  void UNIT();
  void RCONS();
  void CONS();
  void HEAD();
  void TAIL();
  void EMPTYP();
  void LEN();
  void RET();
  void HALT();
  void CALL();
  void VCALL();
  void VRET();
  void TCALL();
  void COPY();
  void JNE(uint16_t);
  void JEQ(uint16_t);
  void JMP(uint16_t);
  void SETL(uint16_t);
  void GETL(uint16_t);
  void SETC(uint8_t);
  void GETC(uint8_t);
  void SYSCALL(uint8_t);
private:
  uint32_t header = 0xdeadbeef;
  OpCode last_op = OpCode::NOP;
};



}
#endif
