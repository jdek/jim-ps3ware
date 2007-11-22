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
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <math.h>

#include "../../src/types.h"
#include "../../include/nouveau_class.h"
#include "../../src/fifo/utils.h"
#include "../../src/textures/textures.h"
#include "nv_shaders.h"

#include <linux/types.h>
#include <linux/fb.h>
#include <asm/ps3fb.h>


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

#define  BB 31

uint32_t fp_offset = ( BB + 0 ) * 1024 * 1024;
uint32_t vb_offset = ( BB + 1 ) * 1024 * 1024;
uint32_t ib_offset = ( BB + 2 ) * 1024 * 1024;
uint32_t tx_offset = ( BB + 3 ) * 1024 * 1024;

uint8_t data[256][256][4];

int NV40_LoadTexDXT( uint32_t *fifo, uint8_t *fbmem )
{
  
  int fp = open( "../../data/p512.dxt3", O_RDONLY );
  
  if( fp != -1 )
  {
    int status;
    struct stat buffer;
    status = fstat( fp, &buffer );

    void *file = mmap( 0, buffer.st_size, PROT_READ, MAP_PRIVATE, fp, 0 );
    texture_desc_t *desc = file;    
    if( desc )
    {	
	printf( "%d\n", desc->length );
	uint8 *data = file;
	data += sizeof( *desc ); 
	memcpy( fbmem + tx_offset, data, desc->length );
	close( fp ); 
	return set_texture_2D( desc, tx_offset, 0, CLAMP, LINEAR, LINEAR_MIPMAP_LINEAR, fifo, Nv3D ); 
  	
    }
    
    close( fp );
    
  
  }
  
  return 0;
  

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
  uint32_t indices = ib_offset;
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
  
  #define NV40TCL_INDEX_ADDRESS 0x181c
  #define NV40TCL_INDEX_CONTROL 0x1820
  #define NV40TCL_INDEX_DATA    0x1824
  
  BEGIN_RING(Nv3D, NV40TCL_INDEX_ADDRESS, 1 );
  OUT_RING( indices | ( 0 << 31 ) );
  
  BEGIN_RING(Nv3D, NV40TCL_INDEX_CONTROL, 1 );
  OUT_RING( 0x11 );
  
  float *data = (float *)( mem + offset );
  uint16_t *index_data = (uint16_t *)( mem + indices );
  
  

  uint32_t vnum = 120;
  int o = 0;
  
  for( i = 0; i < 30; ++i )
  {
  
    index_data[i * 6 + 0] = i * 4 + 0;
    index_data[i * 6 + 1] = i * 4 + 1;
    index_data[i * 6 + 2] = i * 4 + 2;
    index_data[i * 6 + 3] = i * 4 + 2;
    index_data[i * 6 + 4] = i * 4 + 1;
    index_data[i * 6 + 5] = i * 4 + 3;
      
    int j = pow( 1.6, i );
         
    CV_OUT0( o,     100,     	0.0f, 0.0f, 0.0f  );
    CV_OUT0( o,     100 + j, 	0.0f, 0.0f, 1.0f  );
    CV_OUT0( o + j, 100, 	0.0f, 1.0f, 0.0f  );
    CV_OUT0( o + j, 100 + j,    0.0f, 1.0f, 1.0f  );
    o += j;

  }

  
  
  BEGIN_RING(Nv3D, NV40TCL_BEGIN_END, 1);
  OUT_RING  (NV40TCL_BEGIN_END_TRIANGLES);

  BEGIN_RING(Nv3D, NV40TCL_INDEX_DATA, 1 );
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
  OUT_RING( ( 250 << 16 ) + ( 150 << 8 ) + 50  );

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

  ptr += NV40_LoadTexDXT( ptr, (uint8_t *)fbmem );
  ptr += NV40_LoadVtxProg( ptr,  &nv40_vp );
  ptr += NV40_LoadFragProg( ptr, fbmem,  &nv30_fp );
  //ptr += NV40_EmitGeometry( ptr );
  ptr += NV40_EmitBufferGeometry( ptr, (uint8_t *)fbmem );


  return ptr - fifo;
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
