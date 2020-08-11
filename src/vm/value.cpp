#include <iostream>
#include "value.h"
using std::cout,std::endl;
namespace bindlang::vm{
void printVal(Value val){
  switch(val.type){
    case VAL_NIL:    cout<<"Nil";
    case VAL_BOOL:   cout<<val.as.boolean;break;
    case VAL_NUMBER: cout<<val.as.number;break;
    case VAL_String: cout<<*AS_STRING(val);break;
    default:
      break;
  }
  cout<<endl;
}

}
