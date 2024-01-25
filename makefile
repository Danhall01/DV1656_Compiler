# Write the name of the compile target here, will be used in "make run"
override COMPILE_TARGET ?=

VERSION = std=c2x
COMPILE_FLAGS = -g -O0 -Wall -Wextra
COMPILE_FLAGS_EXTRA =

TIDY_FLAGS = -*,clang-diagnostic-*,clang-analyzer-*,readability-*,bugprone-*


LEX_TARGET = lexer.lex
BISON_TARGET = parser
APP_NAME = app.out
TREE_TARGET = tree

all: compile
	clang $(COMPILE_FLAGS) -o $(APP_NAME) $(wildcard *.o) $(COMPILE_FLAGS_EXTRA)

compile: lex lex.yy.c bison $(BISON_TARGET).tab.c
	clang -c $(COMPILE_FLAGS) $(wildcard *.c) $(COMPILE_FLAGS_EXTRA)

lex:
	flex $(LEX_TARGET)

bison:
	bison $(BISON_TARGET).y

tree: 
	dot -Tpdf $(TREE_TARGET).dot -o$(TREE_TARGET).pdf

format:
	clang-format $(wildcard *.c) -style=file --verbose

tidy:
	clang-tidy $(wildcard *.c) -checks=$(TIDY_FLAGS)

run: all
	./$(APP_NAME) $(COMPILE_TARGET)
clean:
	rm *.o $(BISON_TARGET).tab.c $(BISON_TARGET).tab.h lex.yy.c $(APP_NAME) tree.dot tree.pdf
