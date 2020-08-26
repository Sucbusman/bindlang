#ifndef __token__
#define __token__
#include <iostream>
namespace bindlang{

extern const char oneCharTokens[];
extern const char *toktype_str_list[];

typedef enum 
{
  tok_eof     = -1,
  tok_id      = -2,
  tok_num     = -3,
  tok_num_bin = -4,
  tok_num_oct = -5,
  tok_num_hex  = -6,
  tok_str     = -7,
  tok_err     = -8,
  tok_arrow   = -9,
  tok_equal   = -10,
  tok_newline = -11
} toktype;

struct Token{
  int line;
  int type;
  std::string literal;
  Token(int line,toktype type,std::string literal)
    :line(line),type((int)type),literal(literal){}
  Token(int line,char type,std::string literal)
    :line(line),type((int)type),literal(literal){}
  Token():line(0),type(' '),literal(std::string()){}
  void show();
};

std::ostream & operator<<(std::ostream &out, const Token &tok);

}

#endif
