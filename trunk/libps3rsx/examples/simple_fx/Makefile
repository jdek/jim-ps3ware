CFLAGS=-O2 -W -Wall -I/lib/modules/`uname -r`/build/include
CPPFLAGS=-O2 -lspe -W -Wall -I/lib/modules/`uname -r`/build/include

SRCC=../../src/fifo/dma.c ../../src/rop/rop.c ../../src/fifo/utils.c  ../../src/shaders/fragment.c ../../src/shaders/vertex.c  ../../src/textures/textures.c ../../src/geometry/geometry.c 
OBJC = $(SRCC:%.c=%.o)

all:gpu

spu_server : spu-srv.cpp
	spu-gcc -lm -Wall -O5 -finline-functions  -funroll-all-loops  spu-srv.cpp -o spu_server 
	
spu_handle.o: spu_server
	embedspu spu_handle spu_server spu_handle.o


objc: $(SRCC)
	$(CC) $(CFLAGS) $(SRCC) -c

gpu: $(OBJC) ps3gpu.cpp spu_handle.o
	$(CXX) $(CPPFLAGS) -o ps3gpu ps3gpu.cpp spu_handle.o $(OBJC) 


clean:
	$(RM) $(OBJC) ps3gpu
