#ifndef __ast__
#define __ast__

#include <memory>
#include <vector>
#include <string>
#include <iomanip>

#include "token.h"
#include "value.h"
#include "utils.h"
namespace bindlang{

typedef enum{
  ATOM,
  ID,
  DEFINE,
  FUNC,
  CALL
} expr_type;


struct Expr {
  int type=-1;
  ~Expr() = default;
  virtual void show()=0;
  Expr(int type):type(type){};
};

using ExprPtr = std::unique_ptr<Expr>;
using ExprPtrList = std::vector<ExprPtr>;
using TokenList = std::vector<Token>;

struct ExprEof : Expr{
  ExprEof():Expr(-2){}
  void show(){
    std::cout<<"<Expr EOF >"<<std::endl;
  }
};

struct ExprAtom : Expr{
  Token literal;
  
  ExprAtom(Token literal)
    : Expr(ATOM),literal(literal){}
  void show(){
    std::cout<<BLUE("<Expr ")<<"Atom "<<std::endl;
    std::cout<<std::setw(10)<<std::left<<"  Token:"<<std::endl;
    cout<<std::setw(14)<<' ';literal.show();std::cout<<endl;
    std::cout<<BLUE(" >")<<std::endl;
  }
};

struct ExprId : Expr{
  Token id;
  
  ExprId(Token id)
    : Expr(ID),id(id){}
  void show(){
    std::cout<<BLUE("<Expr ")<<"Id "<<std::endl;
    std::cout<<std::setw(10)<<std::left<<"  Token:"<<std::endl;
    cout<<std::setw(14)<<' ';id.show();std::cout<<endl;
    std::cout<<BLUE(" >")<<std::endl;
  }
};

struct ExprDefine : Expr{
  Token id;
  ExprPtr expr;
  
  ExprDefine(Token id,ExprPtr expr)
    : Expr(DEFINE),id(id),expr(std::move(expr)){}
  void show(){
    std::cout<<BLUE("<Expr ")<<"Define "<<std::endl;
    std::cout<<std::setw(10)<<std::left<<"  Token:"<<std::endl;
    cout<<std::setw(14)<<' ';id.show();std::cout<<endl;
    std::cout<<std::setw(10)<<std::left<<"  ExprPtr:"<<std::endl;
    cout<<std::setw(14)<<' ';expr->show();std::cout<<endl;
    std::cout<<BLUE(" >")<<std::endl;
  }
};

struct ExprFunc : Expr{
  TokenList params;
  ExprPtr body;
  
  ExprFunc(TokenList params,ExprPtr body)
    : Expr(FUNC),params(std::move(params)),body(std::move(body)){}
  void show(){
    std::cout<<BLUE("<Expr ")<<"Func "<<std::endl;
    std::cout<<std::setw(10)<<std::left<<"  TokenList:"<<std::endl;
    for(auto &i:params){
      std::cout<<std::setw(14)<<" ";i.show();std::cout<<endl;
    }
    std::cout<<std::setw(10)<<std::left<<"  ExprPtr:"<<std::endl;
    cout<<std::setw(14)<<' ';body->show();std::cout<<endl;
    std::cout<<BLUE(" >")<<std::endl;
  }
};

struct ExprCall : Expr{
  Token id;
  ExprPtrList args;
  
  ExprCall(Token id,ExprPtrList args)
    : Expr(CALL),id(id),args(std::move(args)){}
  void show(){
    std::cout<<BLUE("<Expr ")<<"Call "<<std::endl;
    std::cout<<std::setw(10)<<std::left<<"  Token:"<<std::endl;
    cout<<std::setw(14)<<' ';id.show();std::cout<<endl;
    std::cout<<std::setw(10)<<std::left<<"  ExprPtrList:"<<std::endl;
    for(auto &i:args){
      std::cout<<std::setw(14)<<" ";i->show();std::cout<<endl;
    }
    std::cout<<BLUE(" >")<<std::endl;
  }
};

};
#endif
