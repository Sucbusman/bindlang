#ifndef __compiler__
#define __compiler__
#include "front/parser.h"
#include "vm/coder.h"
#include <bits/stdint-uintn.h>
#include <unordered_map>
namespace bindlang{

class Compiler{
 public:
  Compiler(){standardEnvironment();}
  void compileFile2mem(string const&);
  void compileFile(string const&);
  void compile(ExprPtr);
  void writeBinary(string const&);
  bool hasError();
  vm::Coder coder;
 private:
  struct Local{
    uint32_t counter=0;
    unordered_map<string,uint32_t> map;
    uint16_t set(string const&);
    uint32_t tellp();
    uint16_t get(string const&);
    void sweep(uint32_t delim);
    bool has(string const&);
  };

  struct Closure{
    unordered_map<string,size_t> map;
    vector<vm::Coder::CapturedValue> values;
    uint8_t set(string const&,bool iflocal,uint8_t idx);
    uint8_t get(string const&);
    bool has(string const&);
    void clear();
  };

  // change compile time environtment only
  void beginScope(vector<string> const& args);
  void beginScope();
  void endScope();
  Local& curScope();

  void resolve(ExprPtr);
  void resolveList(ExprPtr);
  void resolveDefine(ExprPtr);
  void resolveId(ExprPtr);
  void resolveCall(ExprPtr);

  // gen code(& compile time)
  void standardEnvironment();
  void compileAtom(ExprPtr);
  void compileList(ExprPtr);
  void compileId(ExprPtr);
  void compileDefine(ExprPtr);
  void compileFunc(ExprPtr);
  void compileCall(ExprPtr);

  uint32_t emitFunc(string const&,uint8_t,uint8_t,
                    std::function<void(void)> f,bool copyp=true);

  void pushFunc(Local&, string const&, uint8_t,uint8_t,
                std::function<void(void)> f);

  inline void pushTopFunc(string const&,uint8_t,uint8_t,
                          std::function<void(void)> f);

  void pushVar(Local&,string const&,
                         std::function<void(void)> f);

  // helper
  template <typename... Arg>
  void error(Arg... args);

  unordered_map<string,size_t> const_strings;
  size_t CNSTStr(string);

  //if compiling a root expression
  //for unneeded expression elimition
  bool rootp = true;
  //for give function a name
  bool define_funcp = false;
  
  string last_name;

  Local toplevel;
  vector<Local> locals;
  vector<Closure> closures;

  //unordered_map<string,uint8_t> syscallTable;
  unordered_map<string,std::function<void(ExprPtrList)>> keywords;
  int error_num = 0;

  //modules
  vector<string> files;
};

}
#endif
