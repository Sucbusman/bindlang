OBJS=test.o lexer.o
HS=$(shell ls *.h)
test:$(OBJS) $(HS)
	g++ $^ -o $@
%.o:%.cpp
	g++ -c $^ -o $@
.PHONY:clean

clean:
	rm *.o test
