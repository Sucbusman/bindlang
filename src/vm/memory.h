#ifndef __memory__
#define __memory__
#include "object.h"
namespace bindlang::vm::mem{

  ObjString* make_obj(const char* s);
}
#endif
