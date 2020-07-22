#ifndef __ast__
#define __ast__

#include <memory>
#include <vector>
#include <string>

#include "token.h"
#include "value.h"
namespace bindlang{

struct Expr;//declared ahead
struct Visitor{
  virtual Value visit(Expr& expr)=0;
};

typedef enum{
  DEFINE,
  CALL,
  FUNC
} expr_type;


struct Expr {
  int type=-1;
  ~Expr() = default;
  Value accept(Visitor & v);
  Expr(int type):type(type){};
};

using ExprPtr = std::unique_ptr<Expr>;
using ExprPtrList = std::vector<ExprPtr>;
using IdList = std::vector<std::string>;

struct ExprDefine : Expr{
  Token id;
  ExprPtr expr;
  
  ExprDefine(Token id,ExprPtr expr):
    Expr(DEFINE),id(id),expr(std::move(expr)){};
};

struct ExprCall : Expr{
  Token id;
  ExprPtr expr;
  
  ExprCall(Token id,ExprPtr expr):
    Expr(CALL),id(id),expr(std::move(expr)){};
};

struct ExprFunc : Expr{
  Token id;
  ExprPtrList args;
  
  ExprFunc(Token id,ExprPtrList args):
    Expr(FUNC),id(id),args(args){};
};

};
#endif
