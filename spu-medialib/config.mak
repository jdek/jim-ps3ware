SPU_CC=spu-elf-gcc

CC=gcc

SPU_CFLAGS= -O3 -fno-exceptions -g -Wall

CFLAGS= -O3 -fno-exceptions -fPIC -g -Wall

PPU_CC=gcc

EMBED=embedspu

SPU_INC=-I../include -I../../shared/include

INC=-I../shared/include -I../include

PROG=yuv2argb_scaler yuvscaler yuv2rgb

SPU_PROG=spu_yuv2argb_scaler spu_yuvscaler spu_yuv2rgb

SPU_BIN=spu/bin/

SPU_SRC=spu/src/

SPUDIR=spu

SRCDIR=src

SPU_CESOF=spu/cesof/

LIBDIR=lib

LIB=libspu-medialib.a

LIBSO=libspu-medialib.so

PREFIX=/usr
