#ifndef __compiler__
#define __compiler__
#include "front/parser.h"
#include "vm/coder.h"
#include <unordered_map>
namespace bindlang{

class Compiler{
 public:
  Compiler(){standardEnvironment();}
  void runFile(string const&);
  void compile(ExprPtr);
  void writeBinary(string const&);
 private:
  struct Local{
    int counter=0;
    unordered_map<string,int> map;
    int  set(string const&);
    int  get(string const&);
    bool has(string const&);
  };

  struct CapturedVal{
    int distance;
    int idx;
  };

  struct Closure{
    unordered_map<string,size_t> map;
    vector<CapturedVal> values;
    int  set(string const&,int distance, int idx);
    bool has(string const&);
    void clear();
  };

  // change compile time environtment only
  void beginScope(vector<string> const& args);
  void beginScope();
  void endScope();
  Local& curScope();

  void resolve(ExprPtr);
  void resolveDefine(ExprPtr);
  void resolveId(ExprPtr);

  // gen code(& compile time)
  void standardEnvironment();
  void compileAtom(ExprPtr);
  void compileId(ExprPtr);
  void compileDefine(ExprPtr);
  void compileFunc(ExprPtr);
  void compileCall(ExprPtr);

  uint32_t emitFunc(string const&, int,
                    std::function<void(void)> f);
  void pushFunc(Local&, string const&, int,
                std::function<void(void)> f);

  // helper
  template <typename... Arg>
  void error(Arg... args);
  bool hasError();

  Local toplevel;
  vector<Local> locals;
  Closure closure;

  unordered_map<string,uint32_t> syscallTable;
  int error_num = 0;
  vm::Coder coder;
};

}
#endif
