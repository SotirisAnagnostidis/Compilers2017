.PHONY: clean distclean default

export SHELL := /bin/bash

ifeq ($(shell uname -s),Darwin)
export PATH := /usr/local/opt/llvm/bin:$(PATH)
endif

CXXFLAGS=-Wall -g -std=c++11 `llvm-config --cxxflags`
LDFLAGS=`llvm-config --ldflags --system-libs --libs all`

default: dana

lexer.cpp: lexer.l
	flex -s -o lexer.cpp lexer.l

lexer.o: lexer.cpp parser.hpp

parser.hpp parser.cpp: parser.y
	bison -dv -o parser.cpp parser.y

#parser.o: parser.cpp parser.hpp ast.h

ast.o: ast.cpp ast.h 

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

libminibasic.a: writeInteger.o
	$(AR) $(ARFLAGS) $@ $^

dana: lexer.o parser.o general.o error.o symbol.o ast.o 
	$(CXX) $(CXXFLAGS) -o dana $^ $(LDFLAGS) -lfl

clean:
	$(RM) lexer.cpp parser.cpp parser.hpp parser.output dana a.* general.o *.o *~ *.out

distclean: clean
	$(RM) dana libminibasic.a
