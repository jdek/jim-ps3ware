SUBDIRS = src spu

libincludedir = $(includedir)/spu-medialib
libinclude_HEADERS =    include/spu-medialib/spu_control.h \
			include/spu-medialib/yuv_datastructs.h \
			include/spu-medialib/data_2d.h \
			include/spu-medialib/spu_medialib_types.h \
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
