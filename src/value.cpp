#include "value.h"
namespace bindlang{
Obj* objchain = nullptr;

ValPtr Environment::get(string s){
  auto it = map.find(s);
  if(it != map.end()){
    return it->second;
  }else if(outer){
    return outer->get(s);
  }else{
    return nullptr;
  }
}

void Environment::set(string s,const ValPtr& val){
  map[s] = val;
}

ObjString* make_obj(string& s){
  auto o = new ObjString(string(s),objchain);
  objchain = o;
  return o;
}

ObjProcedure* make_obj(EnvPtr closure,TokenList& params,
                       ExprPtr& body){
  auto o = new ObjProcedure(closure,move(params),move(body),objchain);
  objchain = o;
  return o;
}

ObjPrimitive* make_obj(string name,int arity,PrimFunc func){
  auto o = new ObjPrimitive(name,arity,func,objchain);
  objchain = o;
  return o;
}

void printVal(ValPtr val) {
  if(!val) return;
  switch (val->type) {
    case VAL_BOOL:
      if(val->as.boolean){
        cout << "#t";
      }else{
        cout << "#f";
      }
      break;
    case VAL_NUMBER:
      cout << val->as.number;
      break;
    case VAL_OBJ: {
      auto obj = val.get()->as.obj;
      switch (obj->type) {
        case objType::String: {
          auto o = dynamic_cast<ObjString *>(obj);
          cout<<o->s;
          break;
        }
        case objType::Procedure: {
          auto o = dynamic_cast<ObjProcedure *>(obj);
          o->show();
          break;
        }
        case objType::Primitive: {
          auto o = dynamic_cast<ObjPrimitive *>(obj);
          o->show();
          break;
        }
        default:
          break;
      }
      break;
    }
  }
}

void Environment::show(){
  for(auto const& pair:map){
    cout<<BLUE("<Environment ");
    cout<<"key:"<<pair.first<<" value: ";
    printVal(pair.second);
    cout<<BLUE(" >")<<endl;
  }
  cout<<"-------"<<endl;
  if(outer){
    outer->show();
  }
}

}
