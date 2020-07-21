#ifndef __utils__
#define __utils__
#include <ostream>
using namespace std;
#define DEFAULT "\033[0m"
#define GREEN(s) ("\033[0;36m" s DEFAULT)
#define RED(s) ("\033[0;31m" s DEFAULT)
#define BLUE(s) ("\033[0;34m" s DEFAULT)
#define DEBUG(s) \
  do{cerr<<RED("[debug] ")<<s<<endl;}while(0)
#endif
