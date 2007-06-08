PPU_CC=g++
SPU_CC=spu-elf-gcc
PPU_CFLAGS=-I../libfb -Iyuv2rgb -Iyuvscaler -Ispu_utils -I../../libfb/asm -O2
PPU_CFLAGS64=-m64 $(PPU_CFLAGS)
SPU_CFLAGS=-std=c99 -O2 -fno-exceptions -g -Ispu_utils
PPU_BIN=scaleandconvert
SPU_BIN0=spu_yuv2rgb
SPU_BIN1=spu_yuvscaler
PPU_LDFLAGS=yuvscaler/yuvscaler.c yuv2rgb/yuv2rgb.c -L../libfb -lfb -lspe2 
SPU_LDFLAGS=

all: ppu spu


64bit: ppu64 spu

ppu:
	$(PPU_CC) $(PPU_CFLAGS) scaleandconvert.cpp -o $(PPU_BIN) $(PPU_LDFLAGS)

ppu64:
	$(PPU_CC) $(PPU_CFLAGS64) scaleandconvert.cpp -o $(PPU_BIN)64 $(PPU_LDFLAGS)

spu:
	$(SPU_CC) $(SPU_CFLAGS) yuv2rgb/spu_yuv2rgb.c -o $(SPU_BIN0) $(SPU_LDFLAGS)
	$(SPU_CC) $(SPU_CFLAGS) yuvscaler/spu_yuvscaler.c -o $(SPU_BIN1) $(SPU_LDFLAGS)

clean:
	rm -rf *.o scaleandconvert spu_yuv2rgb spu_yuvscaler
