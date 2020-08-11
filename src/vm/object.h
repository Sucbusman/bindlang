#ifndef __object__
#define __object__
#include "value.h"

namespace bindlang::vm{
  
ObjString*    make_obj(string&);
ObjString*    make_obj(const char*);

}
#endif
