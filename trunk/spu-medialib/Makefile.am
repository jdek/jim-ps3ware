SUBDIRS = src spu

libincludedir = $(includedir)/spu-medialib
libinclude_HEADERS =    include/spu_control.h \
			include/yuv_datastructs.h \
			include/data_2d.h \
			src/yuv2argb_scaler.h \
			src/yuv2rgb.h \
			src/yuvscaler.h \
			src/blitter.h \
			src/draw_rect.h 

lib_LTLIBRARIES = libspu-medialib.la
libspu_medialib_la_LIBADD = src/libscaler.la spu/src/*.eo.o
libspu_medialib_la_SOURCES =
libspu_medialib_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libspu-medialib.pc


EXTRA_DIST = COPYING INSTALL
