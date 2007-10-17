include ../config.mak

all: spu binmove embed

spu: force_look
	cd src; $(MAKE)
binmove: force_look
	for a in $(SPU_PROG);do cp src/$$a bin/;done
embed:
	for e in $(SPU_PROG);do $(EMBED) $${e}_handle bin/$$e cesof/$${e}_csf.o ;done

clean:
	cd src; $(MAKE) clean
	rm -rf bin/*
	rm -rf cesof/*
	

force_look :
	true
