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
  stack<EnvPtr> envs;
  EnvPtr toplevel;

  void   standardEnvironment();
  void   debugger(ExprPtr expr);
  ValPtr evalAtom(ExprPtr expr);
  ValPtr evalTuple(ExprPtr expr);
  ValPtr evalId(ExprPtr expr);
  ValPtr evalDefine(ExprPtr expr);
  ValPtr evalFunc(ExprPtr expr);
  ValPtr evalCall(ExprPtr expr);

  // calculate
  ValPtr calculator(ExprPtrList args,unsigned long init,
                    function<double(double,double)>);
  template <typename T>
  ValPtr calcu2(ExprPtrList args,function<T*(T&,T&)>);
  ValPtr Plus(ExprPtrList args);
  ValPtr binary(ExprPtrList args,function<bool(double,double)>);
  ValPtr Eq(ExprPtrList args);

  // control flow
  ValPtr Begin(ExprPtrList args);
  ValPtr If(ExprPtrList args);
  ValPtr Print(ExprPtrList args);
  ValPtr While(ExprPtrList args);
  ValPtr Pipe(ExprPtrList args,bool);
  ValPtr Quote(ExprPtrList args);

  // tuple
  ValPtr Take(ExprPtrList args);
  ValPtr Length(ExprPtrList args);
  ValPtr Set(ExprPtrList args);

  // debug
  ValPtr InspectEnv(ExprPtrList args);
  ValPtr Debug(ExprPtrList args);
  ValPtr Garbage(ExprPtrList args);
};

  
}

#endif
