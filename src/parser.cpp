#include "parser.h"

using namespace bindlang;
using std::make_unique;
using std::move;

void Parser::reset(){
  scanner.reset();
  token_ = Token(0,' ',std::string());
  future = law::fix_queue<Token>(5);
  error_num = 0;
}

bool Parser::error(const char *message){
  std::cerr<<RED("[Parsing error] ")<<message
           <<" with token:"<<token_<<std::endl;
  error_num++;
  return false;
}

bool Parser::hasError(){
  return error_num > 0;
}

bool Parser::fine(){
  return not hasError();
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

Token Parser::peekNext(){
  if(future.empty()){
    return borrow();
  }else{
    return future.front();
  }
}

ExprPtr Parser::parseExpr(){
  /* If we next token at start,make sure point to
     the last token in a whole expression when returned */
  if(hasError()) return nullptr;
  next(); 
  while(token_.type == tok_newline) next();
  if(token_.type == tok_eof)
    return make_unique<ExprEof>();
  if(token_.type == tok_err){
    error("error token in expression.");
    return nullptr;
  }
  switch(token_.type){
    case '\\':    return parseFunc();
    case tok_str:
    case tok_num: return parseAtom();
    case '<':     return parseTuple();
    case '[':     return parseList();
    case tok_id:{
      //next();// tok_id since it is cached
      Token tok = peekNext();
      switch(tok.type){
        case '=':           return parseDefine();
        case '(':           return parseCall();
        default:
          return parseId();
      }
    }
  }
  error("Wrong token in expression.");
  return  nullptr;
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
  verifyId();
  while(not atEnd()){
    params.push_back(token_);
    next();
    if(token_.type == tok_id){
      continue;
    }else if(token_.type == tok_arrow){
      body = parseExpr();
      auto func = make_unique<ExprFunc>(params,move(body)); 
      if(peekNext().type == '('){
        return parseCall(move(func));
      }
      return func;
    }else{
      error("Expect '->' in procedure define");
      return nullptr;
    }
  }
  error("Expect '->' procedure body in procedure define");
  return nullptr;
}

ExprPtr Parser::parseAtom(){
  switch(token_.type){
    case tok_str:
    case tok_num: return make_unique<ExprAtom>(token_);
    case '<':
      return parseTuple();
    case '[':
      return parseList();
  }
  error("Expect atoms.");
  return nullptr;
}

ExprPtr Parser::parseTuple(){
  //eg.   < 1 2 3 expr...>
  //      ^
  ExprPtrList container;
  Token tok;
  while(tok = peekNext(),tok.type != '>'){
    if(tok.type == tok_eof){
      error("Expect closing ')'");
      return nullptr;
    }
    container.push_back(parseExpr());
  }
  next();//go to closing '>'
  return make_unique<ExprTuple>(move(container));
}

ExprPtr Parser::parseList(){
  //eg.   [ 1 2 3 expr...]
  //      ^
  // ExprPtrBtree btree;
  while(token_.type != ']'){
    //parse();
  }
  return nullptr;
}

ExprPtr Parser::parseDefine(){
  Token id = token_;
  //eg. id = 3
  //     ^
  next();//^
  ExprPtr expr = parseExpr();
  return make_unique<ExprDefine>(id,move(expr));
}

ExprPtr Parser::parseCall(ExprPtr callee){
  if(not callee){
    callee = make_unique<ExprId>(token_);
  }
  //eg. someproc(x y | z=expr)
  //           ^
  next();//     ^
  ExprPtrList args;
  ExprPtrList extra;
  Token tok = peekNext();
  bool delim = false;
  if(tok.type == ')') goto FINAL;
  if(tok.type == '|') goto MIDDLE;
  while(true){
    if(delim){
      extra.push_back(parseExpr());
    }else{
      args.push_back(parseExpr());
    }
    tok = peekNext();
  MIDDLE:
    //some token can appear first
    if(tok.type == tok_id or
       tok.type == tok_num or
       tok.type == tok_str or
       tok.type == '<'){
      continue;
    }else if(tok.type == ')'){
      break;
    }else if(tok.type == '|'){
      if(delim){
        error("Duplicate '|' operator ");
        return nullptr;
      }else{
        delim = true;
        next();//eat '|'
      }
    }else{
      error("Expect sub expression or ')' in procedure call ");
      return nullptr;
    }
  }
 FINAL:
  next();//eat ')'
  auto call = make_unique<ExprCall>(move(callee),
                                    move(args),move(extra));
  if(peekNext().type == '('){
    return parseCall(move(call));
  }
  return call; 
}

ExprPtr Parser::parseId(){
  verifyId(token_);
  return make_unique<ExprId>(token_);
}
