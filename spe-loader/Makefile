PPU_CC=gcc
PPU_INC=
PPU_LD=-lpthread -lspe2 
PPU_EXEC=loader

SPU_CC=spu-elf-gcc
SPU_EXEC1=spe_loader
SPU_EXEC2=spe_loadme

all: ppu spu

ppu:
	${CC} ${PPU_INC} ${PPU_EXEC}.c -o ${PPU_EXEC} ${PPU_LD} 
	
spu:
	${SPU_CC} ${SPU_EXEC1}.c -o ${SPU_EXEC1}
	${SPU_CC} -c ${SPU_EXEC2}.c 
	${SPU_CC} ${SPU_EXEC2}.o -o ${SPU_EXEC2}