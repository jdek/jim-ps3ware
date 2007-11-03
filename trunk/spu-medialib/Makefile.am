SUBDIRS = src spu

libincludedir = $(includedir)/spu-medialib
libinclude_HEADERS =    include/spu_control.h \
			include/yuv_datastructs.h
lib_LTLIBRARIES = libspu-medialib.la
libspu_medialib_la_LIBADD = src/libscaler.la spu/src/*.eo.o
libspu_medialib_la_SOURCES =
libspu_medialib_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libspu-medialib.pc


EXTRA_DIST = COPYING INSTALL
