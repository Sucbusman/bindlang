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

void printVal(Value const& val){
  switch(val.type){
    case VAL_NIL:    cout<<"Nil";
    case VAL_BOOL:   cout<<val.as.boolean;break;
    case VAL_NUMBER: cout<<val.as.number;break;
    case VAL_String: cout<<*AS_CSTRING(val);break;
    case VAL_Procedure:{
      auto offset = AS_PROCEDURE(val)->offset;
      cout<<"Func "<<std::dec<<offset;
      break;
    }
    default:
      break;
  }
  cout<<endl;
}

void inspectVal(Value const& val){
  cout<<valTable[val.type]<<' '<<hex<<&val<<' ';
  switch(val.type){
    case VAL_NIL:    cout<<"Nil";
    case VAL_BOOL:   cout<<val.as.boolean;break;
    case VAL_NUMBER: cout<<val.as.number;break;
    case VAL_String: cout<<*AS_CSTRING(val);break;
    case VAL_Procedure:{
      auto offset = AS_PROCEDURE(val)->offset;
      cout<<"func offset:"<<std::dec<<offset;
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
