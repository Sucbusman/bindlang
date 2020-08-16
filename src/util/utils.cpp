#include <string>
#include "util/utils.h"
string prefix(string const& path){
  string pre = "";
  for(auto i=path.size()-1;i!=~0;i--){
    if(path[i] == '.'){
      for(unsigned long int j=0;j<i;j++){
        pre += path[j];
      }
    }
  }
  if(pre == "") pre = path;
  return pre;
}

string basename(string const& path){
  string rbase="";
  for(auto i = path.size()-1;i!=~0;i--){
    if(path[i] == '/'){
      break;
    }
    rbase.push_back(path[i]);
  }
  string base;
  for(auto i=rbase.size()-1;i!=~0;i--){
    base.push_back(rbase[i]);
  }
  return base;
}

string dirname(string const& path){
  string dir = "";
  for(auto i = path.size()-1;i>=0;i--){
    if(path[i] == '/'){
      for(long unsigned int j=0;j<=i;j++){
        dir += path[j];
      }
      break;
    }
  }
  return dir;
}
