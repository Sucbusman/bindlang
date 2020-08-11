Sources=$(shell ls *.cpp)
Objects=$(subst .cpp,.o,$(Sources))
CCFLAG=-std=c++2a -Wall -march=native
CC=clang++
vm:$(Objects)
	$(CC) $(CCFLAG) $(Objects) -o $@
%.o:%.cpp
	$(CC) $(CCFLAG) -c $< -o $@

.PHONY:clean
clean:
	rm *.o
