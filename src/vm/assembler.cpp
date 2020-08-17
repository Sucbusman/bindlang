#include "vm/assembler.h"
#include "vm/value.h"
namespace bindlang::vm{
#define INSTALL_NO_ARG(f)           \
  f(PUSH) f(POP)  f(RET)            \
  f(ADD) f(MINUS) f(MULT) f(DIVIDE) \
  f(EQ)  f(LT) f(GT)                \
  f(HALT) f(CALL)

#define INSTALL_ONE_ARG(f)\
  f(SETL) f(GETL) f(CNST) f(IMM) f(SYSCALL)

#define INSTALL_JMP(f)\
  f(JMP) f(JNE)

void Assembler::loadOp(){
#define noArgFunc(OP) \
  operators[#OP] = [this](){coder.OP();return true;};
#define oneArgFunc(OP) \
  operators[#OP] = [this](){\
    auto tok = expect('$');                     \
    coder.OP((uint32_t)atoi(tok.c_str()+1));    \
    return true;};
#define jmpFunc(OP) \
  operators[#OP] = [this](){                           \
    auto tok = expect('@');                            \
    auto it = lables.find(tok);                        \
    auto lable_exist = it!=lables.end();               \
    size_t offset = 0;                                 \
    if(lable_exist){                                   \
      offset = it->second-coder.tellp();               \
      lables[tok] = offset;                            \
    }else{                                             \
      unfills[tok].push_back(coder.tellp());           \
    }                                                  \
    coder.OP(offset);                                  \
    return true;                                       \
  };
  INSTALL_NO_ARG(noArgFunc);
  INSTALL_ONE_ARG(oneArgFunc);
  INSTALL_JMP(jmpFunc);
#undef noArgFunc
#undef oneArgFunc
#undef jmpFunc
}

inline bool isalpha(char ch){
  return ('A'<=ch and ch<='Z') or
    ('a'<=ch and ch<='z');
}

inline bool isdigit(char ch){
  return '0'<=ch and ch<= '9';
}

void Assembler::reset(){
  lchar = ' ';
  nchar = ' ';
  cache.clear();
  in >> std::noskipws;
}

template <class... Args>
bool Assembler::error(Args...args){
  cerr<<"Error"<<" at line "<<line_num<<":";
  (cerr<< ... <<args)<<endl;
  error_num ++;
  return false;
}

bool Assembler::hasError(){
  return error_num >0 or in.fail();
}

void Assembler::eat(){
  lchar = nchar;
  cache.push_back(lchar);
  if(not in.eof()){
    in>>nchar;
  }else{
    nchar = '\0';
  }
}

char Assembler::peek(){
  return nchar;
}

bool Assembler::atEnd(){
  return in.eof();
}

bool Assembler::skipUntil(std::function<bool(char)> f){
  while(not f(nchar) and not atEnd()){
    eat();
  }
  return not atEnd();
}

void Assembler::parse(){
  parseConstant();
  parseCode();
  fillConstant();
}

void Assembler::parseConstant(){
  string tok;
  while(tok = token(),tok!=tokEof()){
    switch (tok[0]){
      case '=':return;
      case '@':{
        auto slot = coder.tellcp();
        coder.addConst(Value());//place holder
        auto arity = expect('$');
        funcs.push_back(func{tok,atoi(arity.c_str()+1),slot});
        break;
      }
      case '"':
        coder.addConst(Value(make_obj(tok.c_str()+1)));
        break;
      case '$':
        coder.addConst(Value((uint32_t)atoi(tok.c_str()+1)));
        break;
      default:
        error("Expect constant but get ",tok);
        exit(1);
    }
  }
}

void Assembler::parseCode(){
  string tok="unused";
  while(not hasError() and tok != tokEof()){
    tok = token();
    auto it = operators.find(tok);
    if(it!=operators.end()){
      it->second();
    }else{
      switch(tok[0]){
        case '@':{
          auto pc = coder.tellp();
          auto it = lables.find(tok);
          auto it2 = unfills.find(tok);
          bool lable_exist = it!=lables.end();
          bool lable_unfill = it2!=unfills.end();
          if(lable_exist){
            error("Multiple label ",tok);
          }
          else if(lable_unfill){
            auto callsites = it2->second;
            for(auto callsite:callsites){
              coder.modify(callsite+1,pc-callsite);//patch
            }
          }else{
            lables[tok] = pc;
          }
          break;
        }
        default:
          error("unexpect token ",tok);
      }
    }
  }
}

void Assembler::fillConstant(){
  for(auto f:funcs){
    auto it = lables.find(f.name);
    if(it!=lables.end()){
      coder.constants[f.slot] =
        Value(make_obj(f.name,f.arity,it->second));
    }else{
      // function no used,omit for optimized
    }
  }
}

string Assembler::expect(char type){
  auto tok = token();
  if (tok[0] == type){
    return tok;
  }else{
    error("expect ",type," but get ",tok[0]);
    exit(1);
  }
}

void Assembler::skipWhitespace(){
  while(not atEnd()){
    switch(lchar){
    case ' ' :
    case '\r':
    case '\n':
      line_num++;
    case '\t':// fall down
      cache.clear();
      eat();
      break;
    case '#':
      skipUntil([](char ch){return ch=='\n';});
      eat();
      break;
    default:
      return;
    }
  }
}

string Assembler::token(){
  /* look ahead twe char ,
   * every parser responds to 
   * move  one char at end
   */
  skipWhitespace();
  if(hasError()) exit(1);
  if(isdigit(lchar)) return tokNumber();
  if(isalpha(lchar)) return tokOperator();
  switch(lchar){
    case '-':
      if(not isdigit(peek())){
        error("Expect number after '-' ");
        break;
      }
      return tokNumber();//minus number 
    case '"': return tokString();
    case '@': return tokLabel();
    case '=': return tokDelimline();
    case '\0': return tokEof();
    default: error("Unexpected character ",lchar);break;
  }
  return "";
}

string Assembler::makeTok(){
  string tok = cache;
  cache.clear();
  eat();//move ahead
  return tok;
}

string Assembler::tokNumber(){
  skipUntil([](char ch){
    return not isdigit(ch);
  });
  return "$"+makeTok();
}

string Assembler::tokString(){
  //not eat leading '"'
  skipUntil([](char ch){
    return ch=='"';
  });
  auto tok = makeTok();
  eat();//eat closing '"'
  return tok;
}

string Assembler::tokLabel(){
  // not eat leading '@'
  skipUntil([](char ch){
    return not isalpha(ch);
  });
  return makeTok();
}

string Assembler::tokOperator(){
  return tokLabel();
}

string Assembler::tokDelimline(){
  skipUntil([](char ch){
    return ch!='=';
  });
  cache.clear();
  eat();
  return "=";
}

inline string Assembler::tokEof(){
  return "\0EOF";
}

void Assembler::scan(){
  while(not hasError() or not atEnd()){
    cout<<token()<<endl;
  }
}

void Assembler::write(const char* path){
  parse();
  coder.writeBinary(path);
  return;
}

#undef INSTALL_NO_ARG
#undef INSTALL_ONE_ARG
#undef INSTALL_JMP
}
