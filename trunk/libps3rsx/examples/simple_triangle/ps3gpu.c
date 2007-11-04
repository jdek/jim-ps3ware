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
#include <math.h>
//#include "nouveau_reg.h"
//#include "nv30_fpinst.h"
//#include "nv40_vpinst.h"
#include "nouveau_class.h"
#include "nv_shaders.h"



#include <linux/types.h>
#include <linux/fb.h>
#include <asm/ps3fb.h>

#define dbg(...)

#define DEV_MEM		"/dev/mem"
#define DEV_VFB		"/dev/fb0"
#define DEV_GPU_VRAM	"/dev/ps3gpu_vram"
#define DEV_GPU_FIFO	"/dev/ps3gpu_fifo"
#define DEV_GPU_CTRL	"/dev/ps3gpu_ctrl"

#define barrier() asm volatile ("" : : : "memory");



static unsigned int endian( unsigned int v )
{
  return ( ( ( v >> 24 ) & 0xff ) << 0 ) |
         ( ( ( v >> 16 ) & 0xff ) << 8 ) |
         ( ( ( v >> 8 ) & 0xff ) << 16 ) |
         ( ( ( v >> 0 ) & 0xff ) << 24 );
}

static unsigned int endian_fp( unsigned int v )
{
  return ( ( ( v >> 16 ) & 0xffff ) << 0 ) |
         ( ( ( v >> 0 ) & 0xffff ) << 16 );

}

struct resource
{
  void *virt;
  size_t len;
};

struct gpu
{
  struct resource xram;
  struct resource vram;
  struct resource fifo;
  struct resource ctrl;
};


static int gpu_get_info(struct gpu *gpu)
{
  struct ps3fb_ioctl_gpu_info info;
  int ret = -1;
  int fd;


  if ((fd = open(DEV_VFB, O_RDWR)) < 0)
  {
    perror("open");
    return -1;
  }

  if ((ret = ioctl(fd, PS3FB_IOCTL_GPU_INFO, &info)) < 0)
  {
    perror("ioctl");
    goto out;
  }

  printf("vram %d fifo %d ctrl %d\n",
         info.vram_size, info.fifo_size, info.ctrl_size);

  gpu->xram.len = 16 * 1024 * 1024;
  gpu->vram.len = info.vram_size;
  gpu->fifo.len = info.fifo_size;
  gpu->ctrl.len = info.ctrl_size;

  ret = 0;
out:
  close(fd);

  return ret;
}

int map_resource(char const *name, struct resource *res)
{
  void *virt;
  int fd;

  if ((fd = open(name, O_RDWR)) < 0)
  {
    perror("open");
    return -1;
  }

  // TEMP
  printf("mmap: %s len %d\n", name, res->len);

  virt = mmap(0, res->len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (virt == MAP_FAILED)
    return -1;

  res->virt = virt;

  close(fd);

  return 0;
}

int unmap_resource(struct resource *res)
{
  munmap(res->virt, res->len);
  res->virt = NULL;

  return 0;
}

int map_gpu(struct gpu *gpu)
{

  
  if (map_resource(DEV_VFB, &gpu->xram) < 0)
  {
    fprintf(stderr, "failed to map vram\n");
    return -1;
  }

  if (map_resource(DEV_GPU_VRAM, &gpu->vram) < 0)
  {
    fprintf(stderr, "failed to map vram\n");
    return -1;
  }

  if (map_resource(DEV_GPU_FIFO, &gpu->fifo) < 0)
  {
    fprintf(stderr, "failed to map fifo\n");
    goto err_unmap_vram;
  }

  if (map_resource(DEV_GPU_CTRL, &gpu->ctrl) < 0)
  {
    fprintf(stderr, "failed to map ctrl\n");
    goto err_unmap_fifo;
  }

  return 0;

err_unmap_fifo:
  unmap_resource(&gpu->fifo);
err_unmap_vram:
  unmap_resource(&gpu->vram);
  return -1;
}

int unmap_gpu(struct gpu *gpu)
{
  unmap_resource(&gpu->fifo);

  return 0;
}

#define OUT_RING(data) *(ptr)++ = (data)


union f2i
{
  float f;
  uint32_t i;
};

#define OUT_RINGf(data) { union f2i temp; temp.f = data;  *(ptr)++ = temp.i; }

#define BEGIN_RING(chan, tag, size)   OUT_RING(((size) << 18) | ((chan) << 13) | (tag))



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

#define Nv3D 7
int NV40_LoadVtxProg( uint32_t *fifo,  nv_vshader_t *shader)
{
  uint32_t *ptr = fifo;
  uint32_t i;


  BEGIN_RING(Nv3D, NV40TCL_VP_UPLOAD_FROM_ID, 1);
  OUT_RING  ( 0 );
  for (i=0; i<shader->size; i+=4)
  {
    BEGIN_RING(Nv3D, NV40TCL_VP_UPLOAD_INST(0), 4);
    OUT_RING  (shader->data[i + 0]);
    OUT_RING  (shader->data[i + 1]);
    OUT_RING  (shader->data[i + 2]);
    OUT_RING  (shader->data[i + 3]);

  }


  BEGIN_RING(Nv3D, NV40TCL_VP_START_FROM_ID, 1);
  OUT_RING  (0);

  BEGIN_RING(Nv3D, NV40TCL_VP_ATTRIB_EN, 2);
  OUT_RING  (shader->vp_in_reg);
  OUT_RING  (shader->vp_out_reg);

  BEGIN_RING( Nv3D, 0x1478, 1 );
  OUT_RING  (0);



  return ptr - fifo;
}




uint32_t width = 1024;
uint32_t height = 768;
uint32_t pitch = 1280 * 4;

uint32_t fp_offset = 60 * 1024 * 1024;
uint32_t vb_offset = 61 * 1024 * 1024;
uint32_t tx_offset = 62 * 1024 * 1024;



int NV40_LoadTex( uint32_t *fifo, uint8_t *fbmem )
{
  uint32_t *ptr = fifo;
  uint32_t i;
  uint32_t unit = 0;
  uint32_t offset = tx_offset;

  uint32_t width = 128, height = 128;
  for( i = 0; i < width * height * 4; i += 4 )
  {
    fbmem[i + offset + 0] = 255;
    fbmem[i + offset + 1] = 200;
    fbmem[i + offset + 2] = 100;
    fbmem[i + offset + 3] = rand();

  }

  uint32_t swz =
    NV40TCL_TEX_SWIZZLE_S0_X_S1 | NV40TCL_TEX_SWIZZLE_S0_Y_S1 |
    NV40TCL_TEX_SWIZZLE_S0_Z_S1 | NV40TCL_TEX_SWIZZLE_S0_W_S1 |
    NV40TCL_TEX_SWIZZLE_S1_X_X | NV40TCL_TEX_SWIZZLE_S1_Y_Y |
    NV40TCL_TEX_SWIZZLE_S1_Z_Z | NV40TCL_TEX_SWIZZLE_S1_W_W;



  BEGIN_RING(Nv3D, NV40TCL_TEX_OFFSET(unit), 8);
  OUT_RING  ( offset );

  OUT_RING  (
    NV40TCL_TEX_FORMAT_FORMAT_A8R8G8B8 |
    NV40TCL_TEX_FORMAT_LINEAR  | 
    NV40TCL_TEX_FORMAT_DIMS_2D |
    NV40TCL_TEX_FORMAT_DMA0 |
    NV40TCL_TEX_FORMAT_NO_BORDER | (0x8000) |
    (1 << NV40TCL_TEX_FORMAT_MIPMAP_COUNT_SHIFT));

  OUT_RING  (
    NV40TCL_TEX_WRAP_S_REPEAT |
    NV40TCL_TEX_WRAP_T_REPEAT |
    NV40TCL_TEX_WRAP_R_REPEAT);

  OUT_RING  (NV40TCL_TEX_ENABLE_ENABLE);
  OUT_RING  (swz);

  OUT_RING  (
    NV40TCL_TEX_FILTER_MIN_LINEAR |
    NV40TCL_TEX_FILTER_MAG_LINEAR | 0x3fd6);
  OUT_RING  ((width << 16) | height);
  OUT_RING  (0); /* border ARGB */
  BEGIN_RING(Nv3D, NV40TCL_TEX_SIZE1(unit), 1);

  OUT_RING  (
    (1 << NV40TCL_TEX_SIZE1_DEPTH_SHIFT) |
    width * 4 );

  return ptr - fifo;

}

int NV40_LoadFragProg( uint32_t *fifo, uint32_t *fbmem, nv_pshader_t *shader)
{
  uint32_t i;
  uint32_t offset = fp_offset / 4;
  uint32_t *ptr = fifo;
  static int next_hw_id_offset = 0;

  if (!shader->hw_id)
  {

    for( i = 0; i < shader->size; ++i )
    {
      fbmem[ offset + next_hw_id_offset + i] = endian_fp( shader->data[i] );
    }


    shader->hw_id  = offset;
    shader->hw_id += next_hw_id_offset;
    shader->hw_id *= 4;

    next_hw_id_offset += shader->size;
    next_hw_id_offset = (next_hw_id_offset + 63) & ~63;
  }

  printf( "frag prog 0x%x \n", shader->hw_id );
  BEGIN_RING(Nv3D, NV40TCL_FP_ADDRESS, 1);
  OUT_RING  ( shader->hw_id | NV40TCL_FP_ADDRESS_DMA0);
  BEGIN_RING(Nv3D, NV40TCL_FP_CONTROL, 1);
  OUT_RING  ( ( shader->num_regs << NV40TCL_FP_CONTROL_TEMP_COUNT_SHIFT ) );


  return ptr - fifo;
}




#define CV_OUT0( sx,sy, sz, tx, ty) do { data[0] = sx;data[1] = sy;data[2] = sz;data[3] = 1.0f;data[4] = tx;data[5] = ty;data += 6;} while(0)

int NV40_EmitBufferGeometry( uint32_t *fifo, uint8_t *mem )
{

  uint32_t *ptr = fifo;
  uint32_t i;

  uint32_t offset = vb_offset;
  uint32_t xdrmem = 0xfeed0000;
  uint32_t stride = 24;
  //uint32_t pos = 0;
  //uint32_t tx0 = 8;
  

  BEGIN_RING(Nv3D, 0x1dac, 1 );
  OUT_RING( 0 );
  

  
  BEGIN_RING(Nv3D,NV40TCL_DMA_VTXBUF0, 2 );
  OUT_RING  ( xdrmem );
  OUT_RING  ( xdrmem );
  
  //BEGIN_RING(Nv3D, 0x1740, 1 );
  //OUT_RING( 0 );
  
  
  BEGIN_RING(Nv3D, NV40TCL_VTXFMT( 0 ), 9 );
  OUT_RING  ( NV40TCL_VTXFMT_TYPE_FLOAT | ( 4 << NV40TCL_VTXFMT_SIZE_SHIFT ) | ( stride << NV40TCL_VTXFMT_STRIDE_SHIFT )  );
  OUT_RING  ( NV40TCL_VTXFMT_TYPE_FLOAT );
  OUT_RING  ( NV40TCL_VTXFMT_TYPE_FLOAT );
  OUT_RING  ( NV40TCL_VTXFMT_TYPE_FLOAT );
  OUT_RING  ( NV40TCL_VTXFMT_TYPE_FLOAT );
  OUT_RING  ( NV40TCL_VTXFMT_TYPE_FLOAT );
  OUT_RING  ( NV40TCL_VTXFMT_TYPE_FLOAT );
  OUT_RING  ( NV40TCL_VTXFMT_TYPE_FLOAT );
  OUT_RING  ( NV40TCL_VTXFMT_TYPE_FLOAT | ( 2 << NV40TCL_VTXFMT_SIZE_SHIFT ) | ( stride << NV40TCL_VTXFMT_STRIDE_SHIFT )  );
  
  BEGIN_RING(Nv3D, 0x1718, 1);
  OUT_RING  (0);
  BEGIN_RING(Nv3D, 0x1718, 1);
  OUT_RING  (0);
  BEGIN_RING(Nv3D, 0x1718, 1);
  OUT_RING  (0);

  
 
  BEGIN_RING(Nv3D, NV40TCL_VTXBUF_ADDRESS( 0 ), 9 );
  OUT_RING  ( offset | ( 0 << 31 ) );
  OUT_RING  ( offset | ( 0 << 31 ) );
  OUT_RING  ( offset | ( 0 << 31 ) );
  OUT_RING  ( offset | ( 0 << 31 ) );
  OUT_RING  ( offset | ( 0 << 31 ) );
  OUT_RING  ( offset | ( 0 << 31 ) );
  OUT_RING  ( offset | ( 0 << 31 ) );
  OUT_RING  ( offset | ( 0 << 31 ) );
  OUT_RING  ( ( offset + 16 ) | ( 0 << 31 ) );
  
  
  float *data = (float *)( mem + offset );

  uint32_t vnum = 9;
  float pi = atan( 1.0f ) * 4.0f;
  for( i = 0; i < 3; ++i )
  {
    float si = sin( i * pi / 1.5f ) * 2.0f;
    float co = cos( i * pi / 1.5f ) * 2.0f;

    float x1 = 200.0f, y1 = 80.0f;
    float x2 = -200.0f, y2 = 10.0f;
    float x3 = -200.0f, y3 = 150.0f;

    float cx = 1280.0f / 2.0f;
    float cy = 1024.0f / 2.0f;
    
    CV_OUT0( cx + x1 * co + y1 * si, cy - x1 * si + y1 * co, 1.0f, 0.5f, 0.0f  );
    CV_OUT0( cx + x2 * co + y2 * si, cy - x2 * si + y2 * co, 0.0f, 0.0f, 1.0f  );
    CV_OUT0( cx + x3 * co + y3 * si, cy - x3 * si + y3 * co, 0.0f, 1.0f, 1.0f  );

  }

  
  
  BEGIN_RING(Nv3D, NV40TCL_BEGIN_END, 1);
  OUT_RING  (NV40TCL_BEGIN_END_TRIANGLES);

  BEGIN_RING(Nv3D, NV40TCL_VB_VERTEX_BATCH, 1 );
  OUT_RING( ( 0 << NV40TCL_VB_VERTEX_BATCH_START_SHIFT ) | ( vnum << NV40TCL_VB_VERTEX_BATCH_COUNT_SHIFT ) );
  
  BEGIN_RING(Nv3D, NV40TCL_BEGIN_END, 1);
  OUT_RING  (NV40TCL_BEGIN_END_STOP);
  


  return ptr - fifo;

}


#define CV_OUT1( sx,sy, sz, tx, ty) do {                                                     \
	BEGIN_RING(Nv3D, NV40TCL_VTX_ATTR_4F_X(0), 4);                         \
	OUT_RINGf ((sx)); OUT_RINGf ((sy));                                    \
	OUT_RINGf ((sz)); OUT_RINGf ((1.0f));                                    \
	BEGIN_RING(Nv3D, NV40TCL_VTX_ATTR_2F_X(8), 2);                         \
	OUT_RINGf ((tx)); OUT_RINGf ((ty));                                    \
} while(0)

int NV40_EmitGeometry( uint32_t *fifo )
{
  uint32_t *ptr = fifo;
  uint32_t i;
  
  BEGIN_RING(Nv3D, 0x1718, 1);
  OUT_RING  (0);
  BEGIN_RING(Nv3D, 0x1718, 1);
  OUT_RING  (0);
  BEGIN_RING(Nv3D, 0x1718, 1);
  OUT_RING  (0);

  
  BEGIN_RING(Nv3D, NV40TCL_BEGIN_END, 1);
  OUT_RING  (NV40TCL_BEGIN_END_TRIANGLES);

  float pi = atan( 1.0f ) * 4.0f;

  for( i = 0; i < 3; ++i )
  {
    float si = sin( i * pi / 1.5f );
    float co = cos( i * pi / 1.5f );

    float x1 = 200.0f, y1 = 80.0f;
    float x2 = -200.0f, y2 = 10.0f;
    float x3 = -200.0f, y3 = 150.0f;


    CV_OUT1( 256.0f + x1 * co + y1 * si, 256.0f - x1 * si + y1 * co, 1.0f, 0.5f, 0.0f  );
    CV_OUT1( 256.0f + x2 * co + y2 * si, 256.0f - x2 * si + y2 * co, 0.0f, 0.0f, 1.0f  );
    CV_OUT1( 256.0f + x3 * co + y3 * si, 256.0f - x3 * si + y3 * co, 0.0f, 1.0f, 1.0f  );

  }
  
  
 

  BEGIN_RING(Nv3D, NV40TCL_BEGIN_END, 1);
  OUT_RING  (NV40TCL_BEGIN_END_STOP);

  return ptr - fifo;
}



int bind3d(  uint32_t *fifo, uint32_t *fbmem, uint8_t *xdrmem, uint32_t obj )
{

  int i;
  uint32_t *ptr = fifo;
  uint32_t NvDmaNotifier0 = 0x66604200;
  uint32_t NvDmaFB = 0xfeed0000;
  xdrmem = xdrmem;

  BEGIN_RING(Nv3D, 0, 1);
  OUT_RING  (obj);

  BEGIN_RING(Nv3D, NV40TCL_DMA_NOTIFY, 1);
  OUT_RING  (NvDmaNotifier0);
  BEGIN_RING(Nv3D, NV40TCL_DMA_TEXTURE0, 1);
  OUT_RING  (NvDmaFB);

  BEGIN_RING(Nv3D, NV40TCL_DMA_COLOR0, 2);
  OUT_RING  (NvDmaFB);
  OUT_RING  (NvDmaFB);

  BEGIN_RING(Nv3D, NV40TCL_DMA_ZETA, 1 );
  OUT_RING  (NvDmaFB);


  /* voodoo */
  BEGIN_RING(Nv3D, 0x1ea4, 3);
  OUT_RING  (0x00000010);
  OUT_RING  (0x01000100);
  OUT_RING  (0xff800006);
  BEGIN_RING(Nv3D, 0x1fc4, 1);
  OUT_RING  (0x06144321);
  BEGIN_RING(Nv3D, 0x1fc8, 2);
  OUT_RING  (0xedcba987);
  OUT_RING  (0x00000021);
  BEGIN_RING(Nv3D, 0x1fd0, 1);
  OUT_RING  (0x00171615);
  BEGIN_RING(Nv3D, 0x1fd4, 1);
  OUT_RING  (0x001b1a19);
  BEGIN_RING(Nv3D, 0x1ef8, 1);
  OUT_RING  (0x0020ffff);
  BEGIN_RING(Nv3D, 0x1d64, 1);
  OUT_RING  (0x00d30000);
  BEGIN_RING(Nv3D, 0x1e94, 1);
  OUT_RING  (0x00000001);

  BEGIN_RING(Nv3D, NV40TCL_VIEWPORT_TRANSLATE_X, 8);
  OUT_RINGf (0.0);
  OUT_RINGf (0.0);
  OUT_RINGf (0.0);
  OUT_RINGf (0.0);
  OUT_RINGf (1.0);
  OUT_RINGf (1.0);
  OUT_RINGf (1.0);
  OUT_RINGf (0.0);

  /* default 3D state */
  BEGIN_RING(Nv3D, NV40TCL_STENCIL_FRONT_ENABLE, 1);
  OUT_RING  (0);
  BEGIN_RING(Nv3D, NV40TCL_STENCIL_BACK_ENABLE, 1);
  OUT_RING  (0);
  BEGIN_RING(Nv3D, NV40TCL_ALPHA_TEST_ENABLE, 1);
  OUT_RING  (0);
  BEGIN_RING(Nv3D, NV40TCL_DEPTH_WRITE_ENABLE, 1);
  OUT_RING  (1);
  BEGIN_RING(Nv3D, NV40TCL_DEPTH_TEST_ENABLE, 1);
  OUT_RING  (1);
  BEGIN_RING(Nv3D, NV40TCL_DEPTH_FUNC, 1);
  OUT_RING  (NV40TCL_DEPTH_FUNC_LESS);
  BEGIN_RING(Nv3D, NV40TCL_COLOR_MASK, 1);
  OUT_RING  (0x01010101); /* TR,TR,TR,TR */
  BEGIN_RING(Nv3D, NV40TCL_CULL_FACE_ENABLE, 1);
  OUT_RING  (0);
  BEGIN_RING(Nv3D, NV40TCL_BLEND_ENABLE, 1);
  OUT_RING  (0);
  BEGIN_RING(Nv3D, NV40TCL_COLOR_LOGIC_OP_ENABLE, 2);
  OUT_RING  (0);
  OUT_RING  (NV40TCL_COLOR_LOGIC_OP_COPY);
  BEGIN_RING(Nv3D, NV40TCL_DITHER_ENABLE, 1);
  OUT_RING  (0);
  BEGIN_RING(Nv3D, NV40TCL_SHADE_MODEL, 1);
  OUT_RING  (NV40TCL_SHADE_MODEL_SMOOTH);
  BEGIN_RING(Nv3D, NV40TCL_POLYGON_OFFSET_FACTOR,2);
  OUT_RINGf (0.0);
  OUT_RINGf (0.0);
  BEGIN_RING(Nv3D, NV40TCL_POLYGON_MODE_FRONT, 2);
  OUT_RING  (NV40TCL_POLYGON_MODE_FRONT_FILL);
  OUT_RING  (NV40TCL_POLYGON_MODE_BACK_FILL);
  BEGIN_RING(Nv3D, NV40TCL_POLYGON_STIPPLE_PATTERN(0), 0x20);
  for (i=0;i<0x20;i++)
    OUT_RING  (0xFFFFFFFF);
  for (i=0;i<16;i++)
  {
    BEGIN_RING(Nv3D, NV40TCL_TEX_ENABLE(i), 1);
    OUT_RING  (0);
  }

  BEGIN_RING(Nv3D, 0x1d78, 1);
  OUT_RING  (0x110);

  BEGIN_RING(Nv3D, NV40TCL_RT_ENABLE, 1);
  OUT_RING  (NV40TCL_RT_ENABLE_COLOR0);

  BEGIN_RING(Nv3D, NV40TCL_RT_HORIZ, 2);
  OUT_RING  ((1280 << 16));
  OUT_RING  ((1024 << 16));
  BEGIN_RING(Nv3D, NV40TCL_SCISSOR_HORIZ, 2);
  OUT_RING  ((1280 << 16));
  OUT_RING  ((1024 << 16));
  BEGIN_RING(Nv3D, NV40TCL_VIEWPORT_HORIZ, 2);
  OUT_RING  ((1280 << 16));
  OUT_RING  ((1024 << 16));
  BEGIN_RING(Nv3D, NV40TCL_VIEWPORT_CLIP_HORIZ(0), 2);
  OUT_RING  ((1280 << 16));
  OUT_RING  ((1024 << 16));


  BEGIN_RING(Nv3D, NV40TCL_ZETA_OFFSET, 1 );
  OUT_RING( 1280 * 1024 * 4 );
  BEGIN_RING(Nv3D, NV40TCL_ZETA_PITCH, 1 );
  OUT_RING( pitch );



  BEGIN_RING(Nv3D, NV40TCL_RT_FORMAT, 3);
  OUT_RING  (NV40TCL_RT_FORMAT_TYPE_LINEAR |
             NV40TCL_RT_FORMAT_ZETA_Z16 |
             NV40TCL_RT_FORMAT_COLOR_A8R8G8B8);
  OUT_RING  (pitch);
  OUT_RING  (0);
  BEGIN_RING(Nv3D, NV40TCL_CLEAR_VALUE_COLOR, 1 );
  OUT_RING( 0x0 );

  BEGIN_RING(Nv3D, NV40TCL_CLEAR_VALUE_DEPTH, 1 );
  OUT_RING( 0xffff );


  BEGIN_RING(Nv3D,NV40TCL_CLEAR_BUFFERS,1 );
  OUT_RING
  (
    NV40TCL_CLEAR_BUFFERS_COLOR_B |
    NV40TCL_CLEAR_BUFFERS_COLOR_G |
    NV40TCL_CLEAR_BUFFERS_COLOR_R |
    NV40TCL_CLEAR_BUFFERS_COLOR_A |
    NV40TCL_CLEAR_BUFFERS_STENCIL |
    NV40TCL_CLEAR_BUFFERS_DEPTH
  );

  ptr += NV40_LoadTex( ptr, (uint8_t *)fbmem );
  ptr += NV40_LoadVtxProg( ptr,  &nv40_vp );
  ptr += NV40_LoadFragProg( ptr, fbmem,  &nv30_fp );
  //ptr += NV40_EmitGeometry( ptr );
  ptr += NV40_EmitBufferGeometry( ptr, (uint8_t *)fbmem );


  return ptr - fifo;
}



static void fifo_push(struct gpu *gpu, int len)
{
  uint32_t *ctrl = gpu->ctrl.virt;

  ctrl[0x10] += 4 * len;
}

static void fifo_wait(struct gpu *gpu)
{
  volatile uint32_t *ctrl = gpu->ctrl.virt;

  while (ctrl[0x10] != ctrl[0x11]);
}


static void gfx_test(struct gpu *gpu, unsigned int obj )
{
  uint32_t *fifo = gpu->fifo.virt;
  uint32_t *ctrl = gpu->ctrl.virt;
  uint32_t *vram = gpu->vram.virt;
  uint8_t *xram = gpu->xram.virt;

  int wptr;
  int ret;

  wptr = (ctrl[0x10] & (gpu->fifo.len - 1)) / 4;

  ret = bind3d( &fifo[wptr], vram, xram, obj );
  fifo_push(gpu, ret);
  fifo_wait(gpu);


}

/*
static void memcpy_from_ramin(struct gpu *gpu, void *data, int len)
{
  uint32_t *fifo = gpu->fifo.virt;
  uint32_t *ctrl = gpu->ctrl.virt;
  int wptr;
  int ret;
  int i;
 
  wptr = (ctrl[0x10] & (gpu->fifo.len - 1)) / 4;
 
  ret = prepare_ramin_vram_copy(&fifo[wptr]);
  fifo_push(gpu, ret);
  fifo_wait(gpu);
 
  memcpy(data, gpu->vram.virt, len);
  char *data_char = (char *)data;
  for( i = 0; i < len; i += 4 )
  {
    char a = data_char[i + 0];
    char b = data_char[i + 1];
    char c = data_char[i + 2];
    char d = data_char[i + 3];
 
    data_char[i + 0] = d;
    data_char[i + 1] = c;
    data_char[i + 2] = b;
    data_char[i + 3] = a;
 
  }
}*/


static void ramin_write_dword_to_dword_offset( struct gpu *gpu, uint32_t addr, uint32_t data )
{
  uint32_t *fifo = gpu->fifo.virt;
  uint32_t *ctrl = gpu->ctrl.virt;
  uint32_t *vram = gpu->vram.virt;
  uint32_t off = addr & 63;


  //copy aligned line to the begin of framebuffer
  {
    int wptr;
    int ret;
    wptr = (ctrl[0x10] & (gpu->fifo.len - 1)) / 4;
    ret = prepare_ramin_read_line256(&fifo[wptr], addr - off );
    fifo_push(gpu, ret);
    fifo_wait(gpu);

  }
  //wait...
  usleep( 1000 );
  //patch with data
  vram[off] = endian( data );
  //wait...
  usleep( 1000 );
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



static int enter_direct(void)
{
  struct fb_fix_screeninfo fix;
  int ret = 0;
  int fd;
  int val = 0;

  if ((fd = open("/dev/fb0", O_RDWR)) < 0)
  {
    perror("open");
    return -1;
  }

  /* get framebuffer size */
  if ((ret = ioctl(fd, FBIOGET_FSCREENINFO, &fix)) < 0)
  {
    perror("ioctl");
    goto out;
  }


  /* stop that incessant blitting! */
  if ((ret = ioctl(fd, PS3FB_IOCTL_ON, 0)) < 0)
  {
    perror("ioctl");
    goto out;
  }

  /* wait for vsync */
  if ((ret = ioctl(fd, FBIO_WAITFORVSYNC, &val)) < 0)
  {
    perror("ioctl");
    goto out;
  }

  /* wait for vsync */
  if ((ret = ioctl(fd, PS3FB_IOCTL_GPU_SETUP, &val)) < 0)
  {
    perror("ioctl");
    goto out;
  }

  /* keep open */
  return fd;

out:
  close(fd);

  return ret;
}

static int leave_direct(int fd)
{
  int ret = 0;

  if ((ret = ioctl(fd, PS3FB_IOCTL_OFF, 0)) < 0)
  {
    perror("ioctl");
    goto out;
  }

out:
  close(fd);

  return ret;
}

int fb_fd = -1;

void sigint_handler(int sig)
{
  (void) sig;
  if (fb_fd >= 0)
  {
    leave_direct(fb_fd);
    fb_fd = -1;
  }
}

uint32_t RAMIN1[1024 * 1024 / 2];

int main(void)
{
  struct gpu gpu;



  memset(&gpu, 0, sizeof(gpu));

  if (gpu_get_info(&gpu) < 0)
  {
    fprintf(stderr, "Failed to retrieve GPU info\n");
    return -1;
  }

  if (map_gpu(&gpu) < 0)
  {
    fprintf(stderr, "Failed to map gpu card\n");
    return -1;
  }

  fb_fd = enter_direct();
  signal(SIGINT, sigint_handler);


  /*
  memcpy_from_ramin(&gpu, RAMIN1, sizeof(RAMIN1));

  FILE *fp = fopen( "test1", "w" );
  for( i = 0x0; i < 1024 * 1024 / 2; ++i )
  {
    //if( RAMIN1[i] == 0xb0bab0ba )
    fprintf( fp, "%8x %8x \n", i, RAMIN1[i] );
  }
  fclose( fp );*/
  //object ID = 0xfeed0003

  ramin_write_dword_to_dword_offset( &gpu, 0x64cb8, 0xfeed0003 );
  //engine zero, offset
  ramin_write_dword_to_dword_offset( &gpu, 0x64cb9, 0x00105020 );

  //0x40 for NV40, 0x97 - 3D engine
  ramin_write_dword_to_dword_offset( &gpu, 0x1d020 * 4 + 0, 0x00004097 );
  ramin_write_dword_to_dword_offset( &gpu, 0x1d020 * 4 + 1, 0x00000000 );
  //endianness

  ramin_write_dword_to_dword_offset( &gpu, 0x1d020 * 4 + 2, 0x01000000 );
  ramin_write_dword_to_dword_offset( &gpu, 0x1d020 * 4 + 3, 0x00000000 );
  ramin_write_dword_to_dword_offset( &gpu, 0x1d020 * 4 + 4, 0x00000000 );
  ramin_write_dword_to_dword_offset( &gpu, 0x1d020 * 4 + 5, 0x00000000 );
  ramin_write_dword_to_dword_offset( &gpu, 0x1d020 * 4 + 6, 0x00000000 );
  ramin_write_dword_to_dword_offset( &gpu, 0x1d020 * 4 + 7, 0x00000000 );



  gfx_test( &gpu, 0xfeed0003 );

  sleep( 3 );


  if (fb_fd >= 0)
  {
    leave_direct(fb_fd);
    fb_fd = -1;
  }
  unmap_gpu(&gpu);

  return 0;
}
