PPU_CC=g++
SPU_CC=spu-elf-gcc
PPU_CFLAGS=-I../../libfb -I../spu_utils -I../../libfb/asm -O2
PPU_CFLAGS64=-m64 $(PPU_CFLAGS)
SPU_CFLAGS= -O3 -fno-exceptions -Wall -g -I../spu_utils
PPU_BIN=yuv2argb_scaler
SPU_BIN=spu_yuv2argb_scaler
PPU_LDFLAGS=yuv2argb_scaler.c -L../../libfb -lfb -lspe2 
SPU_LDFLAGS=

all: ppu spu


64bit: ppu64 spu

ppu:
	$(PPU_CC) $(PPU_CFLAGS) example.cpp -o $(PPU_BIN) $(PPU_LDFLAGS)

ppu64:
	$(PPU_CC) $(PPU_CFLAGS64) example.cpp -o $(PPU_BIN)64 $(PPU_LDFLAGS)

spu:
	$(SPU_CC) $(SPU_CFLAGS) spu_yuv2argb_scaler.c -o $(SPU_BIN) $(SPU_LDFLAGS)

clean:
	rm -rf *.o yuv2rgb spu_yuv2rgb
