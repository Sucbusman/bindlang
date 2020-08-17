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
  if(it!=map.end()){
    return true;
  }else{
    return false;
  }
}

void Compiler::runFile(string const& fn){
  if(access(fn.c_str(), F_OK)!=0){
    cerr<<"Can not open file:"<<fn<<endl;
    return;
  }
  auto ifs = ifstream(fn);
  auto scn = Scanner(ifs);
  auto parser = Parser(scn);
  while(parser.fine()){
    auto expr = parser.parseNext();
    if(not expr or expr->type<0)
      break;
    expr->show();
    compile(move(expr));
  }
  coder.HALT();
  writeBinary(dirname(fn)+prefix(basename(fn))+".bdc");
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

void Compiler::compileId(ExprPtr expr){
  auto name = rcast(ExprId*,expr)->id.literal;
  if(locals.back().has(name)){
    auto idx = locals.back().get(name);
    coder.GETL(idx);
  }else{
    cout<<"use name is:"<<name<<endl;
    error("using undefined variable ",name," at this scope.");
  }
}

void Compiler::compileDefine(ExprPtr expr){
  auto def = rcast(ExprDefine*,expr);
  auto name = def->id.literal;
  cout<<"define name :"<<name<<endl;
  compile(move(def->expr));
  coder.PUSH();
  locals.back().set(name);
}

void Compiler::compileFunc(ExprPtr expr){
  auto func = rcast(ExprFunc*,expr);
  auto params = func->params;
  emitFunc("anony",(uint32_t)params.size(),
           [this,func](){
             compile(move(func->body));
             coder.RET();
           });
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
  coder.insert(pos_jmp+1,coder.tellp()-pos_jmp);//patch
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
