CFLAGS=-W -Wall  -I/lib/modules/`uname -r`/build/include

init: init.c ../fifo/utils.c
	$(CC) $(CFLAGS) -o init init.c ../fifo/utils.c 
