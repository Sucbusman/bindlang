#ifndef __type__
#define __type__
#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace bindlang::vm{

class  Env;//declare ahead
class  VM;
using  EnvPtr = std::shared_ptr<Env>;
using  std::cout,std::endl,std::cerr,std::string;
using  std::setw,std::hex;
using  std::vector,std::unordered_map;
using  std::move;

template <typename T,typename U>
T* PtrCast(U *ptr){
  return reinterpret_cast<T*>(ptr);
}

}

#endif
