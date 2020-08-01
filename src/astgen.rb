#!/bin/ruby
Exprs = {
  Atom:    ["Token literal"],
  Id:      ["Token id"],
  Define:  ["Token id","ExprPtr expr"],
  Func:    ["TokenList params","ExprPtr body"],
  Call:    ["ExprPtr callee","ExprPtrList args","ExprPtrList extra"],
  Tuple:   ["ExprPtrList container"],
  Quote:   ["ExprPtrList container"],
}

def expr_type
  "typedef enum{\n  "+
    Exprs.keys.map do |key|
      key.to_s.upcase
    end.join(",\n  ")+
  "\n} expr_type;\n"
end

Prelude= <<EOF
#ifndef __ast__
#define __ast__

#include <memory>
#include <vector>
#include <string>
#include <iomanip>

#include "token.h"
#include "utils.h"
#include "type.h"

namespace bindlang{
using std::cout;
using std::end;
using std::setw;
using TokenList = std::vector<Token>;

#{expr_type}

struct Expr {
  int type=-2;
  ~Expr() = default;
  virtual void    show() =0;
  virtual ExprPtr clone()=0;
  Expr(int type):type(type){};
};


struct ExprEof : Expr{
  ExprEof():Expr(-1){}
  void show(){
    cout<<"<Expr EOF >"<<endl;
  }
  ExprPtr clone(){
    return std::make_unique<ExprEof>(); 
  }
};
EOF

def init_list members
  members.map do |member|
    type,id = member.split(' ')
    if type == 'ExprPtr' or
       type == 'ExprPtrList' or
       type == 'TokenList'
      id+"(std::move(#{id}))"
    else
      id+'('+id+')'
    end
  end.join(',')
end

def connector type
  if type.include? 'Ptr'
    '->'
  else
    '.'
  end
end

def gen name,members
  ids = members.map {|s|s.split(' ')[-1]}
ans= <<EOF
struct Expr#{name} : Expr{
  #{members.map{|s| s+=";\n  "}.join }
  Expr#{name}(#{members.join(',')})
    : Expr(#{name.upcase}),#{init_list members}{}

  ExprPtr clone() override;

  void show() override {
    cout<<BLUE("<Expr ")<<"#{name} "<<endl;
    #{members.map do |member|
        type,id = member.split(' ')
        'cout<<setw(10)<<std::left<<"  '+id+':"<<endl;'+
                                                     "\n    "+
        if type.end_with? 'List'
          "for(auto &i:#{id}){\n    "+
          '  cout<<setw(14)<<" ";'+
          "i#{connector(type)}show();cout<<endl;\n    }"
        elsif type == "PrimFunc"
          'cout<<setw(14)<<" primitive "<<endl;'
        else
          "cout<<setw(14)<<' ';"+id+connector(type)+"show();cout<<endl;"
        end
      end.join("\n    ")
     }
    cout<<BLUE(" >")<<endl;
  }
};
EOF
end

Conclude = "};\n#endif"
if __FILE__ == $PROGRAM_NAME
  total = []
  total.append Prelude

  for k,v in Exprs
    total.append gen(k.to_s,v)
  end

  total.append Conclude
  puts total.join("\n")
end
