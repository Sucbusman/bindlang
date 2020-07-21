#ifndef __queue__
#define __queue__
#include <stdlib.h>
#include <iostream>
namespace law{
  template <class T>
  class fix_queue{
  public:
    fix_queue(int capacity):capacity(capacity),start(0),end(0),number(0){
      memory = (T*)malloc(capacity*sizeof(T));
    }
    ~fix_queue(){
      //free(memory);
    }
    bool empty(){
      return number==0;
    }
    bool full(){
      return number==capacity;
    }
    void push(T item){
      if(number<capacity){
        number++;
        memory[end] = item;
        end = (end+1)%capacity;
      }else{
        std::cerr<<"queue push error"<<std::endl;
        exit(1);
      }
    }
    T& back(){
      if(number>0){
        return memory[(end-1)%capacity];
      }else{
        std::cerr<<"queue back error"<<std::endl;
        exit(1);
      }
    }
    T& pop(){
      if(number>0){
        number--;
        int pos=start;
        start = (start+1)%capacity;
        return memory[pos];       
      }else{
        std::cerr<<"queue pop error"<<std::endl;
        exit(1);
      }
    }
    T& front(){
      if (number>0){
        return memory[start];
      }else{
        std::cerr<<"queue front error"<<std::endl;
        exit(1);
      }
    }
    void inspect(){
      if(not empty()){
        for(int i=start;i!=end;i=(i+1)%capacity){
          std::cout<<"queue item:"<<memory[i]<<std::endl;
        }
      }else{
        std::cout<<"queue empty"<<std::endl;
      }
    }
  private:
    T   *memory;
    int capacity;
    int start;
    int end;
    int number;
  };
}

#endif
