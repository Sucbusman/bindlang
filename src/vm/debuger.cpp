#include "vm.h"
namespace bindlang::vm{

void printIndent(int n){
  while(n--)
    cout<<' ';
}

void VM::dumpRegs(){
  cout<<"  frame:"<<frames.size()
      <<" sp:0x"<<hex<<sp<<" bp:0x"<<hex<<bp;
  cout<<endl;
}

void VM::traceFrames(){
  for(auto &frame:frames){
    cout<<"in procedure:"<<frame.name;
    showLine((frame.ip-rom.data()));
    cout<<endl;
  }
}

void VM::showLine(size_t pc){
  if(lines.size()>0){
    if(pc<lines[0].second) cout<<" at prelude ";
    else{
      for(auto i=1;i<lines.size();i++){
        auto p = lines[i];
        if(pc<p.second){
          cout<<" at line "<<dec<<p.first;
          goto OUT;
        }
      }
      cout<<" at line "<<dec<<(*lines.rbegin()).first;
    OUT:{}
    }
  }
}

void VM::dumpLines(){
  cout<<"lines:"<<endl;
  for(auto &p:lines){
    cout<<dec<<p.first<<' '<<p.second<<endl;
  }
}

void VM::dumpStack(){
  //sp point to next slot to place coming value
  //so, sp-1 is the available value at stack top
  for(int it=values.size()-1;it>=0;it--){
    cout<<dec<<setw(3)<<it<<' ';
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
