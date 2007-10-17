include config.mak

all: spu src lib
	
no-spu: src lib

deepclean: clean clean_spu

spu: force_look
	cd $(SPUDIR); $(MAKE)

src: force_look
	cd $(SRCDIR); $(MAKE)

lib: force_look
	cd $(LIBDIR); $(MAKE)

clean_spu: 
	cd $(SPUDIR); $(MAKE) clean 

clean:
	cd $(SRCDIR); $(MAKE) clean
	cd $(LIBDIR); $(MAKE) clean

install: install-lib install-inc

install-lib:
	install -d "$(PREFIX)/lib"
	install -m 644 $(LIBDIR)/$(LIB) "$(PREFIX)/lib"

install-inc:
	install -d "$(PREFIX)/include/spu-medialib"
	install -m 644 shared/include/spu_control.h "$(PREFIX)/include/spu-medialib"
	for a in $(PROG); do install -m 644 $(SRCDIR)/$${a}.h "$(PREFIX)/include/spu-medialib";done
	
uninstall:
	rm -f $(addprefix $(PREFIX)/lib/, $(notdir $(INS_LIB)))
	rm -f $(addprefix $(PREFIX)/include/spu-medialib/, $(notdir spu_control.h))
	for a in $(PROG); do rm -f $(addprefix $(PREFIX)/include/spu-medialib/, $(notdir $${a}.h));done
	-rmdir "$(PREFIX)/include/spu-medialib/"


force_look :
	true
