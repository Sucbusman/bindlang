#ifndef __vm__
#define __vm__
#include <cstdint>
#include <cstdarg>
#include <stack>
#include <vector>
#include <string>
#include "value.h"

namespace bindlang { namespace vm{

class VM{
 public:
  VM(){}
  VM(vector<std::uint8_t> &&codes,
     vector<Value> &&constants)
    :rom(codes),constants(constants){pc=rom.data();}
  void init(vector<std::uint8_t>&&codes,
            vector<Value>&&constants);

  void reset();
  bool run();
  void disassemble();

  // helper
  template<class... Args>
  bool error(Args... args);

  // vm
  vector<std::uint8_t> rom;
  vector<Value>        constants;
  uint8_t* pc;

  // value stack
  vector<Value>        values;
  void  push(Value val);
  Value pop();

  // regs
  Value reg_val;
  word  base_ptr;
  word  stack_ptr;

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
