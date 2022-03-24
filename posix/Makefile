INC=-I..
LIB=-L..
CC=gcc
CFLAGS=$(INC) -DGC_THREADS -Os

OBJS = posix_file.o posix_proc.o

posixscheme: $(OBJS) main.o
	$(CC) $(CFLAGS) -o posixscheme $(OBJS) main.o $(LIB) -lscheme -lm
	strip -x -S $@

clean:
	$(RM) $(OBJS) main.o *~ posixscheme

