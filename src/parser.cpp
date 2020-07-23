#include "parser.h"

using namespace bindlang;
using std::make_unique;
using std::move;

void Parser::reset(){
  scanner.reset();
  token_ = Token(0,' ',std::string());
  future = law::fix_queue<Token>(5);
  cache = std::stack<Token>();
}

void Parser::error(const char *message){
  std::cerr<<RED("[Parsing error] ")<<message
           <<" with token:"<<token_<<std::endl;
}

bool Parser::atEnd(){
  return future.empty() and token_.type == tok_eof;
}

void Parser::next(){
  if(future.empty()){
    token_ = scanner.nextToken();
  }else{
    token_ = future.pop();
  }
}

Token Parser::borrow(){
  Token tok = scanner.nextToken();
  future.push(tok);
  return tok;
}

ExprPtr Parser::parseExpr(){
  /* If we next token at start,make sure point to
     the last token in a whole expression when returned */
  next(); 
  if(token_.type == tok_eof)
    return make_unique<ExprEof>();
  if(token_.type == tok_err){
    error("error token in expression.");
    exit(1);
  }
  while(token_.type == tok_newline) next();
  switch(token_.type){
    case '\\':    return parseFunc();
    case tok_str:
    case tok_num: return parseAtom();
    case tok_id:{
      //next();// tok_id since it is cached
      Token tok = borrow();
      switch(tok.type){
        case '=':           return parseDefine();
        case '(':           return parseCall();
        default:
          return parseId();
      }
    }
  }
  error("Wrong token in expression.");
  exit(1);
}

ExprPtr Parser::parseNext(){
  return parseExpr();
}

bool Parser::verifyId(Token const& tok){
  if(tok.type != tok_id){
    error("Expect identification.");
    return false;
  } 
  return true;
}

bool Parser::verifyId(){
  return verifyId(token_);
}

ExprPtr Parser::parseFunc(){
  //eg. \x,y -> someproc(x,y)
  //    ^
  TokenList params;
  ExprPtr body;
  next();
  if(token_.type == tok_arrow){
    body = parseExpr();
    return make_unique<ExprFunc>(params,move(body));
  }
  while(not atEnd()){
    verifyId(); params.push_back(token_);
    next();
    if(token_.type == ','){
      next();
    }else if(token_.type == tok_arrow){
      body = parseExpr();
      return make_unique<ExprFunc>(params,move(body));
    }else{
      error("Expect ',' or '->' in procedure define");
      exit(1);
    }
  }
  error("Expect '->' procedure body in procedure define");
  exit(1);
}

ExprPtr Parser::parseAtom(){
  return make_unique<ExprAtom>(token_);
}

ExprPtr Parser::parseDefine(){
  Token id = token_;
  //eg. id = 3
  //     ^
  next();//^
  ExprPtr expr = parseExpr();
  return make_unique<ExprDefine>(id,move(expr));
}

ExprPtr Parser::parseCall(){
  Token id = token_;
  //eg. someproc(x,y)
  //           ^
  next();//     ^
  ExprPtrList args;
  Token tok = borrow();
  if(tok.type == ')') goto FINAL;
  while(true){
    args.push_back(parseExpr());
    next();
    if(token_.type == ','){
      continue;
    }if(token_.type == ')'){
      break;
    }else{
      error("Expect ',' or ')' in call args.");
    }
  }
 FINAL:
  next();//go to ')'
  return make_unique<ExprCall>(id,move(args)); 
}

ExprPtr Parser::parseId(){
  verifyId(token_);
  return make_unique<ExprId>(token_);
}
