#include <fstream>
#include "assembler.h"
#include "vm.h"
using namespace bindlang::vm;

void as(const char* path){
  std::ifstream ifs(path);
  if(ifs.fail()){
    std::cerr<<"open file "<<path<<" failed!"<<std::endl;
  }
  auto as = Assembler(ifs);
  as.run();
}

void run(const char* path){
  auto coder = Coder();
  coder.readBinary(path);
  auto vm = VM(move(coder.codes),move(coder.constants));
  vm.disassemble();
  vm.reset();
  vm.run();
}

int main(int argc,const char* argv[]){
  auto showhelp = [argv](){
    cout<<"Usage:"<<argv[0]<<" <run|as> <file>"<<endl;
  };
  if(argc<3){
    showhelp();
    exit(1);
  }else{
    auto action = string(argv[1]);
    if(action == "run" ){
      run(argv[2]);
    }else if(action == "as"){
      as(argv[2]);
    }else{
      showhelp();
      exit(1);
    }
  }
  return 0;
}
