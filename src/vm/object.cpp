#include "vm.h"

namespace bindlang::vm{

void VM::ifgc(){
  if(bytesAllocated == 0xffff){
    recycleMem();
  }else{
    bytesAllocated++;
  }
}

void VM::recycleMem(){
  
}

Obj* VM::copy_obj(Obj* obj){
  switch(obj->type){
    case (int)objType::String:{
      ObjString *obj2 = (ObjString*)obj;
      return make_obj(obj2->s);
    }
  }
}

ObjString* VM::make_obj(string& s){
  //ifgc();
  auto o = new ObjString(string(s),objchain);
  objchain = o;
  return o;
}

ObjString* VM::make_obj(const char* s){
  //ifgc();
  auto o = new ObjString(string(s),objchain);
  objchain = o;
  return o;
}

Value VM::copy_val(Value const& v){
  auto val = Value();
  val.type = v.type;
  val.immutable = v.immutable;
  switch(v.type){
    case VAL_BOOL:
    case VAL_NIL:
    case VAL_NUMBER:
      val.as = v.as;
      break;
    case VAL_String:{
      auto o =v.as.obj;
      val.as.obj = copy_obj(o);
    }
  }
  return val;
}

}
