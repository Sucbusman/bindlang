#ifndef __lexer__
#define __lexer__
#include <istream>
#include <functional>
#include "queue.h"
#include "utils.h"

namespace bindlang {

  typedef enum 
   {
    tok_eof   = -1,
    tok_id    = -2,
    tok_num   = -3,
    tok_str   = -4,
    tok_err   = -5,
    tok_arrow = -6,
    tok_equal = -7
   }toktype;

  struct Token{
    int line;
    int type;
    std::string literal;
    Token(int line,toktype type,std::string literal)
      :line(line),type((int)type),literal(literal){}
    Token(int line,char type,std::string literal)
      :line(line),type((int)type),literal(literal){}
  };
  
  std::ostream & operator<<(std::ostream &out, const Token &tok);

  class Scanner{
   public:
    Scanner(std::istream &in):in(in),
       cache(std::string()),
       /* we borrow at most 10 chars from future */
       future(law::fix_queue<char>(10))
    {
      reset();
    }
    void  reset();
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
