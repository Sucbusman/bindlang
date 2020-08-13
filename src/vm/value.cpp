#include <iostream>
#include "value.h"
using std::cout,std::endl;
namespace bindlang::vm{
void printVal(Value val){
  switch(val.type){
    case VAL_NIL:    cout<<"Nil";
    case VAL_BOOL:   cout<<val.as.boolean;break;
    case VAL_NUMBER: cout<<val.as.number;break;
    case VAL_String: cout<<*AS_CSTRING(val);break;
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
