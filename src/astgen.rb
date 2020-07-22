Exprs = {
  Define: ["Token id","ExprPtr expr"],
  Call:   ["Token id","ExprPtr expr"],
  Func:   ["Token id","ExprPtrList args"]
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

#include "token.h"
#include "value.h"
namespace bindlang{

struct Expr;//declared ahead
struct Visitor{
  virtual Value visit(Expr& expr)=0;
};

#{expr_type}

struct Expr {
  int type=-1;
  ~Expr() = default;
  Value accept(Visitor & v);
  Expr(int type):type(type){};
};

using ExprPtr = std::unique_ptr<Expr>;
using ExprPtrList = std::vector<ExprPtr>;
using IdList = std::vector<std::string>;
EOF


def init_list members
  members.map do |member|
    type,id = member.split(' ')
    if type == 'ExprPtr'
      id+"(std::move(#{id}))"
    else
      id+'('+id+')'
    end
  end.join(',')
end

def gen name,members
ans= <<EOF
struct Expr#{name} : Expr{
  #{members.map{|s| s+=";\n  "}.join }
  Expr#{name}(#{members.join(',')}):
    Expr(#{name.upcase}),#{init_list members}{};
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
