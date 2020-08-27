#ifndef __VM_DEFINE_H_
#define __VM_DEFINE_H_
#include <memory>
#include <unordered_map>
#include <vector>
#include <stdint.h>
#include "util/utils.h"
#include "vm/type.h"

#define VM_INSTALL_ALL_INST(f) \
  f(START) f(NOP)                                       \
  f(GETL) f(SETC) f(GETC) f(SETL)                       \
  f(CNST) f(CNSH) f(IMM)                                \
  f(PUSH) f(POP)                                        \
  f(CAPTURE) f(CALL) f(VCALL)                           \
  f(TCALL)f(MCALL)   f(RET)  f(VRET) f(MRET)            \
  f(JMP)  f(JNE)     f(JEQ)                             \
  f(ADD)  f(MINUS)   f(MULT)  f(DIVIDE)                 \
  f(ADDN) f(MINUSN)  f(MULTN) f(DIVIDEN)                \
  f(EQ)   f(GT)      f(LT)                              \
  f(TRUE) f(FALSE)   f(NOT)                             \
  f(UNIT) f(RCONS) f(CONS) f(HEAD) f(TAIL) f(EMPTYP)    \
  f(LEN)  f(TAKE)  f(CONCAT)                            \
  f(INT2STR) f(STR2INTS) f(INT2FILE)                    \
  f(HALT) f(SYSCALL) f(COPY)
  
#define VM_EXPAND_LIST(i) i,
#define VM_EXPAND_LIST_STR(i) #i,
#define VM_EXPAND_LABEL_LIST(i) &&VML_##i,
#define VM_LABEL(i) VML_##i:
#define VM_INSTALL_ALL_VAL(f)                   \
  f(NIL) f(BOOL) f(NUMBER) f(FILE) f(CHAR)      \
  f(String) f(Procedure) f(List)
#define VM_EXPAND_VAL(i) VAL_##i,
#define VM_INSTALL_ALL_OBJ(f) \
  f(String) f(Procedure) f(List)

namespace bindlang  { namespace vm{

extern const char* opcTable[];
extern const char* valTable[];
extern const char* objTable[];

enum class OpCode : uint8_t{
  VM_INSTALL_ALL_INST(VM_EXPAND_LIST)
};

enum class objType : uint8_t {
  VM_INSTALL_ALL_OBJ(VM_EXPAND_LIST)
};

struct Obj{
  uint8_t type:6;
  uint8_t flag:2;
  struct Obj *next;
  Obj(uint8_t type,Obj* next):type(type),flag(0b00),next(next){}
  virtual Obj* clone()=0;
  virtual ~Obj();
};

struct ObjString:Obj{
  ObjString(string s,Obj *next)
    :Obj((uint8_t)objType::String,next),s(s){}
  string s;
  Obj* clone() override;
};

#define isObj(v) (v.type>VAL_NUMBER)
#define AS_OBJ(v) (v.as.obj)
#define AS_CSTRING(v) (&((ObjString*)(AS_OBJ(v)))->s)
#define AS_STRING(v) ((ObjString*)AS_OBJ(v)s)
#define AS_PROCEDURE(v) ((ObjProcedure*)AS_OBJ(v))
#define AS_LIST(v) ((ObjList*)AS_OBJ(v))

typedef enum{
  VM_INSTALL_ALL_VAL(VM_EXPAND_VAL)
}val_type;

struct ObjProcedure;
struct ObjList;
struct Value{
  uint8_t type;
  union{
    bool boolean;
    char char_;
    int64_t number;
    uint16_t filedes;
    uint64_t address;
    Obj *obj;
  }as;
  Value():type(VAL_NIL){}
  Value(bool boolean)
    :type(VAL_BOOL){as.boolean=boolean;}
  Value(uint32_t number)
    :type(VAL_NUMBER){as.number = number;}
  Value(char ch)
    :type(VAL_NUMBER){as.char_ = ch;}
  Value(uint64_t number)
    :type(VAL_NUMBER){as.number = number;}
  Value(uint16_t fd)
    :type(VAL_FILE){as.filedes = fd;}
#define OBJ2VAL(T)                               \
  Value(Obj##T *obj):type(VAL_##T){as.obj = (Obj*)obj;}
  OBJ2VAL(String);
  OBJ2VAL(Procedure);
  OBJ2VAL(List);
  bool operator== (Value const& v) const;
  size_t to_hash() const{
    size_t ans = type;
    switch(type){
      case VAL_NIL: return ans;
      case VAL_BOOL: return ans|((as.boolean & 1)<<8);
      default:
        return (ans<<(63)|(size_t)as.obj);
    }
  }
#undef OBJ2VAL
};

struct ObjProcedure:Obj{
  ObjProcedure(string name,uint8_t arity,
               uint32_t offset,uint8_t property,Obj *next)
    :Obj((uint8_t)objType::Procedure,next),
     name(name),arity(arity),offset(offset),property(property){}
  Obj* clone() override;
  string name;
  uint8_t arity;
  size_t offset;//offset to the procedure start in rom
  vector<Value> captureds;
  //            number,     result, args
  CallCache cache;
  uint8_t property;//....|side-effect
  bool hasSideEffect(){
    return (property==1); 
  }
  bool multiArgsp(){
    return ((property>>1)==1); 
  }
  size_t hash_call(vector<Value> const& args){
    #define hash_mult 0x1234
    size_t n=0;
    for(auto const& v:captureds){
      n += v.to_hash()*hash_mult;
    }
    for(auto const& v:args){
      n += v.to_hash()*hash_mult;
    }
    return n;
    #undef hash_mult
  }
};

struct ObjList:Obj{
  ObjList(Value head,ObjList * tail,Obj *next)
    :Obj((uint8_t)objType::List,next),head(head),tail(tail){}
  Value head;
  ObjList* tail;
  Obj* clone() override;
};

// value helper
void printVal(Value const& val);
void inspectVal(Value const& val);
bool truthy(Value const& val);
bool valueEqual(Value const&,Value const&);

// object helper
extern size_t bytesAllocated;
extern Obj* objchain;
void printObj(Obj*);
void inspectObj(Obj*);
void printList(ObjList*,std::function<void(Value const&)> f);
ObjString* make_obj(string&);
ObjString* make_obj(const char*);
ObjProcedure* make_obj(string const&,uint8_t,size_t,uint8_t);
ObjProcedure* make_obj(string const&,uint8_t,size_t);
ObjList* make_obj(Value const& head,ObjList* tail);
Value copy_val(Value const& v);

} }

#endif
