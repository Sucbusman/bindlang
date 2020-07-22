#include <iostream>
#include <sstream>
#include "scanner.h"
using namespace bindlang;

int main(){
  //std::istringstream iss;
  auto scn = Scanner(std::cin);
  while(true){
    Token tok = scn.nextToken();
    if(tok.type == tok_eof){
      std::cout<<"bye~"<<std::endl;
      break;
    }
    std::cout<<tok<<std::endl;
  }
  return 0;
}
