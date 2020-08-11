#include "coder.h"
#include "value.h"
#include <bits/stdint-uintn.h>
namespace bindlang::vm{

void Coder::pushInst(OpCode opc,uint32_t opr){
  Inst inst = {static_cast<uint32_t>(opc),opr};
  uint8_t *bytes = PtrCast<uint8_t>(&inst);
  codes.push_back(bytes[0]);
  codes.push_back(bytes[1]);
  codes.push_back(bytes[2]);
  codes.push_back(bytes[3]);
}

void Coder::pushInst(OpCode opc){
  uint8_t *bytes = PtrCast<uint8_t>(&opc);
  codes.push_back(bytes[0]);
}

void Coder::CNST(Value v){
  constants.push_back(v);
  pushInst(OpCode::CNST,constants.size()-1);
}

void Coder::PUSH(){
  pushInst(OpCode::PUSH);
}

void Coder::POP(){
  pushInst(OpCode::POP);
}

void Coder::SETL(uint32_t idx){
  pushInst(OpCode::SETL,idx);
}

void Coder::GETL(uint32_t idx){
  pushInst(OpCode::GETL,idx);
}

void Coder::RET(){
  pushInst(OpCode::RET);
}

void Coder::PRINT(){
  pushInst(OpCode::PRINT);
}

void Coder::JUMP(uint32_t offset){
  pushInst(OpCode::JUMP,offset);
}

}
