#include <cstring>
#include <memory>
#include "interpreter.h"
#include "ast.h"
#include "type.h"
#include "value.h"

namespace bindlang {

#define error(msg) \
  do{std::cerr<<RED("[Runtime error] ")<<msg<<endl; \
     error_num++;}while(0)

#define Expect(v,t) \
  do{if((v)!=(t)){error("Expect "#t);return nullptr;}}while(0)

#define check_error() \
  if(error_num>0){ return nullptr;} 

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
void addCompare(EnvPtr env,const char *name,Object obj,
               Func func,
               function<bool(double,double)> op){
  addPrimitive(env,name,2,func,obj,op);
}

void Interpreter::standardEnvironment(){
#define PRIM(name,arity,func) \
  addPrimitive(toplevel,name,(arity),&Interpreter::func,this)
#define PRIM2(name,arity,func,...) \
  addPrimitive(toplevel,name,(arity),&Interpreter::func,this,__VA_ARGS__)
#define VALUE(name,v) \
  toplevel->set(name,std::make_shared<Value>((v)))
#define COMPARE(name,func) \
  addCompare(toplevel,name,this,&Interpreter::compare,func)
  using namespace std::placeholders;
  auto calc = [this](const char *name,int arity,
                     double init,
                      function<double(double,double)> op){
                 addPrimitive(toplevel, name, arity,
                              &Interpreter::calculator,
                              this,init,op);
               };
#define inifinity -1

  toplevel = make_shared<Environment>();
  envs.push(toplevel);
  VALUE("false",false);
  VALUE("true",true);

  PRIM("+",2,Plus);
  calc("-",2,0,[](double x,double y){return x-y;});
  calc("*",2,1,[](double x,double y){return x*y;});
  calc("/",2,1,[](double x,double y){return x/y;});

  PRIM("eq",2,Eq);
  COMPARE("gt",[](double x,double y){return x>y;});
  COMPARE("lt",[](double x,double y){return x<y;});
  COMPARE("ge",[](double x,double y){return x>=y;});
  COMPARE("le",[](double x,double y){return x<=y;});
  PRIM("not",1,Not);
  PRIM("and",1,And);
  PRIM("or", 1,Or);

  PRIM("begin",1,Begin);
  PRIM("if",2,If);
  PRIM("while",2,While);
  PRIM2("=>",1,Pipe,true);
  PRIM("'",0,Quote);

  PRIM("empty?",1,isEmpty);

  PRIM("take",2,Take);
  PRIM("len",1,Length);
  PRIM("set!",2,Set);
  PRIM("push!",2,Push);
  PRIM("pop!",1,Pop);

  PRIM("cons",2,Cons);
  PRIM("hd",1,Head);
  PRIM("tl",1,Tail);

  PRIM("print",1,Print);

  PRIM("env",0,InspectEnv);
  PRIM("debug",0,Debug);
  PRIM("garbage",0,Garbage);
  PRIM("gc",0,Gc);
#undef VALUE
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
  check_error();
  if(interrupt){
    auto copy = expr->clone();
    debugger(move(copy)); 
  }
  switch(expr->type){
    case ATOM:     return evalAtom(move(expr));break;
    case ID:       return evalId(move(expr));break;
    case DEFINE:   return evalDefine(move(expr));break;
    case FUNC:     return evalFunc(expr->clone());break;
    case CALL:     return evalCall(move(expr));break;
    case TUPLE:    return evalTuple(move(expr));break;
    case LIST:     return evalList(move(expr));break;
    case EOF:      error_num++;return nullptr;
    default:
      error("Unreachable");
      return nullptr;
  }
  return nullptr;
}

ValPtr Interpreter::evalAtom(ExprPtr expr){
  auto atom = rcast(ExprAtom*,expr);
  auto tok = atom->literal;
  switch(tok.type){
    case tok_num:
      return make_shared<Value>(stod(tok.literal));
    case tok_str:
      return make_shared<Value>(make_obj(tok.literal));
  }
  return make_shared<Value>();
}

inline ValPtr expr2val(ExprPtr expr){
  auto e = expr.release();
  return make_shared<Value>(make_obj(e));
}

ValPtr Interpreter::evalTuple(ExprPtr expr){
  auto tup = rcast(ExprTuple*,expr);
  auto c = ValPtrList();
  if(tup->protect){
    for(auto & e:tup->container){
      c.push_back(expr2val(move(e)));
    }
  }else{
    for(auto & e:tup->container){
      c.push_back(eval(move(e)));
    }
  }
  return make_shared<Value>(make_obj(c));
}

ValPtr Interpreter::evalList(ExprPtr expr){
  auto e = rcast(ExprList*,expr);
  auto lst = move(e->container);
  ObjList* last = make_obj(nullptr,nullptr);
  if(e->protect){
    for(auto it=lst.rbegin();it!=lst.rend();it++){
      last = make_obj(expr2val(move(*it)),last);
    }
  }else{
    for(auto it=lst.rbegin();it!=lst.rend();it++){
      last = make_obj(eval(move(*it)),last);
    }
  }
  return make_shared<Value>(last);
}

ValPtr Interpreter::evalId(ExprPtr expr){
  auto id = rcast(ExprId*,expr);
  auto tok = id->id;
  auto v = envs.top()->get(tok.literal);
  if(v == nullptr){
    error(tok.literal<<" not defined.");
    return nullptr;
  }
  return *v;
}

ValPtr Interpreter::evalDefine(ExprPtr expr){
  auto def = rcast(ExprDefine*,expr);
  auto tok = def->id;
  auto val = eval(move(def->expr));
  check_error();
  auto pv = envs.top()->get(tok.literal);
  if(pv){
    if((*pv)->immutable){
      *pv = val;
      return val;
    }else{
      auto copy = make_shared<Value>(*val);
      //change value exists in outer environment
      *pv = copy;
      return copy;
    }
  }else{
    auto copy = make_shared<Value>(*val);
    //define at nearest environment
    envs.top()->set(tok.literal,copy);
    return copy;
  }
}

ValPtr Interpreter::evalFunc(ExprPtr expr){
  auto func = rcast(ExprFunc*,expr);
  auto params = func->params;
  auto body = move(func->body);
  return make_shared<Value>(make_obj(envs.top(),params,body));
}

ValPtr Interpreter::evalCall(ExprPtr expr){
#define call_error(s)\
  do{error(" procedure:"<<s);\
    error_num++;return nullptr;}while(0)
  auto call   = rcast(ExprCall*,expr);
  auto callee = eval(move(call->callee));
  check_error();
  auto args   = move(call->args);
  auto extra  = move(call->extra);
  if(callee){
    if(callee->type == VAL_Procedure){
      auto prim = cast(ObjProcedure*, callee->as.obj);
      return evalCallProc(prim, args, extra);
    }else if(callee->type == VAL_Primitive){
      auto prim = cast(ObjPrimitive*, callee->as.obj);
      return evalCallPrim(prim, args, extra);
    }
  }
  call_error(" can not be called.");
  return nullptr;
}

ValPtr Interpreter::evalCallPrim(ObjPrimitive* prim,ExprPtrList& args,ExprPtrList& extra){
  if((int)args.size() < prim->arity){
    call_error(" Primitive expect at least "
               <<prim->arity<<" args.");
  }
  auto new_env = make_shared<Environment>(envs.top());
  envs.push(new_env);
  // for extra arg expr
  for(auto & extra_expr:extra){
    eval(move(extra_expr));
  }
  auto result = prim->func(move(args));

  envs.pop();
  return result;
}

ValPtr Interpreter::evalCallProc(ObjProcedure* proc,ExprPtrList& args,ExprPtrList& extra){
  auto body = (proc->body)->clone();
  auto new_env = make_shared<Environment>(proc->closure);
  int arity = (int)proc->params.size();
  auto left_arity = arity - args.size();
  if((int)args.size() > arity){
    call_error(" Expect at most "<<arity<<" args.");
  }
  for(int i=0;i<(int)args.size();i++){
    new_env->set((proc->params)[i].literal,
                 eval(move(args[i])));
  }
  check_error();
  // curry
  if(left_arity>0){
    auto left_params = TokenList();
    for(int i=(int)args.size();i<arity;i++){
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
  if(error_num>0){
    envs.pop();
    return nullptr;
  }
  //eval complete function body
  auto result = eval(move(body));
  envs.pop();
  return result;
#undef call_error
}


// calculate
ValPtr Interpreter::calculator(ExprPtrList args,
                               double init,
                               function<double(double,double)> op){
  double ans = init;
  bool   first = true;
  for(const auto & arg:args){
    auto v = eval(arg->clone());
    check_error();
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

template <typename T>
ValPtr Interpreter::calcu2(ExprPtrList args,function<T*(T&,T&)> op){
  T* ans = nullptr;
  bool first = true;
  for(const auto & arg:args){
    auto v = eval(arg->clone());
    check_error();
    //Expect(v->type,VAL_##T);
    T* o = cast(T*,v->as.obj);
    if(o){
      if(first){
        ans = o;
        first = false;
      }else{
        ans = op(*ans,*o);
      }  
    }else{
      error("Expect xxx");
      return nullptr;
    }
  }
  return make_shared<Value>(ans);              
}

ValPtr Interpreter::Plus(ExprPtrList args){
  auto firstval = eval(args[0]->clone());
  check_error();
  switch(firstval->type){
    case VAL_NUMBER:
      return calculator(move(args),0,[](double l,double r){return l+r;});
    case VAL_String:
      return calcu2<ObjString>(move(args),StringPlus);
    case VAL_Tuple:
      return calcu2<ObjTuple>(move(args),TuplePlus);
    default: goto E;
  }
 E:
  error("Plus expect number,string or list");
  return nullptr;
}

// comparison
ValPtr Interpreter::Eq(ExprPtrList args){
  auto l = eval(args[0]->clone());
  check_error();
  auto r = eval(args[1]->clone());
  check_error();
  bool boolean = false;
  if(l->type == r->type){
    switch(l->type){
      case VAL_NIL:
        boolean = true;
        break;
      case VAL_NUMBER:
        boolean = l->as.number == r->as.number;
        break;
      case VAL_String:{
        auto osl = cast(ObjString*,l->as.obj);
        auto osr = cast(ObjString*,r->as.obj);
        boolean = osl->s==osr->s;
        break;
      }
      case VAL_Procedure:
      case VAL_Primitive:
      case VAL_Tuple:
      case VAL_List:
      case VAL_Ast:
        boolean = l->as.obj == r->as.obj;
        break;
      }
    }
  return std::make_shared<Value>(boolean);
}

ValPtr Interpreter::compare(ExprPtrList args,
                           function<bool(double,double)> op){
  
  auto l = eval(args[0]->clone());
  check_error();
  auto r = eval(args[1]->clone());
  check_error();
  Expect(l->type,VAL_NUMBER);
  Expect(r->type,VAL_NUMBER);
  return make_shared<Value>(op(l->as.number,r->as.number));
}

// logic
bool truthy(ValPtr val){
  if(val == nullptr) return false;
  if(val->type == VAL_NIL) return false;
  if(val->type == VAL_BOOL and val->as.boolean == false)
    return false;
  return true;
}

ValPtr Interpreter::Not(ExprPtrList args){
  auto ans = eval(move(args[0]));
  check_error();
  if(truthy(ans)) return make_shared<Value>(false);
  return make_shared<Value>(true);
}

ValPtr Interpreter::And(ExprPtrList args){
  ValPtr ans;
  for(auto & arg:args){
    ans = eval(move(arg));
    if(not truthy(ans)) return ans;
  }
  return ans;
}

ValPtr Interpreter::Or(ExprPtrList args){
  ValPtr ans;
  for(auto & arg:args){
    ans = eval(move(arg));
    if(truthy(ans)) return ans;
  }
  return ans;
}

// control flow
ValPtr Interpreter::Begin(ExprPtrList args){
  ValPtr ans;
  for(auto const& arg :args){
    ans = eval(arg->clone());
  }
  return ans;
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

ValPtr Interpreter::While(ExprPtrList args){
  ValPtr result = nullptr;
  while(truthy(eval(args[0]->clone()))){
    for(auto & arg :args){
      result = eval(arg->clone());
    }
  }
  return result;
}

ValPtr Interpreter::Pipe(ExprPtrList args,bool left_to_right){
  if(args.size()<1){
   error("Expect at least one procedure.");
   return nullptr;
  }
  auto tok = Token(1, tok_id, "#pipe-arg");
  auto params = TokenList();
  params.push_back(tok);
  auto pipearg = make_unique<ExprId>(tok);
  ExprPtr lastcallee = nullptr;
  auto it1 = args.begin();
  auto it2 = args.rbegin();
  ExprPtr callee;
  while(true){
    // check termination
    if(left_to_right){
      if(it1 == args.end())  break;
    }else{
      if(it2 == args.rend()) break;
    }
    // load callee
    if(left_to_right){
      callee = (*it1)->clone();
    }else{
      callee = (*it2)->clone();
    }
    auto lst = ExprPtrList();
    if(not lastcallee){
      lst.push_back(move(pipearg));
      lastcallee = make_unique<ExprCall>(move(callee),
                                              move(lst),ExprPtrList());
    }else{
      lst.push_back(move(lastcallee));
      lastcallee = make_unique<ExprCall>(move(callee),
                                              move(lst),ExprPtrList());
    }
    // iteration
    if(left_to_right){
      it1++;
    }else{
      it2++;
    }
  }
  auto func = make_unique<ExprFunc>(params,move(lastcallee));
  return eval(move(func));
}

ValPtr Interpreter::Quote(ExprPtrList args){
  for(auto & arg:args){
    auto ast = make_shared<Value>(make_obj(move(arg)));
  }
  return nullptr;
}

// tuple
#define TUPLE_PRELUDE(SRC)                   \
  auto tmp1__  = eval(move(SRC));            \
  ValPtrList* pc;                            \
  if(not takeTuple(tmp1__, pc)){             \
    error("Expect Tuple as first argument"); \
    return nullptr;                          \
  }

#define NUMBER_PRELUDE(SRC) \
  double* pn;                                   \
  auto tmp2__   = eval(move(args[1]));          \
  if(not takeNumber(tmp2__, pn)){               \
    error("Expect number as second argument."); \
    return nullptr;                             \
  }

ValPtr Interpreter::Take(ExprPtrList args){
  TUPLE_PRELUDE(args[0]);
  NUMBER_PRELUDE(args[1]);
  int idx = (int)(*pn);
  if(idx<0 or idx>=(int)pc->size()){
    error("index out of range.");
    return nullptr;
  }
  return (*pc)[idx];
}

ValPtr Interpreter::Length(ExprPtrList args){
  TUPLE_PRELUDE(args[0]);
  return make_shared<Value>((double)pc->size());
}

ValPtr Interpreter::Set(ExprPtrList args){
  TUPLE_PRELUDE(args[0]);
  NUMBER_PRELUDE(args[1]);
  int idx = (int)(*pn);
  if(idx<0 or idx>(int)pc->size()){
    error("index out of range.");
    return nullptr;
  }
  auto v = eval(move(args[2]));
  (*pc)[idx] = v;
  return v;
}

ValPtr Interpreter::Push(ExprPtrList args){
  TUPLE_PRELUDE(args[0]);
  auto v= eval(move(args[1]));
  (*pc).push_back(v);
  return v;
}

ValPtr Interpreter::Pop(ExprPtrList args){
  TUPLE_PRELUDE(args[0]);
  if((*pc).empty()) return nullptr;
  auto v = (*pc).back();
  (*pc).pop_back();
  return v;
}

// list
ValPtr Interpreter::isEmpty(ExprPtrList args){
  auto v = eval(move(args[0]));
  if(v->type == VAL_Tuple or v->type == VAL_List){
    auto o = v->as.obj;
    switch(o->type){
      case objType::Tuple:{
        auto tup = cast(ObjTuple*,o);
        return make_shared<Value>(tup->container.size()==0);
      }
      case objType::List:{
        auto lst = cast(ObjList*,o);
        return make_shared<Value>(lst->tail==nullptr);
      }
      default:
        break;
    }
  }
  error("Expect tuple or list!");
  return nullptr;
}

ValPtr Interpreter::Cons(ExprPtrList args){
  auto head = eval(move(args[0]));
  auto v = eval(move(args[1]));
  Expect(v->type,VAL_List);
  auto tail = cast(ObjList*,v->as.obj);
  return make_shared<Value>(make_obj(head,tail));
}

ValPtr Interpreter::Head(ExprPtrList args){
  auto v = eval(move(args[0]));
  ValPtr* phead;
  ObjListPtr* ptail;
  if(not takeList(v,phead,ptail)){
    error("Expect a list");
    return nullptr;
  }
  // return part of list
  return *phead;
}

ValPtr Interpreter::Tail(ExprPtrList args){
  auto v = eval(move(args[0]));
  ValPtr* phead;
  ObjListPtr* ptail;
  if(not takeList(v,phead,ptail)){
    error("Expect a list");
    return nullptr;
  }
  if(*ptail){
    // return part of list
    return make_shared<Value>(*ptail);
  }else{
    return nullptr;
  }
}

// IO
ValPtr Interpreter::Print(ExprPtrList args){
  for(const auto &arg : args){
    auto v = eval(arg->clone());
    printVal(v);
  }
  std::cout<<std::endl;
  return std::make_shared<Value>();
}

// debug
ValPtr Interpreter::InspectEnv(ExprPtrList args){
  envs.top()->show();
  return nullptr;
}

ValPtr Interpreter::Debug(ExprPtrList args){
  interrupt = true;
  cout<<"[+] set break point"<<endl;
  return nullptr;
}

ValPtr Interpreter::Garbage(ExprPtrList args){
  Obj* p = objchain;
  while(p){
    cout<<p<<" ";
    p->show();
    if(p->type == objType::Primitive){
      auto o = cast(ObjPrimitive*,p);
      cout<<" arity:"<<o->arity;
    }
    cout<<endl;
    p = p->next;
  }
  return nullptr;
}

ValPtr Interpreter::Gc(ExprPtrList args){
  recycleMem(envs.top());
  return nullptr;
}

#undef Expect
#undef error
}
