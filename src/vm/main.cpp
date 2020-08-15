#include <fstream>
#include "assembler.h"
#include "vm.h"
using namespace bindlang::vm;

string prefix(string const& path){
  string pre = "";
  for(auto i=path.size()-1;i!=~0;i--){
    if(path[i] == '.'){
      for(unsigned long int j=0;j<i;j++){
        pre += path[j];
      }
    }
  }
  if(pre == "") pre = path;
  return pre;
}

string basename(string const& path){
  string rbase="";
  for(auto i = path.size()-1;i!=~0;i--){
    if(path[i] == '/'){
      break;
    }
    rbase.push_back(path[i]);
  }
  string base;
  for(auto i=rbase.size()-1;i!=~0;i--){
    base.push_back(rbase[i]);
  }
  return base;
}

string dirname(string const& path){
  string dir = "";
  for(auto i = path.size()-1;i>=0;i--){
    if(path[i] == '/'){
      for(long unsigned int j=0;j<=i;j++){
        dir += path[j];
      }
      break;
    }
  }
  return dir;
}

void scan(const char* path){
  std::ifstream ifs(path);
  if(ifs.fail()){
    std::cerr<<"open file "<<path<<" failed!"<<std::endl;
  }
  auto as = Assembler(ifs);
  as.scan();
}

void as(const char* path){
  std::ifstream ifs(path);
  if(ifs.fail()){
    std::cerr<<"open file "<<path<<" failed!"<<std::endl;
  }
  auto as = Assembler(ifs);
  string out = prefix(basename(string(path)))+".bdc";
  as.write(out.c_str());
}

void disas(const char* path){
  auto coder = Coder();
  coder.readBinary(path);
  auto vm = VM(move(coder.codes),move(coder.constants));
  vm.disassemble();
}

void run(const char* path){
  auto coder = Coder();
  coder.readBinary(path);
  auto vm = VM(move(coder.codes),move(coder.constants));
  vm.run();
}

int main(int argc,const char* argv[]){
  auto showhelp = [argv](){
    cout<<"Usage:"<<argv[0]<<" <scan|as|disas|run> <file>"<<endl;
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
    }else if(action == "disas"){
      disas(argv[2]);
    }else if(action == "scan"){
      scan(argv[2]);
    }else{
      showhelp();
      exit(1);
    }
  }
  return 0;
}
