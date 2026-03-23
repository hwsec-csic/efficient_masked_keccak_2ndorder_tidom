BINARY = intermediates
OBJS   = intermediates.o

CC = gcc
CFLAGS = -Wall -O3

INCLUDES = -I/usr/include/hdf5/serial
LDFLAGS  = -L/usr/lib/x86_64-linux-gnu/hdf5/serial
LIBS     = -lhdf5

$(BINARY): $(OBJS)
	$(CC) $(LDFLAGS) -o $(BINARY) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(BINARY)