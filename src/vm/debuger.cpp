#include "vm.h"
namespace bindlang::vm{

void printIndent(int n){
  while(n--)
    cout<<' ';
}

void VM::dumpRegs(){
  cout<<"  frame:"<<frames.size()
      <<" sp:0x"<<hex<<sp<<" bp:0x"<<hex<<bp<<" val_reg:";
  cout<<endl;
}

void VM::dumpStack(){
  //sp point to next slot to place coming value
  //so, sp-1 is the available value at stack top
  for(int it=values.size()-1;it>=0;it--){
    if(it==bp and it==sp-1){
      printIndent(3);
      cout<<"=";
    }
    else if(it==bp){
      printIndent(3);
      cout<<'_';
    }
    else if(it==sp-1){
      printIndent(3);
      cout<<'>';
    }
    else {
      printIndent(4);
    }
    inspectVal(values[it]);
    cout<<endl;
  }
  cout<<"    ------------------------------------"<<endl;
}

}
