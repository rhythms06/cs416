CC = gcc
CFLAGS = -g -Wall
DEPS = Shell.h
OBJ = Shell.o

all: Shell

Shell: $(OBJ)
	$(CC) $^ -o $@ $(CFLAGS)

$(OBJ): Shell.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: clean
clean:
	rm -f *.o Shell