PPU_CC=powerpc64-unknown-linux-gnu-gcc
PPU_INC=
PPU_LD=-lpthread -lspe2 
PPU_EXEC=loader

SPU_CC=spu-elf-gcc
SPU_CFLAGS_DEF= -g -fno-exceptions -Wall -O2
SPU_LD=spu-elf-ld
SPU_EXEC1=spe_loader
SPU_EXEC2=spe_loadme
SHARED_C=shared_functions.c

all: ppu spu

ppu:
	${PPU_CC} ${PPU_INC} ${PPU_EXEC}.c -o ${PPU_EXEC} ${PPU_LD} 
	
spu:
	${SPU_CC} ${SPU_CFLAGS_DEF} ${SPU_EXEC1}.c ${SHARED_C} -o ${SPU_EXEC1}
	${SPU_CC} -fpic -Wl,-mrelocatable -c ${SPU_EXEC2}.c -o ${SPU_EXEC2}.o
	${SPU_LD} -e 0x80  ${SPU_EXEC2}.o -o ${SPU_EXEC2}.elf
	spu-elf-objcopy -O binary -S ${SPU_EXEC2}.elf ${SPU_EXEC2}

