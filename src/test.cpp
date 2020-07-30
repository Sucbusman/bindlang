#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include "scanner.h"
#include "parser.h"
#include "interpreter.h"

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

void runFile(string const& fn){
  auto ifs = ifstream(fn);
  auto scn = Scanner(ifs);
  auto parser = Parser(scn);
  auto interp = Interpreter();
  parser.reset();
  interp.reset();
  while(not parser.atEnd()){
    auto expr = parser.parseNext();
    interp.eval(move(expr));
  }
}

int main(int argc,char *argv[]){
  if(argc>=2)
    runFile(argv[1]);
  else 
    repl();
  return 0;
}
