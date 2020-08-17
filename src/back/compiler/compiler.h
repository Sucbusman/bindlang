#ifndef __compiler__
#define __compiler__
#include "front/parser.h"
#include "vm/coder.h"
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

  template <typename... Arg>
  void error(Arg... args);
  bool hasError();



  Local toplevel;
  vector<Local> locals;

  unordered_map<string,uint32_t> syscallTable;
  int error_num = 0;
  vm::Coder coder;
};

}
#endif
