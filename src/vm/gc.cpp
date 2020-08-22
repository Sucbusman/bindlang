#include "vm.h"
namespace bindlang::vm{
#define USE   0b01
#define UNUSE 0b00
#define UNDESTRUCTABLE 0b10

void VM::ifGc(){
  if(bytesAllocated >= GC_THRESHOLD){
    recycleMem();
    GC_THRESHOLD = GC_THRESHOLD << 1;
  }
}

void VM::prepareGc(){
 // set constant table undestructable
 for(auto &val:constants){
   if(isObj(val)){
     val.as.obj->flag = UNDESTRUCTABLE;
   }
 }
 // set global value undestructable
 for(auto i=0;i<entry;i++){
   auto val = values[i];
   if(isObj(val)){
     val.as.obj->flag = UNDESTRUCTABLE;
   }
 }
}

void VM::recycleMem(){
  mark();
  sweep();
}

void VM::mark(Value &val){
  if(isObj(val)){
    mark(val.as.obj);
  }
}

void VM::mark(Obj *obj){
 // mark obj
  if(obj->flag == UNUSE){
    obj->flag = USE;
    //DEBUG("mark "<<std::hex<<(size_t)obj);
    switch(obj->type){
#define WHEN(T) case (uint8_t)objType::T
      WHEN(String):break;
      WHEN(Procedure):{
        auto proc = cast(ObjProcedure*,obj);
        for(auto & val:proc->captureds){
          mark(val);
        }
        break;
      }
      WHEN(List):{
        auto lst = cast(ObjList*,obj);
        mark(lst->head);
        if(lst->tail){
          mark(lst->tail);
        }
      }
    }
  }
}

void VM::mark(){
  // mark value stack
  for(auto i=entry;i<sp;i++){
    mark(values[i]);
  }
}

void VM::sweep(){
  Obj** pp = &objchain;
  while(*pp){
    if((*pp)->flag == UNUSE){
      auto dp = *pp;
      //DEBUG("delete ");
      //inspectObj(dp);cout<<endl;
      *pp = dp->next;
      delete dp;
    }else{
      (*pp)->flag = UNUSE;//reset
      pp = &((*pp)->next);
    }
  }
}

}
