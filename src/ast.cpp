#include <iostream>
#include "ast.h"
using namespace bindlang;

Value Expr::accept(Visitor & v){
  return v.visit(*this); 
}
