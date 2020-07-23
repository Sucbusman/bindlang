#include <iostream>
#include <sstream>
#include "scanner.h"
#include "parser.h"
using namespace bindlang;
#define print(s) \
  do{std::cout<<s<<std::endl;}while(0)

void printAst(ExprPtr& expr){
  switch(expr->type){
    case ATOM:{
      print("Atom");
      Expr* p = expr.release();
      auto atom = dynamic_cast<ExprAtom*>(p);
      print(atom->literal);
      break;
    }
    case ID:
    case DEFINE:
    case FUNC:
    case CALL:
    case -2:
      expr->show();
      break;
    default:
      std::cerr<<"Unkown type with integer:"<<expr->type<<std::endl;
      break;
  }
}

void testScanner(Scanner & scn){
  while(true){
    Token tok = scn.nextToken();
    if(tok.type == tok_eof){
      std::cout<<"bye~"<<std::endl;
      break;
    }
  std::cout<<tok<<std::endl;
  } 
}

void testParser(Parser & parser){
  while(true){
    ExprPtr expr = parser.parseNext();
    printAst(expr);
    if(parser.atEnd()){
      break;
    }
  }
}

int main(){
  auto scn = Scanner(std::cin);
  auto parser = Parser(scn);
  //testScanner(scn);
  testParser(parser);
  return 0;
}
