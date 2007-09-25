PPU_CC=g++
SPU_CC=spu-elf-gcc
PPU_CFLAGS=-I../libfb -Iyuv2rgb -Iyuvscaler -Ispu_utils -I../../libfb/asm -O2
PPU_CFLAGS64=-fno-exceptions -m64 $(PPU_CFLAGS)
SPU_CFLAGS=-std=c99 -O2 -fno-exceptions -g -Ispu_utils
PPU_BIN=scaleandconvert
SPU_BIN0=spu_yuv2rgb
SPU_BIN1=spu_yuvscaler
PPU_LDFLAGS=yuvscaler/yuvscaler.c yuv2rgb/yuv2rgb.c -L../libfb -lfb -lspe2 
SPU_LDFLAGS=
PREFIX=/usr/local
INS_BIN=spu_yuv2rgb spu_yuvscaler
INS_LIB=libspu-medialib.a ../libfb/libfb.a
INS_INC=spu_utils/spu_control.h yuv2rgb/yuv2rgb.h yuvscaler/yuvscaler.h ../libfb/libfb.h

all: ppu spu


64bit: ppu64 spu

ppu:
	$(PPU_CC) $(PPU_CFLAGS) scaleandconvert.cpp -o $(PPU_BIN) $(PPU_LDFLAGS)

ppu64:
	$(PPU_CC) $(PPU_CFLAGS64) scaleandconvert.cpp -o $(PPU_BIN)64 $(PPU_LDFLAGS)

spu:
	$(SPU_CC) $(SPU_CFLAGS) yuv2rgb/spu_yuv2rgb.c -o $(SPU_BIN0) $(SPU_LDFLAGS)
	$(SPU_CC) $(SPU_CFLAGS) yuvscaler/spu_yuvscaler.c -o $(SPU_BIN1) $(SPU_LDFLAGS)

lib: libspu-medialib.a

libspu-medialib.a:
	$(PPU_CC) $(PPU_CFLAGS64) -c yuvscaler/yuvscaler.c -o yuvscaler.o
	$(PPU_CC) $(PPU_CFLAGS64) -c yuv2rgb/yuv2rgb.c -o yuv2rgb.o
	ar rcs libspu-medialib.a yuvscaler.o yuv2rgb.o

clean:
	rm -rf *.o scaleandconvert spu_yuv2rgb spu_yuvscaler libspu-medialib.a

install: install-spu install-lib install-inc

install-spu: spu
	install -d "$(PREFIX)/bin"
	install -m 755 $(INS_BIN) "$(PREFIX)/bin"

install-lib: lib
	install -d "$(PREFIX)/lib64"
	install -m 644 $(INS_LIB) "$(PREFIX)/lib64"

install-inc:
	install -d "$(PREFIX)/include/spu-medialib"
	install -m 644 $(INS_INC) "$(PREFIX)/include/spu-medialib"

uninstall:
	rm -f $(addprefix $(PREFIX)/bin/, $(INS_BIN))
	rm -f $(addprefix $(PREFIX)/lib64/, $(notdir $(INS_LIB)))
	rm -f $(addprefix $(PREFIX)/include/spu-medialib/, $(notdir $(INS_INC)))
	-rmdir "$(PREFIX)/include/spu-medialib/"

