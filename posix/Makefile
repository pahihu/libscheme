INC=-I..
LIB=-L..
CC=gcc -flto
CFLAGS=$(INC) -DGC_THREADS -O3 -fomit-frame-pointer
STRIP= strip

OBJS = posix_file.o posix_proc.o

posixscheme: $(OBJS) main.o
	$(CC) $(CFLAGS) -o posixscheme $(OBJS) main.o $(LIB) -lscheme -lm
	$(STRIP) -x -S $@

clean:
	$(RM) $(OBJS) main.o *~ posixscheme

