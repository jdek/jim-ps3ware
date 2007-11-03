bin_PROGRAMS = spu_yuv2argb_scaler \
	       spu_yuv2rgb \
	       spu_yuvscaler
noinst_DATA = spu_yuv2argb_scaler.eo \
             spu_yuv2rgb.eo \
             spu_yuvscaler.eo

%.eo: %
	$(EMBEDSPU) $<_handle $< $@
# ugly workaround to make libtool happy
	cp $@ $@.o

INCLUDES = -I$(top_srcdir)/../include -I$(top_srcdir)/include
