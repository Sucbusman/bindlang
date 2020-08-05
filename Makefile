Name=bindlang
EmacsDir=~/.emacs.d/mypackages/bindlang/
Sources=$(shell ls src/*.cpp)
Objects=$(subst src/,obj/,$(subst .cpp,.o ,$(Sources)))
CCFLAG=-std=c++2a -Wall -O2 -march=native -g
CC=clang++
$(Name): $(Objects)
	$(CC) $(CCFLAG) $(Objects) -o $@
obj/%.o: src/%.cpp
	$(CC) $(CCFLAG) -c $< -o $@

.PHONY:clean option ast install
clean:
	rm obj/*.o
option:
	@echo $(Sources)
	@echo $(Objects)
	@echo $(CCFLAG)
ast:
	cp -v src/ast.{h,bak}
	ruby src/astgen.rb > src/ast.h
emacs:
	cp -v editor-plugin/bindlang-mode.el $(EmacsDir)
	cd $(EmacsDir) && emacs --batch --eval '(byte-compile-file "bindlang-mode.el")'
install:
	cp -v $(Name) $(HOME)/.local/bin/$(Name)
