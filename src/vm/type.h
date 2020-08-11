#ifndef __type__
#define __type__
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace bindlang::vm{

struct Env;//declare ahead
using EnvPtr = std::shared_ptr<Env>;
using std::cout,std::endl,std::cerr,std::string;
using std::vector,std::unordered_map;
using std::move;

template <typename T,typename U>
T* PtrCast(U *ptr){
  return reinterpret_cast<T*>(ptr);
}

}

#endif
