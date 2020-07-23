Sources=$(shell ls src/*.cpp)
Headers=$(shell ls src/*.h)
Objects=$(subst src/,obj/,$(subst .cpp,.o ,$(Sources)))
CCFLAG=-std=c++2a -Wall -O2 -march=native 
CC=clang++
main: $(Objects) $(Headers)
	$(CC) $(CCFLAG) $(Objects) -o $@
obj/%.o: src/%.cpp
	$(CC) $(CCFLAG) -c $< -o $@

.PHONY:clean option ast
clean:
	rm obj/*.o
option:
	@echo $(Sources)
	@echo $(Objects)
	@echo $(CCFLAG)
ast:
	ruby src/astgen.rb > src/ast.h
