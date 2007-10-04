PPU_CC=gcc
SPU_CC=spu-elf-gcc
PPU_CFLAGS=-fno-exceptions -I../libfb -Iyuv2rgb -Iyuvscaler -Ispu_utils -I../../libfb/asm -O2 -Wall
PPU_CFLAGS64= -m64 $(PPU_CFLAGS)
SPU_CFLAGS=-std=c99 -O2 -fno-exceptions -g -Ispu_utils -Wall

SPU_BIN0=spu_yuv2rgb
SPU_BIN1=spu_yuvscaler
SPU_BIN2=spu_yuv2argb_scaler

SPU_HANDLE0=spu_yuvscaler_handle
SPU_HANDLE1=spu_yuv2rgb_handle
SPU_HANDLE2=spu_yuv2argb_scaler_handle

SPU_CSF0=$(SPU_BIN0)_csf.o
SPU_CSF1=$(SPU_BIN1)_csf.o
SPU_CSF2=$(SPU_BIN2)_csf.o

PPU_LDFLAGS=yuvscaler/yuvscaler.c yuv2rgb/yuv2rgb.c -lspe2 
SPU_LDFLAGS=
PREFIX=/usr
INS_LIB=libspu-medialib.a 
INS_INC=spu_utils/spu_control.h yuv2rgb/yuv2rgb.h yuvscaler/yuvscaler.h yuv420scaler2argb/yuv2argb_scaler.h

all: spu embed lib

spu:
	$(SPU_CC) $(SPU_CFLAGS) yuv2rgb/spu_yuv2rgb.c -o $(SPU_BIN0) $(SPU_LDFLAGS)
	$(SPU_CC) $(SPU_CFLAGS) yuvscaler/spu_yuvscaler.c -o $(SPU_BIN1) $(SPU_LDFLAGS)
	$(SPU_CC) $(SPU_CFLAGS) yuv420scaler2argb/spu_yuv2argb_scaler.c -o $(SPU_BIN2) $(SPU_LDFLAGS)

embed:
	embedspu $(SPU_HANDLE0) $(SPU_BIN0) $(SPU_CSF0)
	embedspu $(SPU_HANDLE1) $(SPU_BIN1) $(SPU_CSF1)
	embedspu $(SPU_HANDLE2) $(SPU_BIN2) $(SPU_CSF2)

lib:
	$(PPU_CC) $(PPU_CFLAGS64) -c yuvscaler/yuvscaler.c -o yuvscaler.o
	$(PPU_CC) $(PPU_CFLAGS64) -c yuv2rgb/yuv2rgb.c -o yuv2rgb.o
	$(PPU_CC) $(PPU_CFLAGS64) -c yuv420scaler2argb/yuv2argb_scaler.c -o yuv2argb_scaler.o
	
	ar rcs libspu-medialib.a yuvscaler.o yuv2rgb.o yuv2argb_scaler.o $(SPU_CSF0) $(SPU_CSF1) $(SPU_CSF2)

clean:
	rm -rf *.o scaleandconvert spu_yuv2rgb spu_yuvscaler spu_yuv2argb_scaler libspu-medialib.a

install: install-lib install-inc

install-lib: lib
	install -d "$(PREFIX)/lib64"
	install -m 644 $(INS_LIB) "$(PREFIX)/lib64"

install-inc:
	install -d "$(PREFIX)/include/spu-medialib"
	install -m 644 $(INS_INC) "$(PREFIX)/include/spu-medialib"

uninstall:
	rm -f $(addprefix $(PREFIX)/lib64/, $(notdir $(INS_LIB)))
	rm -f $(addprefix $(PREFIX)/include/spu-medialib/, $(notdir $(INS_INC)))
	-rmdir "$(PREFIX)/include/spu-medialib/"

