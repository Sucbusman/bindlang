#include "value.h"
#include "type.h"
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


// make obj helper
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

ObjTuple* make_obj(ValPtrList container){
  auto o = new ObjTuple(container,objchain);
  objchain = o;
  return o;
}

ObjAst* make_obj(ExprPtrList container){
  auto o = new ObjAst(move(container),objchain);
  objchain = o;
  return o;
}

// operator overloading
ObjString* StringPlus(ObjString& l,ObjString& r){
  auto s= l.s+r.s;
  return make_obj(s);
}

ObjString* operator+ (ObjString& l,ObjString& r){
  return StringPlus(l,r);
}

ObjTuple* TuplePlus (ObjTuple& l,ObjTuple& r){
  auto c = ValPtrList(l.container);
  for(auto const& i:r.container){
    c.push_back(i);
  }
  return make_obj(c);
}

ObjTuple* operator+ (ObjTuple& l,ObjTuple& r){
  return TuplePlus(l, r);
}

// take
bool takeBool(ValPtr v,bool*& ans){
  if(v->type != VAL_BOOL) return false;
  ans = &v->as.boolean;
  return true;
}

bool takeNumber(ValPtr v,double*& ans){
  if(v->type != VAL_NUMBER) return false;
  ans = &v->as.number;
  return true;
}

#define CHECK(T1,T2,T3)                       \
  if(v->type != T1) return false;             \
  auto tmp = v->as.obj;                       \
  if(tmp->type != objType::T2) return false;  \
  auto obj = dynamic_cast<T3*>(tmp);

bool takeString(ValPtr v,string*& ans){
  CHECK(VAL_OBJ,String,ObjString);
  ans = &obj->s;
  return true;
}

bool takeTuple(ValPtr v,ValPtrList*& ans){
  CHECK(VAL_OBJ,Tuple,ObjTuple);
  ans = &obj->container;
  return true;
}

// print stuff
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
        case objType::Tuple: {
          auto o = dynamic_cast<ObjTuple *>(obj);
          o->show();
          break;
        }
        case objType::Ast:{
          auto o = dynamic_cast<ObjAst *>(obj);
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

void ObjTuple::show(){
  std::cout<<BLUE("<Tuple ");
  for(auto const& val:container){
    printVal(val);cout<<' ';
  }
  std::cout<<BLUE(" >");
}

void ObjAst::show(){
  std::cout<<BLUE("<Ast ");
  for(auto const& val:container){
    val->show();
  }
  std::cout<<BLUE(" >");
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
