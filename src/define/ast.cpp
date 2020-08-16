#include "define/ast.h"

namespace bindlang{
namespace STDOUT{
int idt = 0;
}
void showidt(){
  for(int i=0;i<STDOUT::idt;i++) cout<<' '; 
}
void indent(){STDOUT::idt+=2;};
void deindent(){if(STDOUT::idt>1) STDOUT::idt-=2;};


ExprPtr ExprAtom::clone(){
  return std::make_unique<ExprAtom>(literal);
}

ExprPtr ExprTuple::clone(){
  ExprPtrList newcontainer;
  for(auto const& i:container){
    newcontainer.push_back(i->clone());
  }
  return std::make_unique<ExprTuple>(move(newcontainer));
}

ExprPtr ExprList::clone(){
  ExprPtrList newcontainer;
  for(auto const& i:container){
    newcontainer.push_back(i->clone());
  }
  return std::make_unique<ExprList>(move(newcontainer));
}

ExprPtr ExprId::clone(){
  return std::make_unique<ExprId>(id);
}

ExprPtr ExprDefine::clone(){
  return std::make_unique<ExprDefine>(id,expr->clone());
}

ExprPtr ExprSet::clone(){
  return std::make_unique<ExprSet>(beset->clone(),expr->clone());
}

ExprPtr ExprFunc::clone(){
  return std::make_unique<ExprFunc>(params,body->clone());
}

ExprPtr ExprCall::clone(){
  ExprPtrList copy_args;
  ExprPtrList copy_extra;
  for(const auto &i:args){
    copy_args.push_back(i->clone());
  }
  for(const auto &i:extra){
    copy_extra.push_back(i->clone());
  }
  return std::make_unique<ExprCall>(callee->clone(),move(copy_args),
                                    move(copy_extra));
}

}
