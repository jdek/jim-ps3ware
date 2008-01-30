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
#include <time.h>


#include "../../include/matrix.h"
#include "../../src/types.h"
#include "../../include/nouveau_class.h"
#include "../../src/fifo/utils.h"
#include "../../src/fifo/dma.h"
#include "../../src/textures/textures.h"
#include "../../src/geometry/geometry.h"
#include "../../src/geometry/model.h"
#include "../../src/shaders/vertex.h"
#include "../../src/shaders/fragment.h"




#define Nv3D 7


uint32_t width;
uint32_t height;
uint32_t pitch;

#define  BB 74


uint32_t fp_offset = ( BB + 0 ) * 1024 * 1024;
uint32_t vb_offset = ( BB + 1 ) * 1024 * 1024;
uint32_t ib_offset = ( BB + 2 ) * 1024 * 1024;
uint32_t tx_offset = ( BB + 3 ) * 1024 * 1024;
uint32_t pl_offset = ( BB + 4 ) * 1024 * 1024;




int put_dma( struct gpu *gpu, uint32_t *fifo, uint32_t *fbmem, uint32_t data, uint32_t dst_off  )
{
	dma_pool_t pool;
	pool.dma_pool = fbmem + pl_offset / 4;
	pool.gpu_dst_base = gpu->ioif;
	pool.gpu_src_base = pl_offset;
	pool.dma_pool_size = 1024;
	return put_dma_dword_async( &pool, data, DDR_TO_XDR, dst_off, fifo );
}


int set_mvp(  uint32_t *fifo, float angle )
{
	
	float matrix[16];
	
	identity( matrix );
	float w = 0.4f;
	float h = 0.3f;
	
	frustrum( -w, +w, -h, +h, 1.0f, 100.0f, matrix );
	translatef( 0.0f, 1.5f, -8.0f, matrix );
	
	rotatef( angle, 0.0f, 1.0f, 0.0f, matrix );
	rotatef( 90.0f, 1.0f, 0.0f, 0.0f, matrix );

	return set_vertex_shader_constants( matrix, 0, 16, fifo, Nv3D );
	
}


void  *map_file( const char *file, int *fd, int *size )
{
	*size = 0;
	*fd = open( file, O_RDONLY );
	if( (*fd) != -1 )
	{
		int status;
		struct stat buffer;
		status = fstat( *fd, &buffer );
		*size = buffer.st_size;
		printf( "mmaped %d bytes from %s \n", (uint32_t)buffer.st_size, file );
		return mmap( 0, buffer.st_size, PROT_READ, MAP_PRIVATE, *fd, 0 );
	}
	return 0;

}

void  unmap_file( void *data, int fd, int size )
{
	
	if( data != 0 )
	{
		munmap( data, size );
	}
	if( fd != -1 )
	{
		close( fd );
	}
}


int load_vertex_shader(  uint32_t *fifo )
{
	int fd, size;
	void *file = map_file( "../../data/mvp.vertex", &fd, &size );

	if( size && file )
	{
		vertex_shader_desc_t *desc = file;
		int res = set_vertex_shader( desc, &desc->aux[1], fifo, Nv3D );
		unmap_file( file, fd, size );
		return res;
	}

	return 0;
}



int load_pixel_shader(  uint32_t *fifo, uint8_t *fbmem )
{
	int fd, size;
	void *file = map_file( "../../data/custom.pixel", &fd, &size );

	if( size && file )
	{
		fragment_shader_desc_t *desc = file;
		memcpy(fbmem + fp_offset, &desc->aux[1], desc->dword_length * 4 );
		int res = set_fragment_shader( desc, fp_offset, fifo, Nv3D );
		unmap_file( file, fd, size );
		return res;
	}

	return 0;
}


int load_texture( uint32_t *fifo, uint8_t *fbmem )
{

	int fd, size;
	void *file = map_file( "../../data/troll.dxt3", &fd, &size );

	if( size && file )
	{
		texture_desc_t *desc = file;
		memcpy( fbmem + tx_offset, sizeof( *desc ) + (uint8_t *)file, desc->length );
		int res = set_texture_2D( desc, tx_offset, 0, REPEAT, LINEAR, LINEAR_MIPMAP_LINEAR, fifo, Nv3D );
		unmap_file( file, fd, size );
		return res;
	}

	return 0;
}


uint32_t     indices_num = 0;
primitives_t indices;

int load_geometry( uint32_t *fifo, uint8_t *mem )
{

	uint32_t *ptr = fifo;


	ptr += set_geometry_pipe( 0xfeed0000, 0xfeed0001, ptr, Nv3D );

	{
		int fd, size;
		void *file = map_file( "../../data/troll.0.model", &fd, &size );

		if( size && file )
		{
			model_desc_t *model = file;
			ptr += set_geometry_source( &model->position, DDR, vb_offset, ptr, Nv3D );
			ptr += set_geometry_source( &model->texcoord, DDR, vb_offset, ptr, Nv3D );
			ptr += set_index_source( DDR, ib_offset, ptr, Nv3D );
			indices_num = model->indices_num;
			indices = model->indices;    
			unmap_file( file, fd, size );
		}
	}

	{
		int fd, size;
		void *file = map_file( "../../data/troll.0.ib", &fd, &size );

		if( size && file )
		{
			memcpy( mem + ib_offset, file, size );
			unmap_file( file, fd, size );
		}
	}

	{
		int fd, size;
		void *file = map_file( "../../data/troll.0.vb", &fd, &size );

		if( size && file )
		{
			memcpy( mem + vb_offset, file, size );
			unmap_file( file, fd, size );
		}
	}

	return ptr - fifo;

}


int setup_surfaces_with_offset( uint32_t *fifo, int off )
{
	uint32_t *ptr = fifo;
	setup_buffer_t setup;
	setup.pitchDepth = pitch;
	setup.pitchColor = pitch;
	setup.offsetDepth = width * height * 4 * 2;
	setup.offsetColor = width * height * 4 * off;
	setup.typeDepth = Z16;
	setup.width = width;
	setup.height = height;
	
    
	ptr += setup_buffers( &setup, ptr, Nv3D );
	return ptr - fifo;
}

int bind3d( uint32_t *fifo, uint32_t *fbmem, uint8_t *xdrmem, uint32_t obj, uint32_t jmp )
{

	uint32_t *ptr = fifo;
	xdrmem = xdrmem;

	
	ptr += setup_and_voodoo( 0x66604200, 0xfeed0000, obj, ptr, Nv3D ); 
	ptr += setup_surfaces_with_offset( ptr, 1 );
	ptr += load_texture( ptr, (uint8_t *)fbmem );
	ptr += load_vertex_shader( ptr );
	ptr += set_mvp( ptr, 180.0f );
	ptr += load_pixel_shader( ptr, (uint8_t *)fbmem );
	ptr += load_geometry( ptr, (uint8_t *)fbmem );
	ptr += jump_to_address( ptr, jmp );	

	return ptr - fifo;
}

int gfx_step(  uint32_t *fifo,  uint32_t jmp, int off )
{
	clear_buffer_t clear;
	clear.clearR = clear.clearG = clear.clearB = clear.clearA = clear.clearD = 1;
	clear.rgba = 250 +  ( 120 << 8 ) + ( 50 << 16 );
	clear.depth = 0xffff;
	


	uint32_t *ptr = fifo;
	static float angle = 180.0f;
	ptr += setup_surfaces_with_offset( ptr, off );
	ptr += clear_buffers( &clear, ptr, Nv3D );
	ptr += set_mvp( ptr, angle += 0.4f );
	ptr += draw_indexed_primitives( indices, 0, indices_num, ptr, Nv3D );
	ptr += jump_to_address( ptr, jmp );	


	return ptr - fifo;
}


static volatile int cnt;

void wait( volatile uint32_t *volatile ctrl, uint32_t jmp )
{
    	ctrl[0x10] = jmp; 
	while( ctrl[0x11] != jmp  )
	{
	    ++cnt;
	}

}


static void gfx_test(struct gpu *gpu, unsigned int obj )
{
	uint32_t *fifo = gpu->fifo.virt;
	uint32_t *ctrl = gpu->ctrl.virt;
	uint32_t *vram = gpu->vram.virt;
	uint8_t *xram = gpu->xram.virt;
	uint32_t i;

	memset( fifo, 0, gpu->fifo.len );
	int wptr;
	int ret;
	uint32 jmp = ctrl[0x10] & ~(gpu->fifo.len - 1 );

	wptr = (ctrl[0x10] & (gpu->fifo.len - 1)) / 4;
	
	printf( "%x %x %x\n", wptr, ctrl[0x10], gpu->xram.len );

	ret = bind3d( &fifo[wptr], vram, xram, obj, jmp );
	wait( ctrl, jmp );
	
	uint32 old_jump = jmp;
	
	for( i = 0; i < 1000; ++i )
	{
	    memset( fifo, 0, gpu->fifo.len );
	    
	    uint32_t jmpt = jmp + 4 + 4 * ( i & 1 );
	    fifo[ ( old_jump - jmp ) / 4 ] = 0x20000000 | ( jmp + 0x10 );
	    old_jump = jmpt;
	    gfx_step( &fifo[0x10], jmpt, i & 1 );
	    wait( ctrl, jmpt );
	    sync_gpu( gpu );
	    flip_scrn( gpu, ( ( i + 1 ) & 1 ) * width * height * 4 );
	}
	printf( "done...\n" );
	
}

struct gpu gpu;

void sigint_handler(int sig)
{
	(void) sig;
	gpu_cleanup(&gpu);
}


int main(void)
{
	if (gpu_init(&gpu) < 0)
	{
		fprintf(stderr, "Failed to initialize GPU\n");
		return -1;
	}

	width = gpu.res.xres;
	height = gpu.res.yres;
	pitch = width * 4;
	signal(SIGINT, sigint_handler);

	gfx_test( &gpu, 0xfeed0007 );

	//sleep( 3 );

	gpu_cleanup(&gpu);

	return 0;
}
