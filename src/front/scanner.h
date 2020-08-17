#ifndef __scanner__
#define __scanner__
#include <istream>
#include <functional>
#include "util/queue.h"
#include "util/utils.h"
#include "front/token.h"

namespace bindlang {

class Scanner{
 public:
  Scanner(std::istream &in):in(in),
     cache(std::string()),
     /* we borrow at most 10 chars from future */
     future(fix_queue<char>(10)){}
  void  reset();
  Token tokNewline();
  Token tokIdentify();
  Token tokNumber();
  Token handleEscape(Token &);
  Token tokString();
  Token nextToken();
  void  debug();
 private:
  bool atEnd();
  bool isIdStart(char);
  template <typename ...Arg>
  bool error(Arg...);
  void nextChar();
  char borrow();
  char peekNext();
  bool skipUntil(std::function<bool(char)>);
  bool skipUntilNext(std::function<bool(char)>);
  void skipWhitespace();
  Token makeToken(toktype type);
  Token makeToken(char type);

  std::istream &in;
  char char_;
  int line_num;
  std::string cache;
  fix_queue<char> future;
};

} // namespace bindlang

#endif
