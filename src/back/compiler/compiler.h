#ifndef __compiler__
#define __compiler__
#include "front/parser.h"
#include "vm/coder.h"
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
    uint32_t set(string const&);
    uint32_t get(string const&);
    bool has(string const&);
  };

  struct Closure{
    unordered_map<string,size_t> map;
    vector<vm::Coder::CapturedValue> values;
    uint32_t set(string const&,bool iflocal,uint32_t idx);
    uint32_t get(string const&);
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

  uint32_t emitFunc(string const&, int,
                    std::function<void(void)> f);
  void pushFunc(Local&, string const&, int,
                std::function<void(void)> f);

  inline void pushTopFunc(string const&,int,
                          std::function<void(void)> f);

  void pushVar(Local&,string const&,
                         std::function<void(void)> f);

  // helper
  template <typename... Arg>
  void error(Arg... args);

  Local toplevel;
  vector<Local> locals;
  vector<Closure> closures;

  unordered_map<string,uint32_t> syscallTable;
  unordered_map<string,std::function<void(ExprPtrList)>> keywords;
  int error_num = 0;
};

}
#endif
