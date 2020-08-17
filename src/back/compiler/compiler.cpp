#include <iostream>
#include <fstream>
#include <unistd.h>
#include "back/compiler/compiler.h"
#include "util/utils.h"
namespace bindlang{

template <typename... Arg>
void Compiler::error(Arg... args){
  cerr<<RED("Compile Error:");
  (cerr<< ... <<args)<<endl;
  error_num++;
}

bool Compiler::hasError(){return error_num>0;}

int Compiler::Local::set(string const& name){
  map[name] = counter;
  return counter++;
}

int Compiler::Local::get(string const& name){
  auto it = map.find(name);
  if(it!=map.end()){
    return it->second;
  }
  return 0;
}

bool Compiler::Local::has(string const& name){
  auto it = map.find(name);
  return it!=map.end();
}

int Compiler::Closure::set(string const& name,
                           int distance,int idx){
  map[name] = values.size();
  values.push_back(CapturedVal{distance,idx});
}

bool Compiler::Closure::has(string const&name){
  auto it=map.find(name);
  return it!=map.end();
}

void Compiler::Closure::clear(){
  map.clear();
  values.clear();
}

void Compiler::runFile(string const& fn){
  if(access(fn.c_str(), F_OK)!=0){
    cerr<<"Can not open file:"<<fn<<endl;
    return;
  }
  auto ifs = ifstream(fn);
  auto scn = Scanner(ifs);
  auto parser = Parser(scn);
  while(parser.fine() and not hasError()){
    auto expr = parser.parseNext();
    if(not expr or expr->type<0)
      break;
    //expr->show();
    compile(move(expr));
  }
  if(not hasError()){
    coder.HALT();
    writeBinary(dirname(fn)+prefix(basename(fn))+".bdc");
  }else{
    cerr<<"compile failed"<<endl;
  }
}

void Compiler::writeBinary(string const& fn){
  coder.writeBinary(fn.c_str());
}

void Compiler::compile(ExprPtr expr){
  if(hasError()) return;
  switch(expr->type){
    case ATOM:     return compileAtom(move(expr));break;
    case ID:       return compileId(move(expr));break;
    case DEFINE:   return compileDefine(move(expr));break;
    case FUNC:     return compileFunc(expr->clone());break;
    case CALL:     return compileCall(move(expr));break;
    case EOF:      error_num++;coder.HALT();return;
    default:
      error("Unreachable");
      return;
  }
}

void Compiler::compileAtom(ExprPtr expr){
  auto atom = rcast(ExprAtom*,expr);
  auto tok = atom->literal;
  switch(tok.type){
    case tok_num:{
      auto n = atoi(tok.literal.c_str());
      coder.IMM(n);
      break;
    }
    case tok_str:
      coder.CNST(vm::Value(vm::make_obj(tok.literal)));
      break;
  }
}

void Compiler::resolveId(ExprPtr expr){
  auto name = rcast(ExprId*,expr)->id.literal;
  if(curScope().has(name)) return;
  else if(locals.size()>1){
    // check closure
    if(closure.has(name)) return;
    // check outer scope
    auto it = locals.rbegin();++it;
    for(int distance=0;it!=locals.rend();++it,++distance){
      if(it->has(name)){
        // capture it to closure
        auto idx = it->get(name);
        closure.set(name,distance,idx);
        return;
      }
    }
  }
  // or compile error
  cout<<"in resolve name is:"<<name<<endl;
  error("using undefined variable ",name);
}

void Compiler::compileId(ExprPtr expr){
  auto name = rcast(ExprId*,expr)->id.literal;
  if(curScope().has(name)){
    // if in the scope
    auto idx = locals.back().get(name);
    coder.GETL(idx);
    return;
  }else{
    // or if in the closure
    if( closure.has(name)){
      
    }
  }
  // or compile error
  cout<<"use name is:"<<name<<endl;
  error("using undefined variable ",name);
}

void Compiler::resolveDefine(ExprPtr expr){
  auto def = rcast(ExprDefine*,expr);
  auto name = def->id.literal;
  resolve(move(def->expr));
  curScope().set(name);
}

void Compiler::compileDefine(ExprPtr expr){
  auto def = rcast(ExprDefine*,expr);
  auto name = def->id.literal;
  compile(move(def->expr));
  coder.PUSH();
  curScope().set(name);
}

inline Compiler::Local& Compiler::curScope(){
  return locals.back();
}

void Compiler::beginScope(vector<string> const& args){
  auto scope = Local();
  for(auto &arg:args){
    scope.set(arg);
  }
  locals.push_back(scope);
}

void Compiler::beginScope(){
  locals.push_back(Local());
}

void Compiler::endScope(){
  locals.pop_back();
}

void Compiler::resolve(ExprPtr expr){
  switch(expr->type){
    case DEFINE:   return resolveDefine(move(expr));
    case ID:       return resolveId(move(expr));
    default: return;
  }
}

void Compiler::compileFunc(ExprPtr expr){
  auto func = rcast(ExprFunc*,expr);
  auto params = func->params;
  beginScope(mapvec<Token,string>(params,
                    [](Token& tok)->string{
                      return tok.literal;}));
  resolve(func->body->clone());
  emitFunc("anony",(uint32_t)params.size(),
           [this,func](){
             compile(move(func->body));
             coder.RET();
           });
  closure.clear();
  endScope();
}

void Compiler::compileCall(ExprPtr expr){
  auto call = rcast(ExprCall*,expr);
  compile(move(call->callee));
  coder.PUSH();
  auto idx = locals.back().set("temp");
  auto args = move(call->args);
  for(auto & arg:args){
    compile(move(arg));
    coder.PUSH();
  }
  coder.GETL(idx);
  coder.CALL();
}

uint32_t Compiler::emitFunc(string const& name,
                            int arity,
                            std::function<void(void)> f){
  auto pos_jmp = coder.tellp();
  coder.JMP(0);//place holder,jump out of function code
  auto pos_func = coder.tellp();
  f();
  coder.modify(pos_jmp+1,coder.tellp()-pos_jmp);//patch
  coder.CNST(vm::Value(vm::make_obj(name,arity,pos_func)));
  return coder.tellp();
}

void Compiler::pushFunc(Local & scope,
                            string const& name,
                            int arity,
                            std::function<void(void)> f){

  emitFunc(name,arity,f);
  coder.PUSH();
  scope.set(name);
  return;
}

void Compiler::standardEnvironment(){
  // push predefine function
  // print
  pushFunc(toplevel,"print",1,[this](){
    coder.GETL(0);
    coder.SYSCALL(0);
    coder.RET();
  });
  locals.push_back(toplevel);
}

}
