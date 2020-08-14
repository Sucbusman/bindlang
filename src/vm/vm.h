#ifndef __vm__
#define __vm__
#include <cstdint>
#include <cstdarg>
#include <stack>
#include <vector>
#include <string>
#include "value.h"

namespace bindlang { namespace vm{
typedef enum{
  PRINT
}SYSCALL_TABLE;

struct callFrame{
  callFrame(uint8_t* ip,size_t bp,size_t sp)
    :ip(ip),bp(bp),sp(sp){}
  uint8_t* ip;
  size_t   bp;
  size_t   sp;
};

class VM{
 public:
  VM(){}
  VM(vector<std::uint8_t> &&codes,
     vector<Value> &&constants)
    :rom(codes),constants(constants){
    ip=rom.data();
  }
  void init(vector<std::uint8_t>&&codes,
            vector<Value>&&constants);

  void reset();
  bool run();
  uint8_t* disas_inst(uint8_t*);
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

  // call frame
  vector<callFrame> frames;

  // register
  uint8_t* ip;
  Value    reg_val;
  size_t   bp;
  size_t   sp;

  // syscall
  vector<thunk> syscalls;
  void standardSyscalls();
  
  // env
  Env toplevel;
  stack<Env> envs;
  void standardEnv();
};

bool sys_print(VM& vm);

} }

#endif
