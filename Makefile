Sources=$(shell ls src/*.cpp)
Objects=$(subst src/,obj/,$(subst .cpp,.o ,$(Sources)))
CCFLAG=-std=c++2a -Wall -O2 -march=native -g
CC=g++
main: $(Objects)
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
	cp -v src/ast.{h,bak}
	ruby src/astgen.rb > src/ast.h
