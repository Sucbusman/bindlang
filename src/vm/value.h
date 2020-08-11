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
  f(GETL) f(GETG) f(SETL) f(SETG) f(FUN) \
  f(CNST) f(PUSH) f(POP) f(RET) f(JUMP) \
  f(PRINT)
#define VM_EXPAND_LIST(i) i,
#define VM_EXPAND_LABEL(i) &&VML_##i,
#define VM_LABEL(l) VML_##l:

namespace bindlang  { namespace vm{

enum class OpCode : uint32_t{
  VM_INSTALL_ALL(VM_EXPAND_LIST)
};

enum class objType : uint8_t {
  String
};

struct Obj{
  uint8_t type:6;
  uint8_t flag:2;
  struct Obj *next;
  Obj(uint8_t type,Obj* next):type(type),next(next){}
};

struct ObjString:Obj{
  ObjString(string s,Obj *next)
    :Obj((uint8_t)objType::String,next),s(s){}
  string s;
};

#define AS_OBJ(v) (v.as.obj)
#define AS_STRING(v) (&((ObjString*)(AS_OBJ(v)))->s)

typedef enum{
  VAL_NIL,
  VAL_BOOL,
  VAL_NUMBER,
  VAL_String
}val_type;

struct Value{
  uint8_t type:7;
  uint8_t immutable:1;
  union{
    bool boolean;
    int32_t number;
    Obj *obj;
  }as;
  Value():type(VAL_NIL),immutable(0){}
  Value(bool boolean)
    :type(VAL_BOOL),immutable(0){as.boolean=boolean;}
  Value(uint32_t number)
    :type(VAL_NUMBER){as.number = number;}
#define OBJ2VAL(T,m)                               \
  Value(Obj##T *obj):type(VAL_##T){as.obj = obj;immutable=m;}
  OBJ2VAL(String,0);
};

using ValPtr = std::shared_ptr<Value>;

void printVal(Value val);

struct Inst{
  uint32_t opc:VM_INST_OPC_WIDTH;
  uint32_t opr:VM_INST_OPR_WIDTH;
};
  
} }

#endif
