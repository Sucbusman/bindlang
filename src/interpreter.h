#ifndef __interpreter__
#define __interpreter__
#include "parser.h"

namespace bindlang {

class Interpreter{
 public:
  Interpreter(Parser &parser):parser(parser){reset();}
  void reset(){}
 private:
  Parser& parser;
};

  
}

#endif
