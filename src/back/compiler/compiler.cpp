#include <cstdint>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include "back/compiler/compiler.h"
#include "define/type.h"
#include "util/utils.h"
namespace bindlang{
#define backup(V)  auto V##_backup = V 
#define recover(V) do{V = V##_backup;}while(0)

template <typename... Arg>

void Compiler::error(Arg... args){
  cerr<<RED("Compile Error:");
  (cerr<< ... <<args)<<endl;
  error_num++;
}

bool Compiler::hasError(){return error_num>0;}

uint16_t Compiler::Local::set(string const& name){
  map[name] = counter;
  return counter++;
}

uint16_t Compiler::Local::get(string const& name){
  return map[name];
}

uint32_t Compiler::Local::tellp(){
  return counter;
}

void Compiler::Local::sweep(uint32_t delim){
  auto dits = vector<decltype(map.begin())>();
  for(auto it=map.begin();it!=map.end();++it){
    if(it->second >= delim){
      dits.push_back(it);
    }
  }
  for(auto & it :dits){
    map.erase(it);
  }
  counter = delim;//recover counter before virtual scope
}

bool Compiler::Local::has(string const& name){
  auto it = map.find(name);
  return it!=map.end();
}

uint8_t Compiler::Closure::set(string const& name,
                           bool iflocal,uint8_t idx){
  auto count = values.size();
  map[name] = count;
  values.push_back(vm::Coder::CapturedValue{idx,iflocal,1});
  return count;
}

uint8_t Compiler::Closure::get(string const&name){
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

#define mkval(EXP) (vm::Value(vm::make_obj(EXP)))
void Compiler::compileFile2mem(string const& filename){
  string fn;
  bool top_modulep = false;
  if(files.empty()){
    fn = string(filename);
    pushVar(curScope(), "_MAIN",[this,&fn](){coder.CNST(mkval(fn));});
    top_modulep = true;
  }else{
    auto s = files.back();
    fn = dirname(s)+filename;
    for(auto const& already:files){
      if(already==fn) return;
    }
  }
  if(access(fn.c_str(), F_OK)!=0){
    cerr<<"Can not open file:"<<fn<<endl;
    return;
  }
  files.push_back(fn);
  pushVar(curScope(), "_FILE",
         [this,&fn](){coder.CNST(mkval(fn));});
  auto ifs = ifstream(fn);
  auto scn = Scanner(ifs);
  auto parser = Parser(scn);
  while(parser.fine() and not hasError()){
    auto expr = parser.parseNext();
    if(not expr or expr->type<0)
      break;
    rootp = true;
    define_funcp = false;
    compile(move(expr));
  }
  if(hasError()){
    cerr<<"compile failed :("<<endl;
  }else if(top_modulep){
    coder.HALT();
  }
}

void Compiler::writeBinary(string const& fn){
  coder.writeBinary(fn.c_str());
}

void Compiler::compileFile(string const& fn){
  compileFile2mem(fn);
  if(not hasError()){
    writeBinary(dirname(fn)+prefix(basename(fn))+".bdc");
  }
}

inline void nop(){}
void Compiler::compile(ExprPtr expr){
  if(hasError()) return;
  switch(expr->type){
    // pure
    case ATOM:     return rootp?nop():compileAtom(move(expr));
    case LIST:     return rootp?nop():compileList(move(expr));
    case ID:       return rootp?nop():compileId(move(expr));
    case FUNC:     return rootp?nop():compileFunc(expr->clone());
    // site effect
    case DEFINE:   return compileDefine(move(expr));
    case CALL:     return compileCall(move(expr));
    // helper
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
    compile(move(*it));
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
        uint8_t vidx;
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
  if(def->expr->type==FUNC){
    //for recursive self reference call
    curScope().set(name);
    resolve(move(def->expr));
  }else{
    resolve(move(def->expr));
    curScope().set(name);
  }
}

void Compiler::compileDefine(ExprPtr expr){
  if(rootp){rootp = false;}
  auto def = rcast(ExprDefine*,expr);
  auto name = def->id.literal;
  auto it = keywords.find(name);
  if(it!=keywords.end()){
    error("can not define ",name," this name is keyword");
  }
  if(def->expr->type == FUNC){
    define_funcp = true;
    curScope().set(name);
    last_name = name;
    compile(move(def->expr));
  }else{
    define_funcp = false;
    compile(move(def->expr));
    curScope().set(name);
  }
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
  string name = "anony";
  if(define_funcp) name = last_name;
  resolve(func->body->clone());
  locals = locals_backup;//restore locals
  emitFunc(name,(uint8_t)params.size(),0,
           [this,func](){
             compile(move(func->body));
           });
  coder.CAPTURE(closures.back().values);
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
  backup(rootp);
  rootp = false;
  auto call = rcast(ExprCall*,expr);
  auto args = move(call->args);
  if (call->callee->type == ID){
    // check if keyword
    auto name = rcast(ExprId*,call->callee->clone())->id.literal;
    auto it = keywords.find(name);
    if(it != keywords.end()){
      it->second(move(args));
      recover(rootp);
      if(rootp){
        coder.POP();
      }
      return;
    }
  }
  for(auto & arg:args){
    compile(move(arg));
  }
  compile(move(call->callee));
  coder.CALL();
  recover(rootp);
  if(rootp){
    coder.POP();
  }
}

#define PLACE_HOLDER 0
uint32_t Compiler::emitFunc(string const& name,
                            uint8_t arity,uint8_t property,
                            std::function<void(void)> f,bool copyp){
  auto pos_jmp = coder.tellp();
  coder.JMP(PLACE_HOLDER);
  auto pos_func = coder.tellp();
  f();
  coder.RET();//normal function always return...
  coder.modify16(pos_jmp+1,coder.tellp()-pos_jmp);//patch
  coder.CNST(vm::Value(vm::make_obj(name,arity,pos_func,property)));
  if(copyp){
    coder.COPY();
  }
  return coder.tellp();
}

void Compiler::pushFunc(Local & scope,
                            string const& name,
                        uint8_t arity,uint8_t property,
                            std::function<void(void)> f){
  emitFunc(name,arity,property,f,false);
  scope.set(name);
  return;
}

inline void Compiler::pushTopFunc(string const& name,
                                  uint8_t arity,uint8_t property,
                           std::function<void(void)> f){
  pushFunc(toplevel,name,arity,property,f);
}

void Compiler::pushVar(Local & scope,string const& name,
                       std::function<void(void)> f){
  f();
  scope.set(name);
}

void Compiler::standardEnvironment(){
  // push predefine function
  // print
  pushTopFunc("gc",0,1,[this](){coder.SYSCALL(2);});
  pushTopFunc("+",2,0,[this](){coder.ADD();});
  pushTopFunc("-",2,0,[this](){coder.MINUS();});
  pushTopFunc("*",2,0,[this](){coder.MULT();});
  pushTopFunc("/",2,0,[this](){coder.DIVIDE();});
  pushTopFunc("gt",2,0,[this](){coder.GT();});
  pushTopFunc("lt",2,0,[this](){coder.LT();});
  pushTopFunc("eq",2,0,[this](){coder.EQ();});
  pushTopFunc("cons",2,0,[this](){coder.CONS();});
  pushTopFunc("hd",1,0,[this](){coder.HEAD();});
  pushTopFunc("tl",1,0,[this](){coder.TAIL();});
  pushTopFunc("empty?",1,0,[this](){coder.EMPTYP();});
  pushTopFunc("not",1,0,[this](){coder.NOT();});
  pushTopFunc("len",1,0,[this](){coder.LEN();});

  keywords["print"] = [this](ExprPtrList args){
    if(args.size()<1){
      error("print expect at least one expression.");
    }else{
      compile(move(args[0]));
      coder.SYSCALL(0);
      auto it = args.begin();++it;
      for(;it!=args.end();++it){
        coder.POP();
        compile(move(*it));
        coder.SYSCALL(0);
      }
    }
  };
  keywords["if"] = [this](ExprPtrList args){
    if(args.size()!=2 and args.size()!=3)
      error("Wrong argument number ",args.size(),
            ", expect 2 or 3 args.");
    else{
      backup(rootp);
      rootp = false;
      compile(move(args[0]));
      auto test_end = coder.tellp();
      coder.JNE(PLACE_HOLDER);
      compile(move(args[1]));

      auto then_end = coder.tellp();
      coder.JMP(PLACE_HOLDER);
      auto else_start = coder.tellp();
      coder.modify16(test_end+1,else_start-test_end);
      if(args.size()>2){
        compile(move(args[2]));
      }else{
        coder.FALSE();
      }
      auto else_end = coder.tellp();
      coder.modify16(then_end+1,else_end-then_end);
      recover(rootp);
    }
  };
  keywords["let"] = [this](ExprPtrList args){
    int arity = args.size();
    if(arity<2)
      error("let expect at least twe arguments.");
    else if(args[arity-1]->type == DEFINE){
      error("define is not allowerd at the end of let.");
    }else{
      backup(rootp);
      uint32_t pos = curScope().tellp();
      int i=0;
      // def, cache all the names in let expression's def part
      for(;args[i]->type==DEFINE;i++){
        compile(move(args[i]));
      }
      // use
      coder.VCALL();
      rootp = true;
      for(;i<arity-1;i++){
        compile(move(args[i]));
      }
      rootp = false;
      compile(move(args[i]));
      coder.VRET();
      //clear all definitions in let
      curScope().sweep(pos);
      recover(rootp);
    }
  };
  keywords["begin"] = [this](ExprPtrList args){
    auto rootp_backup = rootp;
    auto it=args.begin();
    for(;it!=--args.end();++it){
      rootp = true;
      compile(move(*it));
    }
    rootp = false;
    compile(move(*it));
    rootp = rootp_backup;
  };

  keywords["and"] = [this](ExprPtrList args){
    if(args.size()<1)
      error("and expect one more argument");
    else{
      auto jmp_points = vector<size_t>();
      compile(move(args[0]));
      for(int i=1;i<args.size();i++){
        jmp_points.push_back(coder.tellp());
        coder.JNE(PLACE_HOLDER);
        compile(move(args[i]));
      }
      auto all_eval = coder.tellp();
      coder.JMP(PLACE_HOLDER);
      for(auto & point:jmp_points){
        coder.modify16(point+1,coder.tellp()-point);
      }
      coder.FALSE();
      coder.modify16(all_eval+1,coder.tellp()-all_eval);
    }
  };

  keywords["or"] = [this](ExprPtrList args){
    if(args.size()<1)
      error("and expect one more argument");
    else{
      auto jmp_points = vector<size_t>();
      compile(move(args[0]));
      for(int i=1;i<args.size();i++){
        jmp_points.push_back(coder.tellp());
        coder.JEQ(PLACE_HOLDER);
        compile(move(args[i]));
      }
      auto all_eval = coder.tellp();
      coder.JMP(PLACE_HOLDER);
      for(auto & point:jmp_points){
        coder.modify16(point+1,coder.tellp()-point);
      }
      coder.TRUE();
      coder.modify16(all_eval+1,coder.tellp()-all_eval);
    }
  };
  keywords["+"] = [this](ExprPtrList args){
    if(args.size()<2)
      error("plus expect twe more arguments.");
    else{
#define REDUCE(OP)                              \
      compile(move(args[0]));                   \
      for(int i=1;i<args.size();i++){           \
        compile(move(args[i]));                 \
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
      coder.HEAD();
    }
  };
  keywords["tl"]=[this](ExprPtrList args){
    if(args.size()!=1){
      error("hd expect one argument.");
    }else{
      compile(move(args[0]));
      coder.TAIL();
    }
  };
  keywords["load"]=[this](ExprPtrList args){
    for(auto & arg:args){
      if(arg->type != ATOM)
        error("load expect strings.");
      else{
        auto atom = rcast(ExprAtom*,arg);
        auto tok = atom->literal;
        switch(tok.type){
          case tok_str:{
            compileFile2mem(tok.literal);
            //load do not pop,so here push to against it
            coder.PUSH();
            break;
          }
          default:{
            error("load expect string literals.");
            break;
          }
        }
      }
    }
  };
  pushVar(toplevel,"null",[this](){coder.UNIT();});
  pushVar(toplevel,"#t",[this](){coder.CNST(vm::Value(true));});
  pushVar(toplevel,"#f",[this](){coder.CNST(vm::Value(false));});
  locals.push_back(toplevel);
  closures.push_back(Closure());
  coder.START();
}

#undef PLACE_HOLDER
}
