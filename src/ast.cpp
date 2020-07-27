#include "ast.h"

namespace bindlang{

ExprPtr ExprAtom::clone(){
  return std::make_unique<ExprAtom>(literal);
}

ExprPtr ExprId::clone(){
  return std::make_unique<ExprId>(id);
}

ExprPtr ExprDefine::clone(){
  return std::make_unique<ExprDefine>(id,expr->clone());
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
