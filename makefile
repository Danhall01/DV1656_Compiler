VERSION = std=c2x
COMPILE_FLAGS = -g -O0 -Wall -Wextra
TIDY_FLAGS =

LEX_TARGET = lexer.lex
APP_NAME = app.out

all: lex lex.yy.o
	clang $(COMPILE_FLAGS) -o $(APP_NAME) lex.yy.o

lex:
	flex $(LEX_TARGET)

lex.yy.o: lex.yy.c
	clang -c $(COMPILE_FLAGS) lex.yy.c


format:
	clang-format $(wildcard *.c) -style=file --verbose

tidy:
	clang-tidy $(wildcard *.c) -checks=-*,clang-diagnostic-*,clang-analyzer-*,readability-*,bugprone-*

run: all
	./$(APP_NAME)
clean:
	rm *.o lex.yy.c $(APP_NAME)
