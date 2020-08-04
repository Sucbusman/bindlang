#ifndef __ast__
#define __ast__

#include <memory>
#include <vector>
#include <string>
#include <iomanip>

#include "token.h"
#include "utils.h"
#include "type.h"

namespace bindlang{
using std::cout;
using std::end;
using std::setw;
using TokenList = std::vector<Token>;

typedef enum{
  ATOM,
  ID,
  DEFINE,
  FUNC,
  CALL,
  TUPLE,
  LIST,
  QUOTE
} expr_type;


struct Expr {
  int type=-2;
  bool protect = false;
  ~Expr() = default;
  virtual void    show() =0;
  virtual ExprPtr clone()=0;
  Expr(int type):type(type){};
  Expr(int type,bool protect)
    :type(type),protect(protect){};
};


struct ExprEof : Expr{
  ExprEof():Expr(-1){}
  void show(){
    cout<<"<Expr EOF >"<<endl;
  }
  ExprPtr clone(){
    return std::make_unique<ExprEof>(); 
  }
};

struct ExprAtom : Expr{
  Token literal;
  
  ExprAtom(Token literal)
    : Expr(ATOM),literal(literal){}

  ExprAtom(Token literal,bool protect)
    : Expr(ATOM,protect),literal(literal){}

  ExprPtr clone() override;

  void show() override {
    cout<<BLUE("<Expr ")<<"Atom ";
    if(protect) cout<<"+";
    cout<<endl;
    cout<<setw(10)<<std::left<<"  literal:"<<endl;
    cout<<setw(14)<<' ';literal.show();cout<<endl;
    cout<<BLUE(" >")<<endl;
  }
};

struct ExprId : Expr{
  Token id;
  
  ExprId(Token id)
    : Expr(ID),id(id){}

  ExprId(Token id,bool protect)
    : Expr(ID,protect),id(id){}

  ExprPtr clone() override;

  void show() override {
    cout<<BLUE("<Expr ")<<"Id ";
    if(protect) cout<<"+";
    cout<<endl;
    cout<<setw(10)<<std::left<<"  id:"<<endl;
    cout<<setw(14)<<' ';id.show();cout<<endl;
    cout<<BLUE(" >")<<endl;
  }
};

struct ExprDefine : Expr{
  Token id;
  ExprPtr expr;
  
  ExprDefine(Token id,ExprPtr expr)
    : Expr(DEFINE),id(id),expr(std::move(expr)){}

  ExprDefine(Token id,ExprPtr expr,bool protect)
    : Expr(DEFINE,protect),id(id),expr(std::move(expr)){}

  ExprPtr clone() override;

  void show() override {
    cout<<BLUE("<Expr ")<<"Define ";
    if(protect) cout<<"+";
    cout<<endl;
    cout<<setw(10)<<std::left<<"  id:"<<endl;
    cout<<setw(14)<<' ';id.show();cout<<endl;
    cout<<setw(10)<<std::left<<"  expr:"<<endl;
    cout<<setw(14)<<' ';expr->show();cout<<endl;
    cout<<BLUE(" >")<<endl;
  }
};

struct ExprFunc : Expr{
  TokenList params;
  ExprPtr body;
  
  ExprFunc(TokenList params,ExprPtr body)
    : Expr(FUNC),params(std::move(params)),body(std::move(body)){}

  ExprFunc(TokenList params,ExprPtr body,bool protect)
    : Expr(FUNC,protect),params(std::move(params)),body(std::move(body)){}

  ExprPtr clone() override;

  void show() override {
    cout<<BLUE("<Expr ")<<"Func ";
    if(protect) cout<<"+";
    cout<<endl;
    cout<<setw(10)<<std::left<<"  params:"<<endl;
    for(auto &i:params){
      cout<<setw(14)<<" ";i.show();cout<<endl;
    }
    cout<<setw(10)<<std::left<<"  body:"<<endl;
    cout<<setw(14)<<' ';body->show();cout<<endl;
    cout<<BLUE(" >")<<endl;
  }
};

struct ExprCall : Expr{
  ExprPtr callee;
  ExprPtrList args;
  ExprPtrList extra;
  
  ExprCall(ExprPtr callee,ExprPtrList args,ExprPtrList extra)
    : Expr(CALL),callee(std::move(callee)),args(std::move(args)),extra(std::move(extra)){}

  ExprCall(ExprPtr callee,ExprPtrList args,ExprPtrList extra,bool protect)
    : Expr(CALL,protect),callee(std::move(callee)),args(std::move(args)),extra(std::move(extra)){}

  ExprPtr clone() override;

  void show() override {
    cout<<BLUE("<Expr ")<<"Call ";
    if(protect) cout<<"+";
    cout<<endl;
    cout<<setw(10)<<std::left<<"  callee:"<<endl;
    cout<<setw(14)<<' ';callee->show();cout<<endl;
    cout<<setw(10)<<std::left<<"  args:"<<endl;
    for(auto &i:args){
      cout<<setw(14)<<" ";i->show();cout<<endl;
    }
    cout<<setw(10)<<std::left<<"  extra:"<<endl;
    for(auto &i:extra){
      cout<<setw(14)<<" ";i->show();cout<<endl;
    }
    cout<<BLUE(" >")<<endl;
  }
};

struct ExprTuple : Expr{
  ExprPtrList container;
  
  ExprTuple(ExprPtrList container)
    : Expr(TUPLE),container(std::move(container)){}

  ExprTuple(ExprPtrList container,bool protect)
    : Expr(TUPLE,protect),container(std::move(container)){}

  ExprPtr clone() override;

  void show() override {
    cout<<BLUE("<Expr ")<<"Tuple ";
    if(protect) cout<<"+";
    cout<<endl;
    cout<<setw(10)<<std::left<<"  container:"<<endl;
    for(auto &i:container){
      cout<<setw(14)<<" ";i->show();cout<<endl;
    }
    cout<<BLUE(" >")<<endl;
  }
};

struct ExprList : Expr{
  ExprPtrList container;
  
  ExprList(ExprPtrList container)
    : Expr(LIST),container(std::move(container)){}

  ExprList(ExprPtrList container,bool protect)
    : Expr(LIST,protect),container(std::move(container)){}

  ExprPtr clone() override;

  void show() override {
    cout<<BLUE("<Expr ")<<"List ";
    if(protect) cout<<"+";
    cout<<endl;
    cout<<setw(10)<<std::left<<"  container:"<<endl;
    for(auto &i:container){
      cout<<setw(14)<<" ";i->show();cout<<endl;
    }
    cout<<BLUE(" >")<<endl;
  }
};

struct ExprQuote : Expr{
  ExprPtrList container;
  
  ExprQuote(ExprPtrList container)
    : Expr(QUOTE),container(std::move(container)){}

  ExprQuote(ExprPtrList container,bool protect)
    : Expr(QUOTE,protect),container(std::move(container)){}

  ExprPtr clone() override;

  void show() override {
    cout<<BLUE("<Expr ")<<"Quote ";
    if(protect) cout<<"+";
    cout<<endl;
    cout<<setw(10)<<std::left<<"  container:"<<endl;
    for(auto &i:container){
      cout<<setw(14)<<" ";i->show();cout<<endl;
    }
    cout<<BLUE(" >")<<endl;
  }
};

};
#endif
