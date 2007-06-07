PPU_CC=g++
SPU_CC=spu-elf-gcc
PPU_CFLAGS=-I../../libfb -I../spu_utils -O2 yuvscaler.c 
PPU_CFLAGS64=-m64 $(PPU_CFLAGS)
SPU_CFLAGS=-O2 -O2 -fno-exceptions -g -I../spu_utils
PPU_BIN=yuvscaler
SPU_BIN=spu_yuvscaler
PPU_LDFLAGS=-lspe2
SPU_LDFLAGS=

all: ppu spu


64bit: ppu64 spu

ppu:
	$(PPU_CC) $(PPU_CFLAGS) example.cpp -o $(PPU_BIN) $(PPU_LDFLAGS)

ppu64:
	$(PPU_CC) $(PPU_CFLAGS64) example.cpp -o $(PPU_BIN)64 $(PPU_LDFLAGS)

spu:
	$(SPU_CC) $(SPU_CFLAGS) spu_yuvscaler.c -o $(SPU_BIN) $(SPU_LDFLAGS)

clean:
	rm -rf *.o yuvscaler spu_yuvscaler
