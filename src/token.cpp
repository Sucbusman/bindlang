#include <iostream>
#include <iomanip>
#include "token.h"
#include "utils.h"
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
