#include "parser.h"
#include <memory>

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

bool Parser::use_protect(){
  //one shot, quote can only proctect one expression
  bool now = protect;
  if(now){
    protect = not protect;
  }
  return now;
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
    case '\'':    protect = true;return parseExpr();
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
        case '.':           return parseDot();
        default:
          return make_unique<ExprId>(token_);
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

#define CHECKINT(n)                          \
  if((n) > 0x7fffffff){                      \
    error("Expect at most 2**31-1 items");   \
    return nullptr;                          \
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
    CHECKINT(params.size());
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
  ExprPtr atom;
  switch(token_.type){
    case tok_str:
    case tok_num:
       atom =  make_unique<ExprAtom>(token_);
       break;
    case '<':
      atom =  parseTuple();
      break;
    case '[':
      atom =  parseList();
      break;
    default:
      error("Expect atoms.");
      return nullptr;
  }
  if(peekNext().type == '.')
    return parseDot(move(atom)); 
  else
    return atom;
}

ExprPtr Parser::parseDefine(){
  Token id = token_;
  //eg. id = 3
  //     ^
  next();//^
  ExprPtr expr = parseExpr();
  return make_unique<ExprDefine>(id,move(expr),use_protect());
}

ExprPtr Parser::parseSet(ExprPtr beset){
  //eg. somefunc(somearg) = 5
  //                    ^
  next();               //^
  ExprPtr expr = parseExpr();
  return make_unique<ExprSet>(move(beset),move(expr),use_protect());
}

ExprPtr Parser::parseId(){
  //eg. mike.age
  //        ^
  next();//  ^
  verifyId(token_);
  return make_unique<ExprId>(token_);
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
  return make_unique<ExprTuple>(move(container),use_protect());
}

ExprPtr Parser::parseList(){
  //eg.   [ 1 2 3 expr...]
  //      ^
  ExprPtrList container;
  Token tok;
  while(tok = peekNext(),tok.type != ']'){
    if(tok.type == tok_eof){
      error("Expect closing ')'");
      return nullptr;
    }
    container.push_back(parseExpr());
  }
  next();//go to closing ']'
  return make_unique<ExprList>(move(container),use_protect());
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
      CHECKINT(extra.size());
      extra.push_back(parseExpr());
    }else{
      CHECKINT(args.size());
      args.push_back(parseExpr());
    }
    tok = peekNext();
  MIDDLE:
    if(tok.type == ')'){
      break;
    }else if(tok.type == '|'){
      if(delim){
        error("Duplicate '|' operator ");
        return nullptr;
      }else{
        delim = true;
        next();//eat '|'
      }
    }
  }
 FINAL:
  next();//eat ')'
  auto call = make_unique<ExprCall>(move(callee),
                                    move(args),move(extra));
  tok = peekNext();
  if(tok.type == '('){
    return parseCall(move(call));
  }else if(tok.type == '='){
    return parseSet(move(call));
  }else if(tok.type == '.'){
    return parseDot(move(call));
  }
  return call; 
}

ExprPtr Parser::parseDot(ExprPtr object){
  if(not object){
    object = make_unique<ExprId>(token_);
  }
  //eg. person.age.plus-one
  //         ^
  next();//   ^
  ExprPtrList args;
  ExprPtrList extra;
  args.push_back(move(object));
  // parseCall | parseId
  ExprPtr callee;
  auto tok = peekNext();
  if(tok.type == tok_id){
    next();
    tok = peekNext();
    if(tok.type=='(') callee = parseCall();
    else callee = make_unique<ExprId>(token_);
  }else{
    error("Expect identify");
    return nullptr;
  }
  auto call = make_unique<ExprCall>(move(callee),
                                    move(args),move(extra));
  tok = peekNext();
  if(tok.type == '.'){//left associate
    return parseDot(move(call));
  }else if(tok.type == '='){
    return parseSet(move(call));
  }else{
    return call;
  }
}

#undef CHECKINT
