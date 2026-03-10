CC := gcc
CFLAGS := -Wall -Wextra
OBJ_FLAG := -c
DEBUG  := -g
INCL_DIR := -Iinclude

SRC_DIR := src

all: src/ast.c src/process.c src/list.c src/parser.c src/pipeline.c src/utils.c src/lexer/*.c src/main.c src/shell.c src/io/input.c src/executor/*.c src/job.c src/sig.c src/builtin/*.c src/user.c
	$(CC) $(CFLAGS) $(INCL_DIR) $^ -o shell

debug: src/ast.c src/process.c src/list.c src/parser.c src/pipeline.c src/utils.c src/lexer/*.c src/main.c src/shell.c src/io/input.c src/executor/*.c src/job.c src/sig.c src/builtin/*.c src/user.c
	$(CC) $(CFLAGS) $(DEBUG) $(INCL_DIR) $^ -o shell

clean:
	rm  *.o a.out
