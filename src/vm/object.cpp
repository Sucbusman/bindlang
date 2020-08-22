#include "back/interpreter/value.h"
#include "vm/value.h"
#include "vm/vm.h"

namespace bindlang::vm{

// global
Obj* objchain=nullptr;
size_t bytesAllocated = 0;

Obj::~Obj(){
  switch(type){
#define DELETE(T)                               \
    case (uint8_t)objType::T:{                  \
      auto o=cast(Obj##T*,this);                \
      delete o;                                 \
      break;                                    \
    }
    DELETE(String);
    DELETE(Procedure);
    DELETE(List);
  }
  #undef DELETE
}


ObjString* make_obj(string& s){
  auto o = new ObjString(string(s),objchain);
  objchain = o;
  bytesAllocated += s.size();
  return o;
}

ObjString* make_obj(const char* s){
  auto cxxs = string(s);
  auto o = new ObjString(cxxs,objchain);
  objchain = o;
  bytesAllocated += cxxs.size();
  return o;
}

Obj* ObjString::clone(){
    auto o = new ObjString(s,objchain);
    objchain = o;
    bytesAllocated += s.size();
    return o;
}

ObjProcedure* make_obj(string const&name,uint8_t arity,
                       size_t offset){
  auto o = new ObjProcedure(name,arity,offset,objchain);
  objchain = o;
  bytesAllocated += name.size()+sizeof(arity)+sizeof(offset);
  return o;
}

Obj* ObjProcedure::clone() {
  auto o = new ObjProcedure(name,arity,offset,objchain);
  o->captureds = captureds;
  objchain = o;
  bytesAllocated += name.size()+sizeof(arity)+sizeof(offset);
  return o;
}

ObjList* make_obj(Value const& head,ObjList* tail){
  auto o = new ObjList(head,tail,objchain);
  objchain = o;
  bytesAllocated += sizeof(ObjList);
  return o;
}

Obj* ObjList::clone(){
  auto o = new ObjList(head,tail,objchain);
  objchain = o;
  bytesAllocated += sizeof(ObjList);
  return o;
}

void printObj(Obj* obj){
  switch(obj->type){
#define WHEN(T,EXPS) \
    case (uint8_t)objType::T:{                 \
      auto o = cast(Obj##T*,obj);              \
      EXPS;                                    \
      break;                                   \
    }
    WHEN(String,cout<<o->s;)
    WHEN(List,printList(o,printVal);)
    WHEN(Procedure,
         cout<<o->name<<'('<<o->arity<<')'
         <<" 0x"<<std::hex<<o->offset;)
    default:break;
  }
#undef WHEN
}

void inspectObj(Obj* obj){
  cout<<BLUECODE<<"<Object "<<objTable[(uint8_t)obj->type]
      <<"> "<<DEFAULT<<std::hex<<"0x"<<(size_t)obj<<' ';
  switch(obj->type){
#define WHEN(T,EXPS) \
    case (uint8_t)objType::T:{                 \
      auto o = cast(Obj##T*,obj);              \
      EXPS;                                    \
      break;                                   \
    }
    WHEN(String,cout<<o->s;)
    WHEN(List,printList(o,inspectVal);)
    WHEN(Procedure,
      cout<<o->name<<'('<<o->arity<<')'
          <<" 0x"<<std::hex<<o->offset;)
    default:break;
  }
#undef WHEN
}

void printList(ObjList* obj,
               std::function<void(Value const&)> f){
  cout<<"[";
  bool first = true;
  while(obj){
    if(first){
      if(obj->tail){
        first = false;
        f(obj->head);
      }
    }else{
      if(obj->tail){//omit last ObjList(bool,nullptr)
        cout<<' ';
        f(obj->head);
      }
    }
    obj = obj->tail;
  }
  cout<<']';
}

}
