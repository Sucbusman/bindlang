#ifndef __assembler__
#define __assembler__
#include <iostream>
#include <string>
#include <functional>
#include "vm/coder.h"
namespace bindlang::vm {

/* token is a special string 
   with a notice as first char
   @:id
   ":string
   $:number 
   =:delimline
   other:opcode */

class Assembler{
/* bindlang assembly parser 
   one pass from ascii to data structure */
public:
  Assembler(std::istream &in)
    :in(in){
    loadOp();
    in>>std::noskipws;
    in>>lchar;
    cache.push_back(lchar);
    in>>nchar;
  }
  void write(const char* path);
  void scan();
  Coder coder;
private:
  template <class... Args>
  bool error(Args...args);
  bool hasError();
  void reset();

  void   parse();
  void   parseConstant();
  void   parseCode();
  void   fillConstant();
  string expect(char type);

  string  token();
  string  makeTok();
  string  tokNumber();
  string  tokString();
  string  tokLabel();
  string  tokOperator();
  string  tokDelimline();
  inline string  tokEof();
  void    skipWhitespace();
  
  bool atEnd();
  void eat();
  char peek();
  bool skipUntil(std::function<bool(char)>);

  void loadOp();
  
  struct func{
    string name;
    int    arity;
    size_t slot;
  };

  unordered_map<string,size_t> lables;
  unordered_map<string,std::function<bool(void)>> operators;
  unordered_map<string,vector<size_t>> unfills;
  
  vector<func>  funcs;
  std::istream &in;
  char nchar;
  char lchar;
  int  line_num=0;
  int  error_num=0;
  std::string cache;
};

}
#endif
