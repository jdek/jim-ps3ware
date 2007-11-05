bin_PROGRAMS = spu_yuv2argb_scaler \
	       spu_yuv2rgb \
	       spu_yuvscaler \
               spu_blit_yuv422 \
               spu_blit_yuv420 \
               spu_blit_yuv420_scale

noinst_DATA = spu_yuv2argb_scaler.eo \
             spu_yuv2rgb.eo \
             spu_yuvscaler.eo \
             spu_blit_yuv422.eo \
             spu_blit_yuv420.eo \
             spu_blit_yuv420_scale.eo


%.eo: %
	$(EMBEDSPU) $<_handle $< $@
# ugly workaround to make libtool happy
	cp $@ $@.o

INCLUDES = -I$(top_srcdir)/../include -I$(top_srcdir)/include
