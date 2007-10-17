include ../../config.mak

all: spu

spu:
	for a in $(SPU_PROG); do $(SPU_CC) $(SPU_CFLAGS) $${a}.c -o $$a $(SPU_INC);done

clean:
	for a in $(SPU_PROG); do rm -rf  $${a};done
