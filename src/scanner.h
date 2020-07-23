#ifndef __lexer__
#define __lexer__
#include <istream>
#include <functional>
#include "queue.h"
#include "utils.h"
#include "token.h"

namespace bindlang {

class Scanner{
 public:
  Scanner(std::istream &in):in(in),
     cache(std::string()),
     /* we borrow at most 10 chars from future */
     future(law::fix_queue<char>(10)){}
  void  reset();
  Token tokNewline();
  Token tokIdentify();
  Token tokNumber();
  Token tokString();
  Token nextToken();
  void  debug();
 private:
  bool atEnd();
  bool isIdStart(char);
  bool error(const char *message);
  void nextChar();
  char borrow();
  bool skipUntil(std::function<bool(char)>);
  bool skipUntilNext(std::function<bool(char)>);
  void skipWhitespace();
  Token makeToken(toktype type);
  Token makeToken(char type);

  std::istream &in;
  char char_;
  int line_num;
  std::string cache;
  law::fix_queue<char> future;
};

} // namespace bindlang

#endif
