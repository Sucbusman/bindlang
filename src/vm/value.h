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

#define VM_INSTALL_ALL(f) \
  f(GETL) f(GETG) f(SETL) f(SETG) f(FUN)    \
  f(CNST) f(PUSH) f(POP) f(RET) f(JUMP)     \
  f(CALL) f(TCALL) f(HALT) f(SYSCALL)
#define VM_EXPAND_LIST(i) i,
#define VM_EXPAND_LABEL(i) &&VML_##i,
#define VM_LABEL(l) VML_##l:

namespace bindlang  { namespace vm{

enum class OpCode : uint32_t{
  VM_INSTALL_ALL(VM_EXPAND_LIST)
};

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
               word offset,Obj *next)
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
  VAL_NIL,
  VAL_BOOL,
  VAL_NUMBER,
  VAL_String,
  VAL_Procedure
}val_type;

struct Value{
  uint8_t type;
  union{
    size_t address;
    bool boolean;
    int32_t number;
    Obj *obj;
  }as;
  Value():type(VAL_NIL){}
  Value(size_t address)
    :type(VAL_NUMBER){as.address = address;}
  Value(bool boolean)
    :type(VAL_BOOL){as.boolean=boolean;}
  Value(uint32_t number)
    :type(VAL_NUMBER){as.number = number;}
#define OBJ2VAL(T)                               \
  Value(Obj##T *obj):type(VAL_##T){as.obj = obj;}
  OBJ2VAL(String);
  OBJ2VAL(Procedure);
};

void printVal(Value val);
struct Inst{
  uint32_t opc:VM_INST_OPC_WIDTH;
  uint32_t opr:VM_INST_OPR_WIDTH;
};

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
