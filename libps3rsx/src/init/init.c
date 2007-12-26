/*
 * PS3 GPU blitting test program
 *
 * Copyright 2007 Vivien Chappelier <vivien.chappelier@free.fr>
 * Copyright 2007 Peter Popov <IronSPeter@gmail.com>
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>
#include "../../include/nouveau_class.h"
#include "../fifo/utils.h"

#include <linux/types.h>
#include <linux/fb.h>
#include <asm/ps3fb.h>



int prepare_ramin_read_line256( uint32_t *fifo, unsigned int addr)
{
  uint32_t *ptr = fifo;

  /* 2MB DDR->DDR blit */
  BEGIN_RING( 6,  0x184, 1 );
  OUT_RING( 0xfeed0000 );
  BEGIN_RING( 6, 0x198, 1 );
  OUT_RING( 0x313371c3 );
  BEGIN_RING( 3, 0x300, 1 );
  OUT_RING( 0x0000000a );
  BEGIN_RING(3, 0x30c, 1);
  OUT_RING( 0 );
  BEGIN_RING( 3, 0x304, 1 );
  OUT_RING( 0x01000100 );
  BEGIN_RING( 6, 0x2fc, 9 );
  OUT_RING( 0x00000001 );
  OUT_RING( 0x00000003 );
  OUT_RING( 0x00000003 );
  OUT_RING( 0x00000000 );
  OUT_RING( 0x00010040 ); /* 1x1 */
  OUT_RING( 0x00000000 );
  OUT_RING( 0x00010040 ); /* 1x1 */
  OUT_RING( 0x00100000 );
  OUT_RING( 0x00100000 );
  BEGIN_RING( 6, 0x400, 4 );
  OUT_RING( 0x00010040 ); /* 1x4 */
  OUT_RING( 0x00020100 ); /* pitch = 4, corner */
  OUT_RING( 1024 * 1024 * 254 + addr * 4 );
  OUT_RING( 0x00000000 );

  return ptr - fifo;
}

int prepare_ramin_vram_copy(uint32_t *fifo)
{
  uint32_t *ptr = fifo;

  /* 2MB DDR->DDR blit */
  BEGIN_RING( 6,  0x184, 1 );
  OUT_RING( 0xfeed0000 );
  BEGIN_RING( 6, 0x198, 1 );
  OUT_RING( 0x313371c3 );
  BEGIN_RING( 3, 0x300, 1 );
  OUT_RING( 0x0000000a );
  BEGIN_RING(3, 0x30c, 1);
  OUT_RING( 0 );
  BEGIN_RING( 3, 0x304, 1 );
  OUT_RING( 0x10001000 );
  BEGIN_RING( 6, 0x2fc, 9 );
  OUT_RING( 0x00000001 );
  OUT_RING( 0x00000003 );
  OUT_RING( 0x00000003 );
  OUT_RING( 0x00000000 );
  OUT_RING( 0x02000400 );
  OUT_RING( 0x00000000 );
  OUT_RING( 0x02000400 );
  OUT_RING( 0x00100000 );
  OUT_RING( 0x00100000 );
  BEGIN_RING( 6, 0x400, 4 );
  OUT_RING( 0x02000400 );
  OUT_RING( 0x00021000 );
  OUT_RING( 1024 * 1024 * 254 );
  OUT_RING( 0x00000000 );


  return ptr - fifo;
}




int prepare_ramin_write_line256(uint32_t *fifo, unsigned int addr)
{
  uint32_t *ptr = fifo;

  /* 2MB DDR->DDR blit */
  BEGIN_RING( 6,  0x184, 1 );
  OUT_RING( 0xfeed0000 );
  BEGIN_RING( 6, 0x198, 1 );
  OUT_RING( 0x313371c3 );
  BEGIN_RING( 3, 0x300, 1 );
  OUT_RING( 0x0000000a );
  BEGIN_RING(3, 0x30c, 1);
  OUT_RING( addr * 4 + 1024 * 1024 * 254 );
  BEGIN_RING( 3, 0x304, 1 );
  OUT_RING( 0x01000100 );
  BEGIN_RING( 6, 0x2fc, 9 );
  OUT_RING( 0x00000001 );
  OUT_RING( 0x00000003 );
  OUT_RING( 0x00000003 );
  OUT_RING( 0x00000000 );
  OUT_RING( 0x00010040 ); /* 1x1 */
  OUT_RING( 0x00000000 );
  OUT_RING( 0x00010040 ); /* 1x1 */
  OUT_RING( 0x00100000 );
  OUT_RING( 0x00100000 );
  BEGIN_RING( 6, 0x400, 4 );
  OUT_RING( 0x00010040 ); /* 1x4 */
  OUT_RING( 0x00020100 ); /* pitch = 4, corner */
  OUT_RING( 0x00000000 );
  OUT_RING( 0x00000000 );

  return ptr - fifo;
}


static void ramin_write_dword_to_dword_offset( struct gpu *gpu, uint32_t addr, uint32_t data )
{
  uint32_t *fifo = gpu->fifo.virt;
  uint32_t *ctrl = gpu->ctrl.virt;
  uint32_t *vram = gpu->vram.virt;
  uint32_t off = addr & 63;

{
    int wptr;
    int ret;
    wptr = (ctrl[0x10] & (gpu->fifo.len - 1)) / 4;
    ret = prepare_ramin_read_line256(&fifo[wptr], addr - off );
    fifo_push(gpu, ret);
    fifo_wait(gpu);

  }
  //wait...
  usleep( 10000 );
  //patch with data
  vram[off] = endian( data );
  //wait...
  usleep( 10000 );
  //copy data back
  {
    int wptr;
    int ret;

    wptr = (ctrl[0x10] & (gpu->fifo.len - 1)) / 4;

    ret = prepare_ramin_write_line256(&fifo[wptr], addr - off );
    fifo_push(gpu, ret);
    fifo_wait(gpu);

  }


}

#define RAMIN_BASE 0x180000
#define RAMHT_BASE 0x190000

static void create_gfx_object(struct gpu *gpu,
			      uint32_t klass, uint32_t handle, uint32_t offset)
{
  uint32_t hash;

  hash = hash_handle(gpu->hw_id, handle);

  ramin_write_dword_to_dword_offset( gpu,
				     (RAMHT_BASE + hash) / 4, handle );
  //engine zero, offset
  ramin_write_dword_to_dword_offset( gpu,
				     (RAMHT_BASE + hash + 4) / 4,
				     (gpu->hw_id << 23) |
				     (1 << 20)  |
				     (offset >> 4));

  ramin_write_dword_to_dword_offset( gpu, (RAMIN_BASE + offset) / 4 + 0, klass );
  ramin_write_dword_to_dword_offset( gpu, (RAMIN_BASE + offset) / 4 + 1, 0x00000000 );
  //endianness
  ramin_write_dword_to_dword_offset( gpu, (RAMIN_BASE + offset) / 4 + 2, 0x01000000 );
  ramin_write_dword_to_dword_offset( gpu, (RAMIN_BASE + offset) / 4 + 3, 0x00000000 );
  ramin_write_dword_to_dword_offset( gpu, (RAMIN_BASE + offset) / 4 + 4, 0x00000000 );
  ramin_write_dword_to_dword_offset( gpu, (RAMIN_BASE + offset) / 4 + 5, 0x00000000 );
  ramin_write_dword_to_dword_offset( gpu, (RAMIN_BASE + offset) / 4 + 6, 0x00000000 );
  ramin_write_dword_to_dword_offset( gpu, (RAMIN_BASE + offset) / 4 + 7, 0x00000000 );
}

struct gpu gpu;

void sigint_handler(int sig)
{
  (void) sig;

  gpu_cleanup(&gpu);
}

uint32_t RAMIN1[1024 * 1024 / 2];

int main(void)
{

  if (gpu_init(&gpu) < 0)
  {
    fprintf(stderr, "Failed to initialize GPU\n");
    return -1;
  }

  signal(SIGINT, sigint_handler);

  //0x40 for NV40, 0x97 - 3D engine
  create_gfx_object(&gpu, 0x00004097, 0xfeed0007, 0x40200);

  printf( "3d engine setup completed \n" );
  gpu_cleanup(&gpu);

  return 0;
}
