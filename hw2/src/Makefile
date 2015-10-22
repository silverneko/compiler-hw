TARGET = scanner
OBJECT = lex.yy.o symboltable.o 
CC = cc -g
LEX = flex
LIBS = -lfl 
CXX = g++
CXXFLAGS = -std=c++11

scanner:	lex.yy.o symboltable.o
	$(CXX) -o scanner lex.yy.o symboltable.o $(CXXFLAGS)

symboltable.o: symboltable.c 
	cp symboltable.c symboltable.cpp
	$(CXX) -c symboltable.cpp $(CXXFLAGS)

lex.yy.o: lex.yy.c 
	cp lex.yy.c lex.yy.cpp
	$(CXX) -c lex.yy.cpp $(CXXFLAGS)

lex.yy.c: lexer.l
	$(LEX) $(LIB) lexer.l

test: scanner
	cat ../test/test.c | ./scanner

clean:
	rm -f $(TARGET) $(OBJECT) 

