#include "coder.h"
#include "vm.h"
using namespace bindlang::vm;

int main(){
  auto coder = Coder();
  //_start:
  auto _start = coder.tellp();
  coder.CNST(Value(make_obj("inc",1,0)));
  coder.PUSH();
  coder.IMM(2u);
  coder.PUSH();
  coder.GETL(0);
  coder.CALL(1);
  coder.HALT();
  //inc:
  auto inc = coder.tellp();
  coder.CNST(Value(1u));
  coder.PUSH();
  coder.ADD();
  coder.SYSCALL(0);
  coder.RET();

  // patch func offset
  cout<<"inc offset when init:"<<inc-_start<<endl;
  coder.constants[0] = Value(make_obj("inc",1,inc-_start));

  coder.writeBinary("bin");
  coder.readBinary("bin");
  auto vm = VM(move(coder.codes),move(coder.constants));
  cout<<"===dissameble==="<<endl;
  vm.disassemble();
  //for(auto i:vm.constants){
  //  printVal(i);
  //}
  cout<<"===run==="<<endl;
  vm.reset();
  vm.run();
  return 0;
}
