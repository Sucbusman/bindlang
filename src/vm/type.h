#ifndef __VM_TYPE_H__
#define __VM_TYPE_H__
#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace bindlang::vm{

class  VM;
struct Value;
using  std::cout,std::endl,std::cerr,std::string;
using  std::setw,std::hex;
using  std::vector,std::unordered_map;
using  std::move;
using CachePair = std::pair<Value,vector<Value>>;
using CallCache = unordered_map<size_t,CachePair>;

template <typename T,typename U>
T* PtrCast(U *ptr){
  return reinterpret_cast<T*>(ptr);
}

}

#endif
