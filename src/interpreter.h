#ifndef __interpreter__
#define __interpreter__
#include <unordered_map>
#include "parser.h"
#include "value.h"
#include "type.h"

namespace bindlang {


class Interpreter{
 public:
  Interpreter(){standardEnvironment();}
  void reset();
  ValPtr eval(ExprPtr expr);
 private:
  bool   interrupt = false;
  int    error_num=0;
  EnvPtr toplevel;

  void   standardEnvironment();
  void   debugger(ExprPtr expr);
  ValPtr evalAtom(ExprPtr expr);
  ValPtr evalTuple(ExprPtr expr);
  ValPtr evalList(ExprPtr expr);
  ValPtr evalId(ExprPtr expr);
  ValPtr evalDefine(ExprPtr expr);
  ValPtr evalSet(ExprPtr expr);
  ValPtr evalFunc(ExprPtr expr);
  ValPtr evalCall(ExprPtr expr);
  ValPtr evalCallProc(ObjProcedure* proc,
                      ExprPtrList& args,ExprPtrList& extra);
  ValPtr evalCallPrim(ObjPrimitive* prim,
                      ExprPtrList& args,ExprPtrList& extra);

  // calculate
  ValPtr calculator(ExprPtrList args,double init,
                    function<double(double,double)>);
  template <typename T>
  ValPtr calcu2(ExprPtrList args,function<T*(T&,T&)>);
  ValPtr Plus(ExprPtrList args);

  // comparison
  ValPtr compare(ExprPtrList args,function<bool(double,double)>);
  ValPtr Eq(ExprPtrList args);

  // logic
  ValPtr Not(ExprPtrList args);
  ValPtr And(ExprPtrList args);
  ValPtr Or(ExprPtrList args);

  // control flow
  ValPtr Begin(ExprPtrList args);
  ValPtr If(ExprPtrList args);
  ValPtr While(ExprPtrList args);
  ValPtr Pipe(ExprPtrList args,bool);

  // tuple:mutable flat array,random access
  ValPtr isEmpty(ExprPtrList args);

  ValPtr Take(ExprPtrList args);
  ValPtr Length(ExprPtrList args);
  ValPtr Set(ExprPtrList args);
  ValPtr Push(ExprPtrList args);
  ValPtr Pop(ExprPtrList args);

  // list:immutable,binary tree,easy to recurse
  ValPtr Cons(ExprPtrList args);
  ValPtr Head(ExprPtrList args);
  ValPtr Tail(ExprPtrList args);

  // IO
  ValPtr Print(ExprPtrList args);

  // debug
  ValPtr InspectEnv(ExprPtrList args);
  ValPtr Debug(ExprPtrList args);
  ValPtr Garbage(ExprPtrList args);
  ValPtr Gc(ExprPtrList args);
};

  
}

#endif
