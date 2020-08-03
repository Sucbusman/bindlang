#ifndef __type__
#define __type__
#include <memory>
#include <vector>
#include <functional>
#include "token.h"

namespace bindlang{

struct Expr;
struct Value;
struct ObjList;
using  TokenList = std::vector<Token>;
using  ObjListPtr = ObjList*;
using  ExprPtr = std::unique_ptr<Expr>;
using  ExprPtrList = std::vector<ExprPtr>;
using  ValPtr = std::shared_ptr<Value>;
using  ValPtrList = std::vector<ValPtr>;
using  PrimFunc = std::function<ValPtr(ExprPtrList)>;
class  Environment;
using  EnvPtr = std::shared_ptr<Environment>;

}

#endif
