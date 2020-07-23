#ifndef __value__
#define __value__
#
struct Value{
  int type;
  union{
    double number;
    char* string;
  }as;
};
#endif
