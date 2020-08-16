#include "define/value.h"
#include "define/type.h"
#include <cstdint>
namespace bindlang{

Obj* objchain = nullptr;
stack<EnvPtr> envs = stack<EnvPtr>();

namespace gc{

uint8_t NONE=0x0;
uint8_t USE=~0x0;
uint16_t number=0;
inline void swapFlag(){
  NONE = ~NONE; 
  USE  = ~USE; 
}

}

Obj::~Obj(){
  switch(type){
#define DELETE(T) \
    case objType::T:{\
      auto o=cast(Obj##T*,this);                \
      delete o;                                 \
      break;                                    \
    }
    DELETE(String);
    DELETE(Procedure);
    DELETE(Primitive);
    DELETE(Tuple);
    DELETE(List);
    DELETE(Ast);
  }
  #undef DELETE
}


// make obj helper
void ifgc(){
  if(gc::number == 0xffff){
    //0xffff=2**16=65536
    recycleMem(envs.top());
    gc::number = 0;
  }else{
    gc::number++;
  }
}

ObjString* make_obj(string& s){
  ifgc();
  auto o = new ObjString(string(s),objchain);
  objchain = o;
  return o;
}

ObjProcedure* make_obj(EnvPtr closure,TokenList& params,
                       ExprPtr& body){
  ifgc();
  auto o = new ObjProcedure(closure,params,move(body),objchain);
  objchain = o;
  return o;
}

ObjPrimitive* make_obj(string name,int arity,PrimFunc func){
  ifgc();
  auto o = new ObjPrimitive(name,arity,func,objchain);
  objchain = o;
  return o;
}

ObjTuple* make_obj(ValPtrList container){
  ifgc();
  auto o = new ObjTuple(container,objchain);
  objchain = o;
  return o;
}

ObjList* make_obj(ValPtr head,ObjListPtr tail){
  ifgc();
  auto o = new ObjList(head,tail,objchain);
  objchain = o;
  return o;
}

ObjAst* make_obj(ExprPtr expr){
  ifgc();
  auto o = new ObjAst(move(expr),objchain);
  objchain = o;
  return o;
}

ObjAst* make_obj(Expr* expr){
  ifgc();
  auto o = new ObjAst(expr,objchain);
  objchain = o;
  return o;
}

Obj* copy_obj(Obj* obj){
  switch(obj->type){
    case objType::String:{
      auto obj2 = cast(ObjString*,obj);
      return make_obj(obj2->s);
    }
    case objType::Ast:{
      auto obj2 = cast(ObjAst*,obj);
      return make_obj(obj2->expr);
    }
    case objType::Procedure:{
      auto obj2 = cast(ObjProcedure*,obj);
      // environment should be same
      return make_obj(obj2->closure,obj2->params,obj2->body);
    }
    case objType::Primitive:{
      auto obj2 = cast(ObjPrimitive*,obj);
      return make_obj(obj2->name,obj2->arity,obj2->func);
    }
    case objType::Tuple:{
      auto obj2 = cast(ObjTuple*,obj);
      return make_obj(obj2->container);
    }
    case objType::List:{
      auto obj2 = cast(ObjList*,obj);
      return make_obj(make_shared<Value>(*obj2->head),obj2->tail);
    }
  }
  return nullptr;
}


void mark(Obj* obj){
  // mark obj
  if(obj->flag == gc::NONE){
    obj->flag = gc::USE;
    switch(obj->type){
      case objType::String:
      case objType::Ast:
      case objType::Primitive:
        break;
      case objType::Procedure:{
        auto o = cast(ObjProcedure*,obj);
        mark(o->closure);
        break;
      }
      case objType::Tuple:{
        auto o = cast(ObjTuple*,obj);
        for(auto & val:o->container){
          if(val->isObj()){
            mark(val->as.obj);
          }
        }
        break;
      }
      case objType::List:{
        auto o = cast(ObjList*,obj);
        if(o->head->isObj()){
          mark(o->head->as.obj);
        }
        if(o->tail){
          mark(o->tail);
        }
      }
    }
  }
}

void mark(EnvPtr env){
  while(env){
    for(auto &pair :env->map){
      auto val= pair.second;
      if(val->isObj()){
        mark(val->as.obj);
      }
    }
    env = env->outer;
  }
}

void sweep(){
  // free memory
  Obj** pp = &objchain;
  while(*pp){
    if((*pp)->flag != gc::USE){
      auto dp = *pp;
      //DEBUG("delete ");
      //dp->show();
      //cerr<<endl;
      *pp = dp->next;
      delete dp;
    }else{
      pp = &((*pp)->next);
    }
  }
  gc::swapFlag();
}

void recycleMem(EnvPtr env){
  // garbage collector
  mark(env);
  sweep();
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

#define CHECK(T,DST)                           \
  if(v->type != VAL_##T) return false;       \
  auto tmp___ = v->as.obj;                   \
  auto DST = dynamic_cast<Obj##T*>(tmp___);

bool takeString(ValPtr v,string*& ans){
  CHECK(String,obj);
  ans = &obj->s;
  return true;
}

bool takeTuple(ValPtr v,ValPtrList*& ans){
  CHECK(Tuple,obj);
  ans = &obj->container;
  return true;
}

bool takeList(ValPtr v,ValPtr*& head,ObjListPtr*& tail){
  CHECK(List,obj);
  head = &obj->head;
  tail = &obj->tail;
  return true;
}

// print stuff
void printVal(ValPtr val) {
  if(!val){
    return; 
  }
  switch (val->type) {
    case VAL_BOOL:
      if(val->as.boolean){
        cout << "true";
      }else{
        cout << "false";
      }
      break;
    case VAL_NUMBER:
      cout << val->as.number;
      break;
    case VAL_String:{
      auto o = cast(ObjString*,val.get()->as.obj);
      cout<<o->s;
      break;
    }
#define SHOW(T)\
    case VAL_##T:{\
      auto o=cast(Obj##T*,val.get()->as.obj);\
      o->show();break;                       \
    }
    SHOW(Procedure)
    SHOW(Primitive)
    SHOW(List)
    SHOW(Tuple)
    SHOW(Ast)
    default:
        break;
    }
#undef SHOW
}

void ObjTuple::show(){
  cout<<BLUE("<Tuple ");
  for(auto const& val:container){
    printVal(val);cout<<' ';
  }
  cout<<BLUE(" >");
}

void ObjList::show(){
  cout<<"[";
  bool first = true;
  ObjListPtr o = this;
  while(o){
    if(first){
      first = false;
      printVal(o->head);
    }else{
      if(o->head){
        cout<<" "; 
        printVal(o->head);
      }
    }
    o = o->tail;
  }
  cout<<"]";
}

void ObjAst::show(){
  cout<<BLUE("<Ast ")<<endl;
  indent();
    showidt();expr->show();cout<<endl;
  deindent();
  cout<<BLUE(">");
}

// environment
ValPtr* Environment::get(string s){
  auto it = map.find(s);
  if(it != map.end()){
    return &it->second;
  }else if(outer){
    return outer->get(s);
  }else{
    return nullptr;
  }
}

void Environment::set(string s,const ValPtr& val){
  map[s] = val;
}

void Environment::show(){
  for(auto const& pair:map){
    cout<<BLUE("<Environment ");
    cout<<" key:"<<std::left<<std::setw(10)<<pair.first
        <<" arr:"<<pair.second<<" value: ";
    printVal(pair.second);
    cout<<BLUE(" >")<<endl;
  }
  cout<<"-------"<<endl;
  if(outer){
    outer->show();
  }
}


}//namespace
