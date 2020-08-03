#ifndef __value__
#define __value__
#include <cstdint>
#include <unordered_map>
#include <stack>
#include "type.h"
#include "ast.h"

namespace bindlang{

using std::string;

enum class objType{
  String,
  Procedure,
  Primitive,
  Tuple,
  List,
  Ast
};

namespace gc{
extern uint8_t NONE;
extern uint8_t USE;
extern uint16_t number;
};

struct Obj{
  objType type;
  struct Obj *next;
  uint8_t flag;
  Obj()
    :type(objType::String),next(nullptr),flag(gc::NONE){}
  Obj(objType type)
    :type(type),next(nullptr),flag(gc::NONE){}
  Obj(objType type,Obj *next)
    :type(type),next(next),flag(gc::NONE){}
  virtual ~Obj();
  virtual void show()=0;
};


struct ObjString : Obj{
  ObjString()
    :Obj(objType::String){}
  ObjString(string s)
    :Obj(objType::String),s(s){}
  ObjString(string s,Obj *next)
    :Obj(objType::String,next),s(s){}
  void show(){
    std::cout<<BLUE("<String ")<<s<<BLUE(">")<<endl;
  }
  string s;
};

ObjString* StringPlus(ObjString& l,ObjString& r);
ObjString* operator+ (ObjString& l,ObjString& r);

struct ObjProcedure : Obj{
  ObjProcedure()
    :Obj(objType::Procedure){}
  ObjProcedure(EnvPtr closure,TokenList params, ExprPtr body,Obj *next)
    :Obj(objType::Procedure,next),closure(closure)
    ,params(move(params)),body(move(body)){}
  EnvPtr closure;
  TokenList params;
  ExprPtr body;
  void show(){
    std::cout<<BLUE("<Procedure ")<<BLUE(" >");
  }
};

struct ObjPrimitive : Obj {
  ObjPrimitive(string name,int arity,
               std::function<ValPtr(ExprPtrList)> func,
               Obj *next)
    :Obj(objType::Primitive,next),
     name(name),arity(arity),func(func){}
  string name;
  int arity;
  PrimFunc func;
  void show(){
    std::cout<<BLUE("<Primitive ")
             <<name<<BLUE(" >");
  }
};

struct ObjTuple : Obj{
  ObjTuple(ValPtrList container,Obj *next)
    :Obj(objType::Tuple,next),container(container){}
  ValPtrList container;
  void show() override;
};

struct ObjList : Obj{
  ObjList(ValPtr head,Obj *next)
    :Obj(objType::List,next),head(head),tail(nullptr){}
  ObjList(ValPtr head,ObjList* tail,Obj *next)
    :Obj(objType::List,next),head(head),tail(tail){}
  ValPtr head;
  ObjListPtr tail;
  void show() override;
};

ObjTuple* TuplePlus(ObjTuple& l,ObjTuple& r);
ObjTuple* operator+ (ObjTuple& l,ObjTuple& r);

struct ObjAst : Obj{
  ObjAst(ExprPtrList container,Obj *next)
    :Obj(objType::Ast,next),container(move(container)){}
  ExprPtrList container;
  void show() override;
};
/* extract cpp value from Value */

bool takeBool(ValPtr v,bool*& ans);
bool takeNumber(ValPtr v,double*& ans);
bool takeString(ValPtr v,string*& ans);
bool takeTuple(ValPtr v,ValPtrList*& ans);
bool takeList(ValPtr v,ValPtr*& head,ObjListPtr*& tail);

/* gc stuf */
extern Obj* objchain;
extern stack<EnvPtr> envs;
ObjString*    make_obj(string&);
ObjProcedure* make_obj(EnvPtr,TokenList&,ExprPtr&);
ObjPrimitive* make_obj(string,int,PrimFunc);
ObjTuple*     make_obj(ValPtrList);
ObjList*      make_obj(ValPtr head,ObjList* tail);
ObjAst*       make_obj(ExprPtrList);
Obj*          copy_obj(Obj* obj);
void          recycleMem(EnvPtr env);
void          mark(EnvPtr obj);
void          mark(Obj* obj);
void          sweep();

typedef enum{
             VAL_BOOL,
             VAL_NIL,
             VAL_NUMBER,
             VAL_String,
             VAL_Procedure,
             VAL_Primitive,
             VAL_Tuple,
             VAL_List,
             VAL_Ast,
             VAL_OBJ
} val_type;

struct Value{
  Value()
    :type(VAL_NIL){}
  Value(bool boolean)
    :type(VAL_BOOL){as.boolean = boolean;}
  Value(double number)
    :type(VAL_NUMBER){as.number = number;}
#define OBJ2VAL(T,m)                               \
  Value(Obj##T *obj):type(VAL_##T){as.obj = obj;immutable=m;}
  OBJ2VAL(String,false);
  OBJ2VAL(Procedure,true);
  OBJ2VAL(Primitive,true);
  OBJ2VAL(Tuple,false);
  OBJ2VAL(List,true);
  OBJ2VAL(Ast,false);
  Value(Obj *obj)
    :type(VAL_OBJ){as.obj = obj;}
  Value(Value const& v){
    type = v.type;
    switch(type){
      case VAL_BOOL:
      case VAL_NIL:
      case VAL_NUMBER:
        as = v.as;
        break;
      case VAL_String:
      case VAL_Procedure:
      case VAL_Primitive:
      case VAL_Tuple:
      case VAL_List:
      case VAL_Ast:
      case VAL_OBJ:{
        auto o = v.as.obj;
        as.obj = copy_obj(o);
        break;
      }
    }
  }
  uint8_t type;
  bool    immutable = false;
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
  Environment(Environment const& e){
    outer = e.outer;
    map  = unordered_map<string,ValPtr>(e.map);
  }
  ValPtr* get(string);
  void   set(string,const ValPtr&);
  void   show();
  EnvPtr outer;
  std::unordered_map<string, ValPtr> map;
};

}//namespace

#endif
