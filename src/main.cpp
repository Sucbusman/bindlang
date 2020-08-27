#include <iostream>
#include <ostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include "front/scanner.h"
#include "front/parser.h"
#include "back/interpreter/interpreter.h"
#include "back/compiler/compiler.h"
#include "vm/vm.h"
#include "vm/assembler.h"

using namespace bindlang;

#define print(s) \
  do{std::cout<<s<<std::endl;}while(0)

void testScanner(Scanner & scn){
  Token tok;
  while(true){
    tok= scn.nextToken();
    if(tok.type == tok_eof){
      break;
    }
    std::cout<<tok<<std::endl;
  }
}

void testParser(Parser & parser){
  ExprPtr expr;
  while(parser.fine() and not parser.atEnd()){
    expr= parser.parseNext();
    expr->show();
    cout<<endl;
  }
}

void testInterpreter(Parser& parser,Interpreter& interp){
  ExprPtr expr;
  while(parser.fine()){
    expr = parser.parseNext();
    if(not expr or expr->type < 0) break;
    ValPtr val = interp.eval(move(expr));
    printVal(val);
  }
}

enum class Mode{ 
     tok,
     ast,
     val
};

bool preprocess(const char *buf,Mode & m){
  auto cmd = string(buf);
#define P(s) cout<<"[+] Set mode to "<<s<<endl;
  if(cmd==":tok"){
    m = Mode::tok;
    P(buf+1);
  }else if(cmd == ":ast"){
    m = Mode::ast;
    P(buf+1);
  }else if(cmd == ":val"){
    m = Mode::val;
    P(buf+1);
  }else{
    return false;
  }
  return true;
#undef P
}

void repl(){
  auto mode = Mode::val;
  std::istringstream iss;
  auto scn = Scanner(iss);
  auto parser = Parser(scn);
  auto interp = Interpreter();
  auto prompt = '>';
  char buf[200];
  cout<<prompt;
  while(cin.getline(buf,200)){
    if(preprocess(buf,mode)){
      memset(buf,0,200);
      goto NEXT;
    }
    iss.str(buf);
    iss.clear();
    parser.reset();
    interp.reset();
    memset(buf,0,200);
    switch(mode){
      case Mode::tok:
        testScanner(scn);
        break;
      case Mode::ast:
        testParser(parser);
        break;
      case Mode::val:
        testInterpreter(parser, interp);
        break;
    }
  NEXT:
    cout<<endl;
    cout<<prompt;
  }
  std::cout<<"bye~"<<std::endl;
}

void interpFile(string const& fn){
  auto interp = Interpreter();
  interp.runFile(fn);
}

void compileFile(string const& fn){
  auto compiler = Compiler();
  compiler.compileFile(fn);
}

void compileAndRun(string const& fn,bool debugp){
  auto compiler = Compiler();
  compiler.compileFile2mem(fn);
  if(not compiler.hasError()){
    auto vm = vm::VM(move(compiler.coder.codes),
                     move(compiler.coder.constants),
                     move(compiler.coder.lines));
    vm.debugp = debugp;
    vm.run();
  }
}

void scan(const char* path){
  std::ifstream ifs(path);
  if(ifs.fail()){
    std::cerr<<"open file "<<path<<" failed!"<<std::endl;
  }
  auto as = vm::Assembler(ifs);
  as.scan();
}

void as(const char* path){
  std::ifstream ifs(path);
  if(ifs.fail()){
    std::cerr<<"open file "<<path<<" failed!"<<std::endl;
  }
  auto as = vm::Assembler(ifs);
  string out = dirname(path)+prefix(basename(string(path)))+".bdc";
  as.write(out.c_str());
}

void disas(const char* path){
  auto coder = vm::Coder();
  coder.readBinary(path);
  auto vm = vm::VM(move(coder.codes),move(coder.constants),
                   move(coder.lines));
  vm.disassemble();
}

void runBytecode(const char* path,bool debugp){
  auto coder = vm::Coder();
  coder.readBinary(path);
  auto vm = vm::VM(move(coder.codes),move(coder.constants),
                   move(coder.lines));
  vm.debugp = debugp;
  vm.run();
}

int main(int argc,char *argv[]){
  auto showhelp = [argv](){
    cout<<"Usage:"<<argv[0]<<
      " [<tok|interp|compile|scan|as|disas|run|debug> <file>]"
        <<endl
        <<"default:zero arg:repl"
        <<"one arg:compile"<<endl;
  };
  if(argc<2){
    repl();
  }else if(argc<3){
    auto fn = string(argv[1]);
    if (fn.size()>4 and fn[fn.size()-1] == 'c'){
      runBytecode(argv[1],false);
    }else {
      compileAndRun(fn,false);
    }
  }else if(argc<4){
    auto action = string(argv[1]);
#define when(S) if(action == (S))
    when("run"){
      runBytecode(argv[2],false);
    }else when("tok"){
      fstream fs(argv[2]);
      auto scn = Scanner(fs);
      scn.reset();
      testScanner(scn);
    }else when("as"){
      as(argv[2]);
    }else when("disas"){
      disas(argv[2]);
    }else when("scan"){
      scan(argv[2]);
    }else when("compile"){
      compileFile(string(argv[2]));
    }else when("interp"){
      interpFile(argv[2]);
    }else when("debug"){
      auto fn = string(argv[2]);
      if(fn.size()>4 and fn[fn.size()-1] == 'c'){
        runBytecode(argv[2], true);
      }else{
        compileAndRun(fn, true);
      }
    }else{
      showhelp();
      exit(1);
    }
  }else{
    showhelp();
    exit(1);
  }
#undef when
  return 0;
}
