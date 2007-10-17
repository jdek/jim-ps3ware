include ../config.mak

all: src

src:
	for a in $(PROG); do $(CC) $(CFLAGS) $${a}.c -c $(INC);done

clean:
	rm -rf *.o
