CC = gcc
CFLAGS = -g -Wall
DEPS = Shell.h
OBJ = Shell.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

Shell: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o