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
  void   standardEnvironment();
  void   debugger(ExprPtr expr);
  ValPtr evalAtom(ExprPtr expr);
  ValPtr evalId(ExprPtr expr);
  ValPtr evalDefine(ExprPtr expr);
  ValPtr evalFunc(ExprPtr expr);
  ValPtr evalCall(ExprPtr expr);
  
  stack<EnvPtr> envs;
  EnvPtr toplevel;
  ValPtr calculator(ExprPtrList args,unsigned long init,
                    function<double(double,double)>);

  ValPtr binary(ExprPtrList args,function<bool(double,double)>);

  ValPtr Begin(ExprPtrList args);
  ValPtr If(ExprPtrList args);
  ValPtr Eq(ExprPtrList args);
  ValPtr Print(ExprPtrList args);
  ValPtr While(ExprPtrList args);
  ValPtr Pipe(ExprPtrList args,bool);
  ValPtr InspectEnv(ExprPtrList args);
  ValPtr Debug(ExprPtrList args);
};

  
}

#endif
