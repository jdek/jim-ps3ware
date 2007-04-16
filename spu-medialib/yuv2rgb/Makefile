PPU_CC=g++
SPU_CC=spu-gcc
PPU_CFLAGS=
PPU_CFLAGS64=-m64 $(PPU_CFLAGS)
SPU_CFLAGS=-std=c99 -O2
PPU_BIN=yuv2rgb
SPU_BIN=spu_yuv2rgb
PPU_LDFLAGS=-lspe2
SPU_LDFLAGS=

all: ppu spu


64bit: ppu64 spu

ppu:
	$(PPU_CC) $(PPU_CFLAGS) yuv2rgb.cpp -o $(PPU_BIN) $(PPU_LDFLAGS)

ppu64:
	$(PPU_CC) $(PPU_CFLAGS64) yuv2rgb.cpp -o $(PPU_BIN)64 $(PPU_LDFLAGS)

spu:
	$(SPU_CC) $(SPU_CFLAGS) spu_yuv2rgb.c -o $(SPU_BIN) $(SPU_LDFLAGS)

clean:
	rm -rf *.o yuv2rgb spu_yuv2rgb
