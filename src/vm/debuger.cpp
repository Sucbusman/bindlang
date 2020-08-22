#include "vm.h"
namespace bindlang::vm{

void printIndent(int n){
  while(n--)
    cout<<' ';
}

void VM::dumpRegs(){
  cout<<"  frame:"<<frames.size()
      <<" sp:0x"<<hex<<sp<<" bp:0x"<<hex<<bp<<" val_reg:";
  printVal(reg_val);
  cout<<endl;
}

void VM::dumpStack(){
  for(int it=values.size()-1;it>=0;it--){
    if(it==bp and it==sp){
      printIndent(3);
      cout<<"=";
    }
    else if(it==bp){
      printIndent(3);
      cout<<'_';
    }
    else if(it==sp){
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
