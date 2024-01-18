VERSION = std=c2x
COMPILE_FLAGS = -g -O0 -Wall -Wextra -Werror
TIDY_FLAGS =

APP_NAME = app.out

ALL_TARGETS = main.o
TARGET_FILE = main.c

all: format $(ALL_TARGETS)
	clang $(COMPILE_FLAGS) -o $(APP_NAME) main.o


# Add each file like this and add to ALL_TARGETS list
main.o: main.c
	clang -c $(COMPILE_FLAGS) main.c


format:
	clang-format $(wildcard *.c) -style=file --verbose

tidy:
	clang-tidy $(wildcard *.c) -checks=-*,clang-diagnostic-*,clang-analyzer-*,readability-*,bugprone-*
clean:
	rm *.o app.out
