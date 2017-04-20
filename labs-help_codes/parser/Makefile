.PHONY: clean distclean default

CC=gcc
CFLAGS=-Wall

default: minibasic

lexer.c: lexer.l
	flex -s -o lexer.c lexer.l

lexer.o: lexer.c my_parser.h

my_parser.h my_parser.c: my_parser.y
	bison -dv -o my_parser.c my_parser.y

my_parser.o: my_parser.c

minibasic: lexer.o my_parser.o
	$(CC) $(CFLAGS) -o minibasic $^ -lfl

clean:
	$(RM) lexer.c my_parser.c my_parser.h my_parser.output *.o *~

distclean: clean
	$(RM) minibasic
