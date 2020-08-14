#include "coder.h"
#include "vm.h"
using namespace bindlang::vm;

int main(){
  auto coder = Coder();
  // coder.IMM(1u);
  // coder.PUSH();
  // coder.SYSCALL(0);
  // coder.IMM(2u);
  // coder.SETL(0);
  // coder.IMM(3u);
  // coder.GETL(0);
  // coder.SYSCALL(0);
  // coder.HALT();
  //_start:
  auto _start = coder.tellp();
  coder.CNST(Value(make_obj("inc",1,0)));
  coder.PUSH();
  coder.IMM(0u);
  coder.PUSH();
  auto  b1 = coder.tellp();
  coder.GETL(1);
  coder.PUSH();
  coder.GETL(0);
  coder.CALL();
  coder.POP();
  coder.GETL(1);
  coder.PUSH();
  coder.IMM(6u);
  coder.PUSH();
  coder.EQ();
  coder.JNE((uint32_t)(int64_t)(b1-coder.tellp()));
  coder.HALT();
  //inc:
  auto inc = coder.tellp();
  coder.CNST(Value(1u));
  coder.PUSH();
  coder.ADD();
  coder.SETL(-1);
  coder.SYSCALL(0);//print
  coder.RET();

  // patch func offset
  coder.constants[0] = Value(make_obj("inc",1,inc-_start));

  //coder.writeBinary("bin");
  //coder.readBinary("bin");
  auto vm = VM(move(coder.codes),move(coder.constants));
  cout<<"===dissameble==="<<endl;
  vm.disassemble();
  cout<<"===run==="<<endl;
  vm.reset();
  vm.run();
  return 0;
}
