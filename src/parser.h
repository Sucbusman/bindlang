#ifndef __parser__
#define __parser__
#include <stack>
#include "scanner.h"
#include "ast.h"
#include "utils.h"
#include "queue.h"

namespace bindlang {

class Parser {
 public:
  Parser(Scanner &scanner):
    scanner(scanner),error_num(0),
    future(law::fix_queue<Token>(5)),
    protect(false){
    scanner.reset();
  }
  void reset(); // namespace bindlang
  ExprPtr parseNext();
  bool    atEnd();
  bool    hasError();
  bool    fine();

 private:
  Scanner& scanner;
  int     error_num;
  bool    error(const char *message);
  bool    use_protect();

  void    next();
  Token   borrow();
  Token   peekNext();
  
  bool    verifyId(Token const& tok);
  bool    verifyId();

  ExprPtr parseExpr();
  ExprPtr parseAtom();
  ExprPtr parseDefine();
  ExprPtr parseSet(ExprPtr);
  ExprPtr parseId();
  ExprPtr parseList();
  ExprPtr parseTuple();
  ExprPtr parseFunc();
  ExprPtr parseCall(ExprPtr callee=nullptr);
  ExprPtr parseDot(ExprPtr object=nullptr);

  Token token_;
  law::fix_queue<Token> future;
  bool  protect;
};

} // namespace bindlang
#endif
