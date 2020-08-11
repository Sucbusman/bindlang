#include "coder.h"
#include "vm.h"
using namespace bindlang::vm;

int main(){
  auto coder = Coder();
  auto vm = VM();
  coder.CNST(Value(1u));
  coder.PUSH();
  coder.PRINT();
  coder.CNST(Value(2u));
  coder.PUSH();
  coder.SETL(0);//1=>2
  coder.POP();
  coder.POP();
  //coder.JUMP(9);
  coder.CNST(Value(vm.make_obj("haha")));
  coder.PRINT();
  coder.PUSH();
  coder.RET();
  coder.RET();
  //auto disassember = Disassembler(coder.codes,coder.constants);
  vm.init(move(coder.codes),move(coder.constants));
  //disassember.run();
  cout<<"vm run"<<endl;
  vm.disassemble();
  vm.reset();
  vm.run();
  return 0;
}
