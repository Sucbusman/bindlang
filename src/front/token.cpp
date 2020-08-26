#include <iostream>
#include <iomanip>
#include "front/token.h"
#include "util/utils.h"
namespace bindlang{

const char *toktype_str_list[] = {
  "tok_eof"  , 
  "tok_id"   , 
  "tok_num"  , 
  "tok_num_bin"  , 
  "tok_num_oct"  , 
  "tok_num_hex"  , 
  "tok_str"  , 
  "tok_err"  , 
  "tok_arrow", 
  "tok_equal",
  "tok_newline"
};

std::ostream & operator<<(std::ostream &out, const Token &tok)
{
    out<<BLUE("<Token")<<" type:";
    if(tok.type<0){
      out<<std::setw(9)<<std::left<<toktype_str_list[(tok.type*-1)-1];
    }else{
      out<<std::setw(9)<<std::left<<(char)tok.type;
    }
    out<<" literal:"<<std::setw(9)<<std::left<<tok.literal<<" line:"<<tok.line
       <<BLUE(" >");
    return out;
}

void Token::show(){
  cout<<*this;
}

}//namespace
