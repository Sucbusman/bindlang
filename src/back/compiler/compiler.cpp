#include <iostream>
#include <fstream>
#include <unistd.h>
#include "back/compiler/compiler.h"
#include "define/type.h"
#include "util/utils.h"
namespace bindlang{

template <typename... Arg>

void Compiler::error(Arg... args){
  cerr<<RED("Compile Error:");
  (cerr<< ... <<args)<<endl;
  error_num++;
}

bool Compiler::hasError(){return error_num>0;}

uint32_t Compiler::Local::set(string const& name){
  map[name] = counter;
  return counter++;
}

uint32_t Compiler::Local::get(string const& name){
  return map[name];
}

bool Compiler::Local::has(string const& name){
  auto it = map.find(name);
  return it!=map.end();
}

uint32_t Compiler::Closure::set(string const& name,
                           bool iflocal,uint32_t idx){
  auto count = values.size();
  map[name] = count;
  values.push_back(vm::Coder::CapturedValue{idx,iflocal,1});
  return count;
}

uint32_t Compiler::Closure::get(string const&name){
  return map[name];
}

bool Compiler::Closure::has(string const&name){
  auto it=map.find(name);
  return it!=map.end();
}

void Compiler::Closure::clear(){
  map.clear();
  values.clear();
}

void Compiler::compileFile2mem(string const& fn){
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
    compile(move(expr));
  }
  if(not hasError()){
    coder.HALT();
  }else{
    cerr<<"compile failed :("<<endl;
  }
}

void Compiler::compileFile(string const& fn){
  compileFile2mem(fn);
  if(not hasError()){
    writeBinary(dirname(fn)+prefix(basename(fn))+".bdc");
  }
}

void Compiler::writeBinary(string const& fn){
  coder.writeBinary(fn.c_str());
}

void Compiler::compile(ExprPtr expr){
  if(hasError()) return;
  switch(expr->type){
    case ATOM:     return compileAtom(move(expr));
    case LIST:     return compileList(move(expr));
    case ID:       return compileId(move(expr));
    case DEFINE:   return compileDefine(move(expr));
    case FUNC:     return compileFunc(expr->clone());
    case CALL:     return compileCall(move(expr));
    case EOF:      error_num++;coder.HALT();
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

void Compiler::resolveList(ExprPtr expr){
  auto lst = move(rcast(ExprList*,expr)->container);
  for(auto it=lst.rbegin();it!=lst.rend();it++){
    resolve(move(*it));
  }
}

void Compiler::compileList(ExprPtr expr){
  auto lst = move(rcast(ExprList*,expr)->container);
  coder.UNIT();
  for(auto it=lst.rbegin();it!=lst.rend();it++){
    coder.PUSH();
    compile(move(*it));
    coder.PUSH();
    coder.RCONS();
  }
}

void Compiler::resolveId(ExprPtr expr){
  auto name = rcast(ExprId*,expr)->id.literal;
  if(curScope().has(name)) return;
  else if(locals.size()>1){
    // check closure
    if(closures.back().has(name)) return;
    // check outer scope
    auto it = locals.rbegin();++it;
    for(int distance=1;it!=locals.rend();++it,++distance){
      if(it->has(name)){
        // capture it to closure
        auto lidx = it->get(name);
        auto j = closures.size()-distance;
      //cout<<"capture "<<name<<" at "<<distance<<' '<<lidx<<endl;
        uint32_t vidx;
        if(closures[j].has(name)){
          vidx = closures[j].get(name);
        }else{
          vidx = closures[j].set(name,true,lidx);
        }
        for(++j;j<closures.size();++j){
          if(closures[j].has(name)){
            vidx = closures[j].get(name);
          }else{
            vidx = closures[j].set(name,false,vidx);
          }
        }
        break;
      }
    }
  }else{
    // or compile error
    error("using undefined variable ",name);
  }
}

void Compiler::compileId(ExprPtr expr){
  auto name = rcast(ExprId*,expr)->id.literal;
  if(curScope().has(name)){
    // if in the scope
    auto idx = locals.back().get(name);
    coder.GETL(idx);
  }else if(closures.back().has(name)){
    // or if in the closure
    auto idx = closures.back().get(name);
    coder.GETC(idx);
  }else{
    // or compile error
    error("using undefined variable ",name);
  }
}

void Compiler::resolveDefine(ExprPtr expr){
  auto def = rcast(ExprDefine*,expr);
  auto name = def->id.literal;
  //if define function,the body should know function name
  //TODO:else compile error
  auto it = keywords.find(name);
  if(it!=keywords.end()){
    error("can not define ",name,",this name is a keyword");
  }
  curScope().set(name);//!here bug
  resolve(move(def->expr));
}

void Compiler::compileDefine(ExprPtr expr){
  auto def = rcast(ExprDefine*,expr);
  auto name = def->id.literal;
  auto it = keywords.find(name);
  if(it!=keywords.end()){
    error("can not define ",name," this name is keyword");
  }
  curScope().set(name);
  //cout<<name<<" scope index:"<<curScope().get(name)<<endl;
  compile(move(def->expr));
  coder.PUSH();
}

inline Compiler::Local& Compiler::curScope(){
  return locals.back();
}

void Compiler::beginScope(vector<string> const& args){
  auto scope = Local();
  for(auto &arg:args){
    scope.set(arg);
  }
  closures.push_back(Closure());
  locals.push_back(scope);
}

void Compiler::beginScope(){
  closures.push_back(Closure());
  locals.push_back(Local());
}

void Compiler::endScope(){
  locals.pop_back();
  closures.pop_back();
}

void Compiler::resolve(ExprPtr expr){
  switch(expr->type){
    case LIST:     return resolveList(move(expr));
    case DEFINE:   return resolveDefine(move(expr));
    case ID:       return resolveId(move(expr));
    case CALL:     return resolveCall(move(expr));
    default: return;
  }
}

void Compiler::compileFunc(ExprPtr expr){
  auto func = rcast(ExprFunc*,expr);
  auto params = func->params;
  beginScope(mapvec<Token,string>(params,
                    [](Token& tok)->string{
                      return tok.literal;}));
  auto locals_backup = locals;
  resolve(func->body->clone());
  locals = locals_backup;
  emitFunc("anony",(uint32_t)params.size(),
           [this,func](){
             compile(move(func->body));
           });
  coder.PUSH();//for capture itself in recursion function
  coder.CAPTURE(closures.back().values);
  coder.POP();
  endScope();
}

void Compiler::resolveCall(ExprPtr expr){
  auto call = rcast(ExprCall*,expr);
  for(auto & arg:call->args){
    resolve(move(arg));
  }
  if (call->callee->type == ID){
    // check if keyword
    auto name = rcast(ExprId*,call->callee->clone())->id.literal;
    auto it = keywords.find(name);
    if(it != keywords.end()){
      return;
    }
  }
  resolve(move(call->callee));
}

void Compiler::compileCall(ExprPtr expr){
  auto call = rcast(ExprCall*,expr);
  auto args = move(call->args);
  if (call->callee->type == ID){
    // check if keyword
    auto name = rcast(ExprId*,call->callee->clone())->id.literal;
    auto it = keywords.find(name);
    if(it != keywords.end()){
      return it->second(move(args));
    }
  }

  for(auto & arg:args){
    compile(move(arg));
    coder.PUSH();
  }
  compile(move(call->callee));
  coder.CALL();   
}

uint32_t Compiler::emitFunc(string const& name,
                            int arity,
                            std::function<void(void)> f){
  auto pos_jmp = coder.tellp();
  coder.JMP(0);//place holder,jump out of function code
  auto pos_func = coder.tellp();
  f();
  coder.RET();//normal function always return...
  coder.modify(pos_jmp+1,coder.tellp()-pos_jmp);//patch
  coder.CNST(vm::Value(vm::make_obj(name,arity,pos_func)));
  coder.COPY();
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

inline void Compiler::pushTopFunc(string const& name,
                           int arity,
                           std::function<void(void)> f){
  pushFunc(toplevel,name,arity,f);
}

void Compiler::pushVar(Local & scope,string const& name,
                       std::function<void(void)> f){
  f();
  coder.PUSH();
  scope.set(name);
}

void Compiler::standardEnvironment(){
  // push predefine function
  // print
  pushFunc(toplevel,"print",1,[this](){
    coder.GETL(0);
    coder.SYSCALL(0);
  });
  pushTopFunc("+",2,[this](){coder.ADD();});
  pushTopFunc("-",2,[this](){coder.MINUS();});
  pushTopFunc("*",2,[this](){coder.MULT();});
  pushTopFunc("/",2,[this](){coder.DIVIDE();});
  pushTopFunc("gt",2,[this](){coder.GT();});
  pushTopFunc("lt",2,[this](){coder.LT();});
  pushTopFunc("eq",2,[this](){coder.EQ();});
  pushTopFunc("cons",2,[this](){coder.CONS();});
  pushTopFunc("hd",1,[this](){coder.HEAD();});
  pushTopFunc("tl",1,[this](){coder.TAIL();});
  pushTopFunc("empty?",1,[this](){coder.EMPTYP();});
  keywords["if"] = [this](ExprPtrList args){
    if(args.size()!=2 and args.size()!=3)
      error("Wrong argument number ",args.size(),
            ", expect 2 or 3 args.");
    else{
      compile(move(args[0]));
      auto test_end = coder.tellp();
      coder.JNE(0);//place holder
      compile(move(args[1]));
      auto then_end = coder.tellp();
      coder.JMP(0);//place holder
      auto else_start = coder.tellp();
      coder.modify(test_end+1,else_start-test_end);
      if(args.size()>2){
        compile(move(args[2]));
      }else{
        coder.FALSE();
      }
      auto else_end = coder.tellp();
      coder.modify(then_end+1,else_end-then_end);
    }
  };

  keywords["begin"] = [this](ExprPtrList args){
    for(auto &arg:args){
      compile(move(arg));
    }
  };

  keywords["while"] = [this](ExprPtrList args){
    if(args.size()<1)
      error("while expect more than one argument.");
    else{
      auto test_start = coder.tellp();
      compile(move(args[0]));
      auto body_start = coder.tellp();
      coder.JNE(0);
      auto it=args.begin();
      for(++it;it!=args.end();++it){
        compile(move(*it));
      }
      coder.JMP(test_start-coder.tellp());
      auto body_end = coder.tellp();
      coder.modify(body_start+1,body_end-body_start);
    }
  };

  keywords["+"] = [this](ExprPtrList args){
    if(args.size()<2)
      error("plus expect twe more arguments.");
    else{
#define REDUCE(OP)                              \
      compile(move(args[0]));                   \
      for(int i=1;i<args.size();i++){           \
        coder.PUSH();                           \
        compile(move(args[i]));                 \
        coder.PUSH();                           \
        coder.OP();                             \
      }
      REDUCE(ADD);
    }
  };
  keywords["-"] = [this](ExprPtrList args){
    if(args.size()<2)
      error("minus expect twe more arguments.");
    else{
      REDUCE(MINUS);
    }
  };
  keywords["*"] = [this](ExprPtrList args){
    if(args.size()<2)
      error("multiply expect twe more arguments.");
    else{
      REDUCE(MULT);
    }
  };
  keywords["/"] = [this](ExprPtrList args){
    if(args.size()<2)
      error("divide expect twe more arguments.");
    else{
      REDUCE(DIVIDE);
    }
  };
  keywords["hd"]=[this](ExprPtrList args){
    if(args.size()!=1){
      error("hd expect one argument.");
    }else{
      compile(move(args[0]));
      coder.PUSH();
      coder.HEAD();
    }
  };
  keywords["tl"]=[this](ExprPtrList args){
    if(args.size()!=1){
      error("hd expect one argument.");
    }else{
      compile(move(args[0]));
      coder.PUSH();
      coder.TAIL();
    }
  };
  pushVar(toplevel,"null",[this](){coder.UNIT();});
  pushVar(toplevel,"#t",[this](){coder.CNST(vm::Value(true));});
  pushVar(toplevel,"#f",[this](){coder.CNST(vm::Value(false));});
  locals.push_back(toplevel);
  closures.push_back(Closure());
}

}
