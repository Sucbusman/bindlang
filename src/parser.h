#ifndef __parser__
#define __parser__
#include "scanner.h"
namespace bindlang {

class Parser {
 public:
  Parser(Scanner &scanner) : scanner(scanner) { reset(); }
  void reset() {} // namespace bindlang

 private:
  Scanner& scanner;
};

} // namespace bindlang
#endif
