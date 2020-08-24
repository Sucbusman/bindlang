#include <ios>
#include <iostream>
#include "vm/value.h"
using std::cout,std::endl;
namespace bindlang::vm{

const char* opcTable[]={
  VM_INSTALL_ALL_INST(VM_EXPAND_LIST_STR)
};

const char* valTable[]={
  VM_INSTALL_ALL_VAL(VM_EXPAND_LIST_STR)
};

const char* objTable[]={
  VM_INSTALL_ALL_OBJ(VM_EXPAND_LIST_STR)
};

bool truthy(const Value &val){
  switch(val.type){
    case VAL_NIL:return false;
    case VAL_BOOL:return val.as.boolean;
    default:return true;
  }
}

bool Value::operator== (Value const& v) const{
  if(v.type!=type) return false;
  switch(type){
    case VAL_NIL:    return true;
    case VAL_BOOL:   return as.boolean == v.as.boolean;
    case VAL_NUMBER: return as.number == v.as.number;
#define WHEN(T,EXPRS)                                  \
    case VAL_##T:{                                     \
      auto l = cast(Obj##T*,as.obj);                   \
      auto r = cast(Obj##T*,v.as.obj);                 \
      EXPRS;}

      WHEN(Procedure,
           return l->name == r->name;)
      WHEN(String,
           return l->s == r->s;)
    default: return as.obj == v.as.obj;
  }
}

bool valueEqual(const Value & v1, const Value & v2){
  if(v1.type != v2.type) return false;
  switch(v1.type){
    case VAL_NIL:return true;
    case VAL_BOOL:return v1.as.boolean == v2.as.boolean;
    case VAL_NUMBER:return v1.as.number == v2.as.number;
    default:return v1.as.obj == v2.as.obj;//object same
  }
}

void printVal(Value const& val){
  // we should not print newline at here
  switch(val.type){
    case VAL_NIL:    cout<<"nil";break;
    case VAL_BOOL:   cout<<(val.as.boolean?"#t":"#f");break;
    case VAL_NUMBER: cout<<dec<<val.as.number;break;
    default:         printObj(val.as.obj);break;
  }
}


void inspectVal(Value const& val){
  cout<<BLUECODE<<left<<setw(10)<<valTable[val.type]<<DEFAULT
      <<' '<<hex<<&val<<' ';
  switch(val.type){
    case VAL_NIL:    cout<<"nil";break;
    case VAL_BOOL:   cout<<(val.as.boolean?"#t":"#f");break;
    case VAL_NUMBER: cout<<dec<<val.as.number;break;
    default:         inspectObj(val.as.obj);break;
  }
}

Value copy_val(Value const& v){
  auto val = Value();
  val.type = v.type;
  switch(v.type){
    case VAL_BOOL:
    case VAL_NIL:
    case VAL_NUMBER:
      val.as = v.as;
      break;
    case VAL_List:
    case VAL_Procedure:
    case VAL_String:{
      auto o =v.as.obj;
      val.as.obj = o->clone();
    }
  }
  return val;
}

}
