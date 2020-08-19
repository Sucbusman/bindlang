#include <ios>
#include <iostream>
#include "value.h"
using std::cout,std::endl;
namespace bindlang::vm{

const char* opcTable[]={
  VM_INSTALL_ALL_INST(VM_EXPAND_LIST_STR)
};

const char* valTable[]={
  VM_INSTALL_ALL_VAL(VM_EXPAND_LIST_STR)
};

bool truthy(const Value &val){
  switch(val.type){
    case VAL_NIL:return false;
    case VAL_BOOL:return val.as.boolean;
    default:return true;
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
    case VAL_NIL:    cout<<"Nil";
    case VAL_BOOL:   cout<<val.as.boolean;break;
    case VAL_NUMBER: cout<<dec<<val.as.number;break;
    case VAL_String: cout<<*AS_CSTRING(val);break;
    case VAL_Procedure:{
      auto proc = AS_PROCEDURE(val);
      cout<<"<procedure> "<<proc->name<<' '
          <<'('<<proc->arity<<')'
          <<" 0x"<<std::hex<<proc->offset;
      break;
    }
    default:
      cerr<<"Unreachable!"<<endl;
      break;
  }
}

void inspectVal(Value const& val){
  try {
    cout<<BLUECODE<<left<<setw(10)<<valTable[val.type]<<DEFAULT
      <<' '<<hex<<&val<<' ';
  } catch(...){
    cout<<"throw"<<endl;
  }
  switch(val.type){
    case VAL_NIL:    cout<<"Nil";break;
    case VAL_BOOL:   cout<<val.as.boolean;break;
    case VAL_NUMBER: cout<<val.as.number;break;
    case VAL_String: cout<<*AS_CSTRING(val);break;
    case VAL_Procedure:{
      auto proc = AS_PROCEDURE(val);
      cout<<proc->name<<'('<<proc->arity<<')'<<" 0x"<<std::hex<<proc->offset;
      break;
    }
    default:
      break;
  }
  cout<<endl;
}

// environment
Value* Env::get(string s){
  auto it = map.find(s);
  if(it != map.end()){
    return &it->second;
  }else if(outer){
    return outer->get(s);
  }else{
    return nullptr;
  }
}

void Env::set(string s,const Value& val){
  map[s] = val;
}

}
