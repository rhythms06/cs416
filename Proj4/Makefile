CC = gcc
CFLAGS = -g -Wall
DEPS = Shell.h
OBJ = Shell.o

Shell: $(OBJ)
	$(CC) $^ -o $@ $(CFLAGS)

%.o: Shell.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: clean
clean:
	rm -f *.o