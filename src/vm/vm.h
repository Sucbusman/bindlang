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
  VM(vector<std::uint8_t> &&codes,vector<Value> &&constants)
    :rom(codes),constants(constants){pc=rom.data();}
  void init(vector<std::uint8_t>&&codes,vector<Value>&&constants);

  void reset();
  bool run();
  void disassemble();

  // helper
  template<class... Args>
  bool error(Args... args);

  // vm
  vector<std::uint8_t> rom;
  vector<Value>        values;
  uint8_t* pc;
  Value reg_val;

  // table
  unordered_map<std::string,Value> globals;
  vector<Value> constants;

  // gc
  size_t bytesAllocated;
  Obj* objchain;
  ObjString* make_obj(const char* s);
  ObjString* make_obj(string& s);
  void ifgc();
  void recycleMem();
  // value
  Value      copy_val(Value const& v);
  Obj*       copy_obj(Obj* obj);

};

} }

#endif
