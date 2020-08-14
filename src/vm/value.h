#ifndef __define__
#define __define__
#include <memory>
#include <unordered_map>
#include <vector>
#include <stdint.h>
#include "type.h"
#include "../utils.h"

#define VM_INST_OPC_WIDTH 5
#define VM_INST_OPR_WIDTH (32-VM_INST_OPC_WIDTH)
#define VM_INST_IMM_MASK  ((1<<VM_INST_OPR_WIDTH)-1)

#define VM_INSTALL_ALL_INST(f) \
  f(GETL) f(GETG)    f(SETL) f(SETG)                  \
  f(CNST) f(CNSH)    f(IMM)                           \
  f(PUSH) f(POP)                                      \
  f(FUN)  f(CALL)    f(TCALL)    f(RET)               \
  f(JMP)  f(JNE)                                      \
  f(ADD)  f(MINUS)   f(MULT) f(DIVIDE)                \
  f(EQ)   f(GT)      f(LT)                            \
  f(HALT) f(SYSCALL) 
  
#define VM_EXPAND_LIST(i) i,
#define VM_EXPAND_LIST_STR(i) #i,

#define VM_INSTALL_ALL_VAL(f)\
  f(NIL) f(BOOL) f(NUMBER) f(String) f(Procedure)
#define VM_EXPAND_VAL(i) VAL_##i,

namespace bindlang  { namespace vm{

enum class OpCode : uint8_t{
  VM_INSTALL_ALL_INST(VM_EXPAND_LIST)
};

extern const char* opcTable[];
extern const char* valTable[];

enum class objType : uint8_t {
  String,
  Procedure
};

struct Obj{
  uint8_t type:6;
  uint8_t flag:2;
  struct Obj *next;
  Obj(uint8_t type,Obj* next):type(type),next(next){}
  virtual Obj* clone(Obj** pchain)=0;
};

struct ObjString:Obj{
  ObjString(string s,Obj *next)
    :Obj((uint8_t)objType::String,next),s(s){}
  string s;
  Obj* clone(Obj** pchain) override {
    auto o = new ObjString(s,*pchain);
    *pchain = o;
    return o;
  }
};

struct ObjProcedure:Obj{
  ObjProcedure(string name,int arity,
               uint32_t offset,Obj *next)
    :Obj((uint8_t)objType::Procedure,next),
     name(name),arity(arity),offset(offset){}
  Obj* clone(Obj** pchain) override {
    auto o = new ObjProcedure(name,arity,offset,*pchain);
    *pchain = o;
    return o;
  }
  string name;
  int arity;
  size_t offset;//offset to the procedure start in rom
};

#define AS_OBJ(v) (v.as.obj)
#define AS_CSTRING(v) (&((ObjString*)(AS_OBJ(v)))->s)
#define AS_STRING(v) ((ObjString*)AS_OBJ(v)s)
#define AS_PROCEDURE(v) ((ObjProcedure*)AS_OBJ(v))

typedef enum{
  VM_INSTALL_ALL_VAL(VM_EXPAND_VAL)
}val_type;

struct Value{
  uint8_t type;
  union{
    bool boolean;
    int64_t number;
    uint64_t address;
    Obj *obj;
  }as;
  Value():type(VAL_NIL){}
  Value(bool boolean)
    :type(VAL_BOOL){as.boolean=boolean;}
  Value(uint32_t number)
    :type(VAL_NUMBER){as.number = number;}
  Value(uint64_t number)
    :type(VAL_NUMBER){as.number = number;}
#define OBJ2VAL(T)                               \
  Value(Obj##T *obj):type(VAL_##T){as.obj = obj;}
  OBJ2VAL(String);
  OBJ2VAL(Procedure);
};

void printVal(Value const& val);
void inspectVal(Value const& val);
bool truthy(Value const& val);
bool valueEqual(Value const&,Value const&);

class Env{
 public:
  Env(){}
  Env(EnvPtr outer):outer(outer){}
  Env(Env const& e){
    outer = e.outer;
    map  = unordered_map<string,Value>(e.map);
  }
  Value*   get(string);
  void     set(string,const Value&);
  unordered_map<std::string,Value> map;
  EnvPtr outer;
};

// gc
extern size_t bytesAllocated;
extern Obj* objchain;
void ifgc();
void recycleMem();
ObjString* make_obj(string&);
ObjString* make_obj(const char*);
ObjProcedure* make_obj(string name,int arity,size_t offset);
Value copy_val(Value const& v);

} }

#endif
