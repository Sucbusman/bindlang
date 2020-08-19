#ifndef __vm__
#define __vm__
#include <cstdint>
#include <cstdarg>
#include <stack>
#include <vector>
#include <string>
#include "vm/value.h"
#include "vm/type.h"

namespace bindlang { namespace vm{
typedef enum{
  PRINT
}SYSCALL_TABLE;

struct callFrame{
  callFrame(uint8_t* ip,size_t bp,size_t sp,vector<Value>* vsp)
    :ip(ip),bp(bp),sp(sp),vsp(vsp){}
  uint8_t* ip;
  size_t   bp;
  size_t   sp;
  vector<Value>* vsp;
};

class VM{
 public:
  VM(){}
  VM(vector<std::uint8_t> &&codes,
     vector<Value> &&constants)
    :rom(codes),constants(constants){
    reset();
  }
  void init(vector<std::uint8_t>&&codes,
            vector<Value>&&constants);

  void reset();
  bool run();
  uint8_t* disas_inst(uint8_t*);
  void dumpConstant();
  void disassemble();
  
  // helper
  template<class... Args>
  bool error(Args... args);
  void dumpStack();
  void dumpRegs();

  // vm
  vector<std::uint8_t> rom;
  vector<Value>        constants;
  
  // value stack
  vector<Value>        values;
  void  push(Value val);
  Value pop();

  // register
  uint8_t* ip;
  Value    reg_val;
  size_t   bp = 0;
  size_t   sp = 0;
  vector<Value>* vsp = nullptr;
  bool     debug = false;

  // last call frames
  vector<callFrame> frames;

  // syscall
  using  thunk = bool(*)(VM&);
  vector<thunk> syscalls;
  void standardSyscalls();
  
  // env
  Env toplevel;
  stack<Env> envs;
  void standardEnv();
};

bool sys_print(VM& vm);
bool sys_inspect(VM& vm);

} }

#endif
