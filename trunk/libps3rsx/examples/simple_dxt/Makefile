CFLAGS=-O2 -W -Wall -I/lib/modules/`uname -r`/build/include
CPPFLAGS=-O2 -W -Wall -I/lib/modules/`uname -r`/build/include

SRCC=../../src/fifo/utils.c  ../../src/textures/textures.c ../../src/geometry/geometry.c ps3gpu.c
OBJC = $(SRCC:%.c=%.o)

all:gpu


objc: $(SRCC)
	$(CC) $(CFLAGS) $(SRCC) -c

gpu: $(OBJC) 
	$(CXX) -o ps3gpu $(OBJC) 


clean:
	$(RM) $(OBJC) ps3gpu
