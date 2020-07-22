#ifndef __value__
#define __value__
#include <string>

struct Value {
  int type;
  union{
    double number;
    std::string string;
    
  }as;
};

#endif
