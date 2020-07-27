#ifndef __value__
#define __value__
#include <cstdint>
#include <unordered_map>
#include "type.h"
#include "ast.h"

namespace bindlang{

using std::string;

enum class objType{
  String,
  Procedure,
  Primitive
};

struct Obj{
  objType type;
  struct Obj *next;
  Obj()
    :type(objType::String),next(nullptr){}
  Obj(objType type)
    :type(type),next(nullptr){}
  Obj(objType type,Obj *next)
    :type(type),next(next){}
  virtual void show()=0;
};


struct ObjString : Obj{
  ObjString(string s)
    :Obj(objType::String),s(s){}
  ObjString(string s,Obj *next)
    :Obj(objType::String,next),s(s){}
  void show(){
    std::cout<<BLUE("<String ")<<s<<BLUE(">")<<endl;
  }
  string s;
};

struct ObjProcedure : Obj{
  ObjProcedure()
    :Obj(objType::Procedure){}
  ObjProcedure(TokenList params, ExprPtr body)
    :Obj(objType::Procedure),
     params(move(params)),body(move(body)){}
  ObjProcedure(TokenList params, ExprPtr body,Obj *next)
    :Obj(objType::Procedure,next),
     params(move(params)),body(move(body)){}
  ObjProcedure(EnvPtr closure,TokenList params, ExprPtr body,Obj *next)
    :Obj(objType::Procedure,next),closure(closure),
     params(move(params)),body(move(body)){}
  EnvPtr closure;
  TokenList params;
  ExprPtr body;
  void show(){
    std::cout<<BLUE("<Procedure >");
  }
};

struct ObjPrimitive : Obj {
  ObjPrimitive(string name,int arity,
               std::function<ValPtr(ExprPtrList)> func,
               Obj *next)
    :Obj(objType::Primitive,next),name(name),func(func){}
  string name;
  unsigned long arity;
  PrimFunc func;
  void show(){
    std::cout<<BLUE("<Primitive ")
             <<name<<BLUE(" >");
  }
};

/* gc stuf */
extern Obj* objchain;
ObjString* make_obj(string&);
ObjProcedure* make_obj(EnvPtr,TokenList&,ExprPtr&);
ObjPrimitive* make_obj(string,int,PrimFunc);

typedef enum{
             VAL_BOOL,
             VAL_NIL,
             VAL_NUMBER,
             VAL_OBJ
} val_type;

struct Value{
  Value()
    :type(VAL_NIL){}
  Value(bool boolean)
    :type(VAL_BOOL){as.boolean = boolean;}
  Value(double number)
    :type(VAL_NUMBER){as.number = number;}
  Value(Obj *obj)
    :type(VAL_OBJ){as.obj = obj;}
  uint8_t type;
  union{
    bool   boolean;
    double number;
    Obj    *obj;
  }as;
};

void printVal(ValPtr);

class Environment{
 public:
  Environment(){}
  Environment(EnvPtr outer):outer(outer){}
  ValPtr get(string);
  void   set(string,const ValPtr&);
  void   show();
 private:
  EnvPtr outer;
  std::unordered_map<string, ValPtr> map;
};

}//namespace

#endif
