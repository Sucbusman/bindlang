Sources=$(shell find *.cpp)
Objects=$(subst .cpp,.o,$(Sources))
CCFLAG=-std=c++2a -Wall -march=native
CC=clang++
bdvm:$(Objects)
	$(CC) $(CCFLAG) $(Objects) -o $@
%.o:%.cpp
	$(CC) $(CCFLAG) -c $< -o $@

.PHONY:clean
clean:
	rm *.o
