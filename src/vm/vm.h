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
enum class SYSCALL_TYPE{
  PRINT,
  INSPECT,
  GC
};

struct callFrame{
  // is used for recording the past calls information
  callFrame(uint8_t* ip,size_t bp,vector<Value>* vsp)
    :ip(ip),bp(bp),vsp(vsp){}
  uint8_t* ip;
  size_t   bp;
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
  bool debugp = false;
  void disassemble();
 private:
  // vm
  vector<std::uint8_t> rom;
  vector<Value>        constants;
  
  // value stack
  vector<Value>        values;
  inline void  push(Value const& val);
  inline Value pop();
  vector<callFrame> frames;// last call frames
  uint32_t entry=0;// determin constant region

  // now only for "let"
  vector<size_t> vframes;


  // register
  uint8_t* ip;
  size_t   bp = 0;
  size_t   sp = 0;
  vector<Value>* vsp = nullptr;

  // syscall
  vector<std::function<bool()>> syscalls;
  void standardSyscalls();

  // memorization
  ObjProcedure*   cur_proc=nullptr;
  size_t          cur_callid;
  vector<Value>   cur_args;

  // gc
  size_t  GC_THRESHOLD = 0xffffffff;//16 MB
  void prepareGc();
  void ifGc();
  void recycleMem();
  void mark();
  void mark(Obj*);
  void mark(Value &);
  void sweep();

  // debug
  uint8_t* disas_inst(uint8_t*);
  void dumpConstant();
  void dumpStack();
  void dumpRegs();
  
  // error
  int error_num=0;
  template<typename... Args>
  bool error(Args... args);
  inline bool hasError();
};


} }

#endif
