#include "coder.h"
#include "vm.h"
using namespace bindlang::vm;

int main(){
  auto coder = Coder();
  auto vm = VM();
  coder.CNST(Value(1u));
  coder.PUSH();
  coder.SYSCALL(0);
  coder.CNST(Value(2u));
  coder.PUSH();
  coder.SETL(0);//1=>2
  coder.POP();
  coder.POP();
  coder.JUMP(12);
  coder.CNST(Value(make_obj("haha")));
  coder.SYSCALL(0);
  coder.HALT();
  /*
  coder.CNST(Value(2333u));
  coder.SYSCALL(0);
  coder.HALT();
  */
  coder.writeBinary("bin");
  coder.readBinary("bin");
  vm.init(move(coder.codes),move(coder.constants));
  cout<<"vm run"<<endl;
  vm.disassemble();
  vm.reset();
  vm.run();
  return 0;
}
