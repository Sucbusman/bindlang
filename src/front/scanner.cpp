#include <functional>
#include "front/scanner.h"

namespace bindlang{

const char oneCharTokens[]=
{
  '(',')',
  '{','}',
  '[',']',
  '<','>',
  ':',';','.',',',
  '|','\\','\'','\''
};

bool includes(const char& c,const char* list,
                      int len){
  for(int i=0;i<len;i++){
    if(c == list[i]){
      return true;
    }
  }
  return false;
}

inline bool isalpha(const char& c){
  return ('A'<=c and c<='Z') or
    ('a'<=c and c<='z');
}

inline bool isdigit(const char& c){
  return '0'<=c and c<='9';
}

inline bool isop(const char& c){
  switch(c){
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
      return true;
    default:
      return false;
  }
}

void Scanner::reset(){
  // cancling compative with stdio to speed up c++ io
  //std::ios::sync_with_stdio(false);
  //std::cin.tie(0);
  line_num  = 1;
  char_     = ' ';
  cache     = std::string();
  future    = fix_queue<char>(10);
  in >> std::noskipws;
}

void Scanner::debug(){
#define ln std::endl
#define cout std::cout
  cout<<"char_:"<<char_<<ln;
  cout<<"cache:"<<cache<<ln;
  cout<<"future:"<<ln;
  future.inspect();
#undef ln
}

bool Scanner::atEnd(){
  return future.empty() and in.eof();
}

void Scanner::nextChar(){
  if(future.empty()){
    in >> char_;
    cache.push_back(char_);
  }else{
    char_ = future.pop();
    cache.push_back(char_);
  }
}

char Scanner::borrow(){
  if(not in.eof()){
    char c;
    in>> c;
    future.push(c);
    return c;
  }else{
    //return blank char when eof so that it does not effect
    return ' ';
  }
}

char Scanner::peekNext(){
  if(future.empty()){
    return borrow();
  }else{
    return future.front();
  }
}

template <typename ...Arg>
bool Scanner::error(Arg... arg){
  std::cerr<<"[Scanner Error] At line "<<line_num;
  (std::cerr<< ... <<arg) << endl;
  std::cerr<<" with cache "<<cache
           <<" ascii:"<<(int)char_<<std::endl;
  return false;
}

Token Scanner::makeToken(toktype type){
  Token tok = Token(line_num,type,std::string(cache));
  cache.clear();
  return tok;
}

Token Scanner::makeToken(char type){
  Token tok = Token(line_num,type,std::string(cache));
  cache.clear();
  return tok;
}

bool Scanner::skipUntil(std::function<bool(char)> cond){
  nextChar();
  while(not in.eof()){
    if(cond(char_)){
      return true;
    }
    nextChar();
  }
  return false;
}

bool Scanner::skipUntilNext(std::function<bool(char)> cond){
  // require future is empty now
  while(not in.eof()){
    char ch = borrow();
    if(cond(ch)){
      return true;
    }
    nextChar();
  }
  return false;
}

void Scanner::skipWhitespace(){
  while(not atEnd()){
    switch(char_){
    case ' ' :
    case '\r':
    case '\n':
      line_num++;
    case '\t':// fall down
      cache.clear();
      nextChar();
      break;
    case ';':
      skipUntil([](char ch){return ch=='\n';});
      break;
    default:
      return;
    }
  }
}

Token Scanner::nextToken(){
  /* If we next char at start, make sure it point
     to the last char in a whole token when returned */
  nextChar();
  skipWhitespace();
  if(atEnd()) return makeToken(tok_eof);

  if(includes(char_,
              oneCharTokens,
              sizeof(oneCharTokens)))
      return makeToken(char_);
  if(isdigit(char_)) return tokNumber();

  switch (char_) {
    //case '\n': return tokNewline();
    case '"':  return tokString();
    case '-': {
      char ch = peekNext();
      if (ch == '>') {
        nextChar();
        return makeToken(tok_arrow);
      } else if(isIdStart(ch) or isdigit(ch)){
        nextChar();
        return tokIdentify();
      }else
        return makeToken(tok_id);
    }
    case '=': {
      char ch = peekNext();
      if (ch == '=') {
        nextChar();
        return makeToken(tok_equal);
      }else if(ch == '>'){
        nextChar();
        return makeToken(tok_id);
      }
      else
        return makeToken('=');
    }
  }
  if(isIdStart(char_)) return tokIdentify();
  return makeToken(tok_err);
}

/*
Token Scanner::tokNewline(){
  line_num++;
  return makeToken(tok_newline);
}
*/

Token Scanner::tokNumber(){
  auto fnot = [](std::function<bool(char)> f){return [f](char ch){return not f(ch);};};
  auto ishex = [](char ch){
    return isdigit(ch) or ('a'<=ch and ch<='f');
  };
  auto isoct = [](char ch){return '0'<=ch and ch<='7';};
  auto isbin = [](char ch){return ch=='0' or ch=='1';};
  auto notdigit = fnot(isdigit);
  skipUntilNext(notdigit);
  char ch = peekNext();
  if(cache.size()==1 and cache[0] == '0'){
    switch(ch){
#define NUMTOK(F,T)                             \
        nextChar();                             \
        cache.clear();                          \
        skipUntilNext(fnot(F));                 \
        return makeToken(T);
      case 'x': NUMTOK(ishex,tok_num_hex);
      case 'o': NUMTOK(isoct,tok_num_oct);
      case 'b': NUMTOK(isbin,tok_num_bin);
      default:
        //just 0
        return makeToken(tok_num);
    }
  }
  //else try float
  if(ch == '.'){
    ch = borrow();
    if(not isdigit(ch)){
      //eg. 3.double
      //    ^
      return makeToken(tok_num);
    }
    nextChar();//eat '.'
    nextChar();
    skipUntilNext(notdigit);
  }
  return makeToken(tok_num);
}

Token Scanner::handleEscape(Token& tok){
  auto s = tok.literal;
  auto type = tok.type;
  string ans;
  for(int i=0;i<s.size();){
    if(s[i] == '\\'){
      if(i+1<s.size()){
        switch(s[i+1]){
          case 'n':ans.push_back('\n');break;
          case 't':ans.push_back('\t');break;
          case 'f':ans.push_back('\f');break;
          case '\\':ans.push_back('\\');break;
          default:
            error("malform escape string with ",
                  s[i+1],"after \\");
            type = tok_err;
            break;
        }
        i+=2;
      }else{
        error("malform escape string");
        type = tok_err;
        ++i;
      }
    }else{
      ans.push_back(s[i]);
      ++i;
    }
  }
  return Token(tok.line,type,ans);
}

Token Scanner::tokString(){
  cache.clear();//eat '"'
  skipUntil([this](char ch)mutable{
              if(ch=='\n') line_num++;
              return ch=='"'; 
            })
    or error("expect closing right quote");
  cache.pop_back();//pop closing '"'
  auto tok = makeToken(tok_str);
  return handleEscape(tok);
}

bool Scanner::isIdStart(char ch){
  if(isalpha(ch) or isop(ch) or ch=='_' or ch=='#'){
    return true;
  }
  else
    return false;
}

Token Scanner::tokIdentify(){
  while(not atEnd()){
    char ch = borrow();
    if(ch == '-'){
      if(atEnd()) break;
      char ch2 = borrow();
      if(ch2 == '>'){
        return makeToken(tok_id);
      }else{
        nextChar();nextChar();//eat twe borrowed char
      }
    }
    else if(not (ch=='?' or
                 ch=='!' or
                 ch=='_' or
                 ch=='>' or
                 isIdStart(ch) or
                 isdigit(ch))) break;
    else
      nextChar();
  }
  return makeToken(tok_id);
}

}//namespace
