Name=bindlang
Prefix=/usr
EmacsDir=~/.emacs.d/mypackages/bindlang
#Sources=$(shell ls src/*.cpp)
Sources=$(shell find src/ -name '*.cpp' -not -path "src/vm/*")
Objects=$(subst src/,obj/,$(subst .cpp,.o ,$(Sources)))
CCFLAG=-std=c++2a -Wall -O2 -march=native -I src/
CC=clang++
$(Name): $(Objects)
	$(CC) $(Objects) -o $@
obj/%.o: src/%.cpp
	mkdir -p $$(dirname $@)
	$(CC) $(CCFLAG) -c $< -o $@

.PHONY:clean option ast install
clean:
	rm obj/*.o
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
