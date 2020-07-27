#include <cstring>
#include "interpreter.h"
#include "ast.h"

namespace bindlang {

#define error(msg) \
  do{std::cerr<<RED("[Runtime error] ")<<msg<<endl; \
     error_num++;}while(0)

void Interpreter::reset(){
  error_num = 0;
}

template <typename Func,typename Object,typename ...Args>
void addPrimitive(EnvPtr env,string name,int arity,
                  Func func,Object obj,Args... other){
  using namespace std::placeholders;
  auto f = std::bind(func,obj,_1,other...);
  auto val = make_obj(name,arity,f);
  env->set(name,std::make_shared<Value>(val));
}

template <typename Func,typename Object>
void addBinary(EnvPtr env,const char *name,Object obj,
               Func func,
               function<bool(double,double)> op){
  addPrimitive(env,name,2,func,obj,op);
}

void Interpreter::standardEnvironment(){
  toplevel = make_shared<Environment>();
#define PRIM(name,arity,func) \
  addPrimitive(toplevel,name,arity,&Interpreter::func,this)
#define BINARY(name,func) \
  addBinary(toplevel,name,this,&Interpreter::binary,func)
  using namespace std::placeholders;
  auto calc = [this](const char *name,unsigned long arity,
                     double init,
                      function<double(double,double)> op){
                 addPrimitive(toplevel, name, arity,
                              &Interpreter::calculator,
                              this,init,op);
               };
#define inifinity -1
  envs.push(toplevel);
  PRIM("print",inifinity,Print);
  PRIM("eq",2,Eq);
  PRIM("if",2,If);
  PRIM("begin",inifinity,Begin);
  PRIM("env",0,InspectEnv);
  PRIM("debug",0,Debug);
  calc("+",inifinity,0,[](double x,double y){return x+y;});
  calc("-",inifinity,0,[](double x,double y){return x-y;});
  calc("*",inifinity,1,[](double x,double y){return x*y;});
  calc("/",inifinity,1,[](double x,double y){return x/y;});
  BINARY("gt",[](double x,double y){return x>y;});
  BINARY("lt",[](double x,double y){return x<y;});
  BINARY("ge",[](double x,double y){return x>=y;});
  BINARY("le",[](double x,double y){return x<=y;});
#undef inifinity
#undef BINARY
#undef PRIM
}

void Interpreter::debugger(ExprPtr expr){
  auto prompt = "[debugger]>";
  cout<<prompt;
  char buf[20];
  while(cin.getline(buf,sizeof(buf))){
    switch(buf[0]){
      case 's':
      case  0:
        return;
      case 'a':
        expr->show();
        break;
      case 'e':
        envs.top()->show();
        break;
      case 'q':
        interrupt = false;
        return;
    }
    memset(buf,0,sizeof(buf));
    cout<<endl<<prompt;
  }
}

ValPtr Interpreter::eval(ExprPtr expr){
  if(error_num>0)  return nullptr;
  if(interrupt){
    auto copy = expr->clone();
    debugger(move(copy)); 
  }
  switch(expr->type){
    case ATOM:     return evalAtom(move(expr));
    case ID:       return evalId(move(expr));
    case DEFINE:   return evalDefine(move(expr));
    case FUNC:     return evalFunc(expr->clone());
    case CALL:     return evalCall(move(expr));
    case EOF:      error_num++;return nullptr;
    default:
      error("Unreachable");
      return nullptr;
  }
}

#define cast(T,e) \
  (dynamic_cast<T>((e).release()))

ValPtr Interpreter::evalAtom(ExprPtr expr){
  auto atom = cast(ExprAtom*,expr);
  auto tok = atom->literal;
  switch(tok.type){
    case tok_num:
      return make_shared<Value>(stod(tok.literal));
    case tok_str:
      return make_shared<Value>(make_obj(tok.literal));
  }
  return make_shared<Value>();
}

ValPtr Interpreter::evalId(ExprPtr expr){
  auto id = cast(ExprId*,expr);
  auto tok = id->id;
  auto v = envs.top()->get(tok.literal);
  if(v == nullptr){
    error(tok.literal<<" not defined.");
  }
  return v;
}

ValPtr Interpreter::evalDefine(ExprPtr expr){
  auto def = cast(ExprDefine*,expr);
  auto tok = def->id;
  auto val = eval(move(def->expr));
  auto v = envs.top()->get(tok.literal);
  if(v){
    //change value exists in outer environment
    *v = *val;
  }else{
    //define at top environment
    envs.top()->set(tok.literal,val);
  }
  return val;
}

ValPtr Interpreter::evalFunc(ExprPtr expr){
  auto func = cast(ExprFunc*,expr);
  auto params = func->params;
  auto body = move(func->body);
  return make_shared<Value>(make_obj(envs.top(),params,body));
}

ValPtr Interpreter::evalCall(ExprPtr expr){
#define call_error(s)\
  do{error(" procedure:"<<s);\
    error_num++;return nullptr;}while(0)

  auto call   = cast(ExprCall*,expr);
  auto callee = eval(move(call->callee));
  auto args   = move(call->args);
  auto extra  = move(call->extra);
  if(callee and callee->type == VAL_OBJ){
    auto obj = callee->as.obj;
    if(obj->type == objType::String){
      call_error(" can not be called");
    }else if(obj->type == objType::Primitive){
      auto prim = dynamic_cast<ObjPrimitive*>(callee->as.obj);
      if(args.size() < prim->arity){
        call_error(" Expect at least "<<prim->arity<<" args.");
      }
      return prim->func(move(args));
    }
    auto proc = dynamic_cast<ObjProcedure*>(callee->as.obj);
    auto body = (proc->body)->clone();
    auto new_env = make_shared<Environment>(proc->closure);
    auto arity = proc->params.size();
    auto left_arity = arity - args.size();
    if(args.size() > arity){
      call_error(" Expect at most"<<arity<<" args.");
    }
    for(unsigned long i=0;i<args.size();i++){
      new_env->set((proc->params)[i].literal,
                   eval(move(args[i])));
    }
    // curry
    if(left_arity>0){
      auto left_params = TokenList();
      for(unsigned long i=args.size();i<arity;i++){
        left_params.push_back(proc->params[i]);
      }
      return make_shared<Value>(make_obj(new_env,
                                         left_params,body));
    }
    // for extra arg expr
    envs.push(new_env);
    for(auto & extra_expr:extra){
      eval(move(extra_expr));
    }

    //eval complete function body
    auto v = eval(move(body));
    envs.pop();
    return v;
  }else{
    call_error(" can not be called.");
  }
  return nullptr;
#undef call_error
}

ValPtr Interpreter::Print(ExprPtrList args){
  for(const auto &arg : args){
    auto v = eval(arg->clone());
    printVal(v);
  }
  std::cout<<std::endl;
  return std::make_shared<Value>();
}

ValPtr Interpreter::Eq(ExprPtrList args){
#define cast2(T,o) (dynamic_cast<T>((o)))
  auto l = eval(args[0]->clone());
  auto r = eval(args[1]->clone());
  bool boolean = false;
  if(l->type == r->type){
    switch(l->type){
      case VAL_NIL:
        boolean = true;
        break;
      case VAL_NUMBER:
        boolean = l->as.number == r->as.number;
        break;
      case VAL_OBJ:{
        auto objl = l->as.obj;
        switch(objl->type){
          case objType::String:{
            auto osl = cast2(ObjString*,objl);
            auto osr = cast2(ObjString*,r->as.obj);
            boolean = osl->s==osr->s;
            break;
          }
          case objType::Procedure:
            boolean = objl == r->as.obj;
            break;
          case objType::Primitive:
            boolean = objl == r->as.obj;
            break;
        }
      }
    }
  }
  return std::make_shared<Value>(boolean);
}


#define Expect(v,t) \
  do{if((v)!=(t)){error("Expect "#t);}}while(0)

ValPtr Interpreter::calculator(ExprPtrList args,
                               unsigned long init,
                               function<double(double,double)> op){
  double ans = init;
  bool   first = true;
  for(const auto & arg:args){                  
    auto v = eval(arg->clone());               
    Expect(v->type,VAL_NUMBER);
    if(first){
      ans = v->as.number;
      first = false;
    }else{
      ans = op(ans,v->as.number);
    }
  }
  return make_shared<Value>(ans);              
}

ValPtr Interpreter::binary(ExprPtrList args,
                           function<bool(double,double)> op){
  
  auto l = eval(args[0]->clone());
  auto r = eval(args[1]->clone());
  Expect(l->type,VAL_NUMBER);
  Expect(r->type,VAL_NUMBER);
  return make_shared<Value>(op(l->as.number,r->as.number));
}

bool truthy(ValPtr val){
  if(val->type == VAL_NIL) return false;
  if(val->type == VAL_BOOL and val->as.boolean == false)
    return false;
  return true;
}

ValPtr Interpreter::If(ExprPtrList args){
  auto c = eval(args[0]->clone());
  if(truthy(c)){
    return eval(args[1]->clone());
  }else{
    if(args.size()>2){
      return eval(args[2]->clone());
    }else{
      return make_shared<Value>(false);
    }
  }
}

ValPtr Interpreter::Begin(ExprPtrList args){
  ValPtr ans;
  auto new_env = make_shared<Environment>(envs.top());
  envs.push(new_env);
  for(const auto &arg :args){
    ans = eval(arg->clone());
  }
  envs.pop();
  return ans;
}

ValPtr Interpreter::InspectEnv(ExprPtrList args){
  envs.top()->show();
  return nullptr;
}

ValPtr Interpreter::Debug(ExprPtrList args){
  interrupt = true;
  cout<<"[+] set break point"<<endl;
  return nullptr;
}

#undef Expect
#undef cast
#undef cast2
#undef error
}
