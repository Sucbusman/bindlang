#ifndef __token__
#define __token__
#include <iostream>

const char oneCharTokens[]=
{
  '(',')',
  '{','}',
  '[',']',
  ':',';','.',',',
  '+','*','/','|'
};

const char *toktype_str_list[] = {
  "tok_eof"  , 
  "tok_id"   , 
  "tok_num"  , 
  "tok_str"  , 
  "tok_err"  , 
  "tok_arrow", 
  "tok_equal"                          
};

typedef enum 
{
  tok_eof   = -1,
  tok_id    = -2,
  tok_num   = -3,
  tok_str   = -4,
  tok_err   = -5,
  tok_arrow = -6,
  tok_equal = -7
} toktype;

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

#endif
