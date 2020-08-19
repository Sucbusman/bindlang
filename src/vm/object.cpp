#include "back/interpreter/value.h"
#include "vm/vm.h"

namespace bindlang::vm{

void ifgc(){
  if(bytesAllocated == 0xffff){
    recycleMem();
  }else{
    bytesAllocated++;
  }
}

void recycleMem(){
  
}

size_t bytesAllocated;
Obj* objchain;
ObjString* make_obj(string& s){
  //ifgc();
  auto o = new ObjString(string(s),objchain);
  objchain = o;
  return o;
}

ObjString* make_obj(const char* s){
  //ifgc();
  auto o = new ObjString(string(s),objchain);
  objchain = o;
  return o;
}

ObjProcedure* make_obj(string const&name,int arity,size_t offset){
  auto o = new ObjProcedure(name,arity,offset,objchain);
  objchain = o;
  return o;
}

ObjList* make_obj(Value const& head,ObjList* tail){
  auto o = new ObjList(head,tail,objchain);
  objchain = o;
  return o;
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
      val.as.obj = o->clone(&objchain);
    }
  }
  return val;
}

}
