Sources=$(shell ls src/*.cpp)
Objects=$(subst src/,obj/,$(subst .cpp,.o ,$(Sources)))
CCFLAG=-std=c++2a -Wall -O2 -march=native 
CC=g++
main: $(Objects)
	$(CC) $(Objects) -o $@
obj/%.o: src/%.cpp
	$(CC) $(CCFLAG) -c $< -o $@
clean:
	rm obj/*.o
option:
	@echo $(Sources)
	@echo $(Objects)
	@echo $(CCFLAG)
