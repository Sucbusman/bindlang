Name=bindlang
Prefix=/usr
EmacsDir=~/.emacs.d/mypackages/bindlang
Sources=$(shell find src/ -name '*.cpp')
Objects=$(subst src/,build/,$(subst .cpp,.o ,$(Sources)))
CCFLAG=-std=c++2a -Wall -march=native -I src/
CC=clang++
$(Name): $(Objects)
	$(CC) $(Objects) -o $@
build/%.o: src/%.cpp
	mkdir -p $$(dirname $@)
	$(CC) $(CCFLAG) -c $< -o $@

.PHONY:clean option ast install
clean:
	find build/ -name *.o -delete
option:
	@echo $(Sources)
	@echo $(Objects)
	@echo $(CCFLAG)
ast:
	ruby src/define/astgen.rb > src/define/ast.h
emacs:
	cp -v editor-plugin/bindlang-mode.el $(EmacsDir)
	cd $(EmacsDir) && emacs --batch --eval '(byte-compile-file "bindlang-mode.el")'
install:
	cp -v $(Name) $(Prefix)/local/bin/$(Name)
