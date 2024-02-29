# Write the name of the compile target here, will be used in "make run"
override COMPILE_TARGET ?= test_files/semantic_errors/DuplicateIdentifiers.java


VERSION = std=c2x
COMPILE_FLAGS = -g -O0 -Wall -Wextra
COMPILE_FLAGS_EXTRA =

TIDY_FLAGS = -*,clang-diagnostic-*,clang-analyzer-*,readability-*,bugprone-*


LEX_TARGET = lexer.lex
BISON_TARGET = parser
TREE_TARGET = tree
ST_TARGET = st
APP_NAME = compiler

all: compile lex.o symboltable.o node.o parser.o semantic.o main.o
	clang $(COMPILE_FLAGS) -o $(APP_NAME) main.o SymbolTable.o Semantic.o lex.yy.o Node.o $(BISON_TARGET).tab.o $(COMPILE_FLAGS_EXTRA)

compile: lex lex.yy.c bison $(BISON_TARGET).tab.c

main.o:
	clang -c $(COMPILE_FLAGS) main.c $(COMPILE_FLAGS_EXTRA)
lex.o:
	clang -c $(COMPILE_FLAGS) lex.yy.c $(COMPILE_FLAGS_EXTRA)
node.o:
	clang -c $(COMPILE_FLAGS) Node.c $(COMPILE_FLAGS_EXTRA)
parser.o:
	clang -c $(COMPILE_FLAGS) $(BISON_TARGET).tab.c $(COMPILE_FLAGS_EXTRA)
symboltable.o:
	clang -c $(COMPILE_FLAGS) SymbolTable.c $(COMPILE_FLAGS_EXTRA)
semantic.o:
	clang -c $(COMPILE_FLAGS) Semantic.c $(COMPILE_FLAGS_EXTRA)

lex:
	flex $(LEX_TARGET)

bison:
	bison $(BISON_TARGET).y

tree: run
	dot -Tpdf $(TREE_TARGET).dot -o$(TREE_TARGET).pdf

st: run
	dot -Tpdf $(ST_TARGET).dot -o$(ST_TARGET).pdf

format:
	clang-format $(wildcard *.c) -style=file --verbose

tidy:
	clang-tidy $(wildcard *.c) -checks=$(TIDY_FLAGS)

run: all
	./$(APP_NAME) $(COMPILE_TARGET)
clean:
	rm *.o $(BISON_TARGET).tab.c $(BISON_TARGET).tab.h lex.yy.c $(APP_NAME) tree.dot tree.pdf st.dot st.pdf
