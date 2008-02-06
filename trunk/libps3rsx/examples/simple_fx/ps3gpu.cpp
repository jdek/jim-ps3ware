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
#include <assert.h>
#include <libspe.h>


extern "C"
{
#include "../../include/matrix.h"
#include "../../src/types.h"
#include "../../include/nouveau_class.h"
#include "../../src/fifo/utils.h"
#include "../../src/fifo/dma.h"
#include "../../src/textures/textures.h"
#include "../../src/geometry/geometry.h"
#include "../../src/geometry/model.h"
#include "../../src/shaders/vertex.h"
#include "../../src/rop/rop.h"
#include "../../src/shaders/fragment.h"
};

#include "structs.h"

#define Nv3D 7


uint32_t width;
uint32_t height;
uint32_t pitch;

#define  BB 64


int set_mvp(  uint32_t *fifo, float angle )
{
	
	float matrix[16];
	
	identity( matrix );
	float w = 0.4f;
	float h = 0.3f;
	
	frustrum( -w, +w, -h, +h, 1.0f, 100.0f, matrix );
	translatef( 0.0f, 0.0f, -4.0f, matrix );
	
	rotatef( angle, 0.0f, 1.0f, 0.0f, matrix );
	rotatef( 90.0f, 1.0f, 0.0f, 0.0f, matrix );

	return set_vertex_shader_constants( matrix, 0, 16, fifo, Nv3D );
	
}


int set_cnst(  uint32_t *fifo )
{
	
	float coeff[8] = { 1.0f / 4096.0f, 1.0f / 4096.0f, 0.0f, 1.0f  };
	return set_vertex_shader_constants( coeff, 4, 4, fifo, Nv3D );
	
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
		printf( "mmaped %10d bytes from %s \n", (uint32_t)buffer.st_size, file );
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


template<class T> struct file_map
{

    int fd;
    int size;
    void *data;
public:
    int get_size() const
    {
	return size;
    }
    T *get_object() const
    {
	return (T *)data;
    }
    file_map( const char *file )
    {
	data = map_file( file, &fd, &size );
    }
    
    ~file_map()
    {
	unmap_file( data, fd, size );
    }
    
};


class  memory_pool
{
    uint8 *data;
    size_t ptr;
    size_t size;
    size_t align;
public:
    size_t get_offset() const
    {
	return ptr;
    }
    memory_pool sub_pool( size_t _size, size_t _align )
    {
	memory_pool pool = *this;
	pool.size = pool.ptr + size;
	pool.align = _align;
	alloc( _size );
	return pool;
    }
    memory_pool() : data( 0 ), ptr( 0 ), size( 0 ), align( 0 ){};
    memory_pool( uint8 *_data, size_t _size, size_t _align ) : data( _data ), ptr( 0 ), size( _size ), align( _align ) {};
    void  *alloc( size_t _size )
    {
	size_t off = _size & ( align - 1 );
	if( off )
	{
	    _size += align - off;
	}
	assert( ptr + _size < size );
	void *result = data + ptr;
	ptr += _size;
	return result;
    }
};


class gpu_ring
{
protected:
    size_t *fifo;
    size_t  cmd_size;
    
public:
    gpu_ring() : fifo( 0 ), cmd_size( 0 ){};
    
    size_t set( uint32_t *_fifo )
    {
	memcpy( _fifo, fifo, cmd_size * 4 );  
	return cmd_size;
    }

};

class vertex_shader_t : public gpu_ring
{
    
public:
    
    bool init( const char *file, memory_pool &xdr_pool )
    {
	file_map<vertex_shader_desc_t> f( file );
	if( vertex_shader_desc_t *desc = f.get_object() )
	{
	    fifo = (size_t *)xdr_pool.alloc( 0 );
	    cmd_size = set_vertex_shader( desc, &desc->aux[1], fifo, Nv3D );
	    xdr_pool.alloc( cmd_size * 4 );
	    return true;
	}
	
	return false;
	
    }
};

class pixel_shader_t : public gpu_ring
{
    
public:
    
    bool init( const char *file, memory_pool &xdr_pool, memory_pool &ddr_pool )
    {
	file_map<fragment_shader_desc_t> f( file );
	if( fragment_shader_desc_t *desc = f.get_object() )
	{
	    size_t off = ddr_pool.get_offset();
	    size_t length = desc->dword_length * 4;
	    memcpy( ddr_pool.alloc( 0 ), &desc->aux[1], length );
	    ddr_pool.alloc( length );
	    fifo = (size_t *)xdr_pool.alloc( 0 );
	    cmd_size = set_fragment_shader( desc, off, fifo, Nv3D );
	    xdr_pool.alloc( cmd_size * 4 );
	    return true;
	}
	
	return false;
	
    }
};

class texture_t : public gpu_ring
{
    
public:
    
    bool init( const char *file, memory_pool &xdr_pool, memory_pool &ddr_pool )
    {
	file_map<texture_desc_t> f( file );
	if( texture_desc_t *desc = f.get_object() )
	{
	    size_t off = ddr_pool.get_offset();
	    size_t length = desc->length;
	    memcpy( ddr_pool.alloc( length ), sizeof( *desc ) + (uint8_t *)desc, desc->length );
	    fifo = (size_t *)xdr_pool.alloc( 0 );
	    cmd_size  = set_texture_2D( desc, off, 0, REPEAT, LINEAR, LINEAR_MIPMAP_LINEAR, fifo, Nv3D );	
	    xdr_pool.alloc( cmd_size * 4 );
	    return true;
	}
	
	return false;
	
    }
};


class raw_data_t 
{
    size_t offset;
    size_t size;
    void   *data;
public:

    void *get_data() const
    {
	return data;
    }
    size_t get_offset() const
    {
	return offset;
    }
    size_t get_size() const
    {
	return size;
    }
    raw_data_t() : offset( 0 ){};
     
    bool init( const char *file, memory_pool &pool )
    {
	file_map<uint8_t> f( file );
	if( uint8_t *desc = f.get_object() )
	{
	    
	    offset = pool.get_offset();
	    size_t length = f.get_size();
	    memcpy( data = pool.alloc( length ), desc, length );
	    size = length;
	    return true;
	}
	
	return false;
	
    }
};

class geometry_t : public gpu_ring
{
public:    
    bool init( const char *file, memory_pool &xdr_pool, size_t vb_offset, size_t ib_offset )
    {
	file_map<model_desc_t> f( file );
	if( model_desc_t *desc = f.get_object() )
	{
	    uint32_t *ptr = (uint32_t *)xdr_pool.alloc( 0 );
	    fifo = ptr;
	    ptr += blend_enable( 0, ptr, Nv3D );
	    ptr += depth_enable( 1, 1, ptr, Nv3D );
	    ptr += set_geometry_source( &desc->position, DDR, vb_offset, ptr, Nv3D );
	    ptr += set_geometry_source( &desc->texcoord, DDR, vb_offset, ptr, Nv3D );
	    ptr += set_index_source( DDR, ib_offset, ptr, Nv3D );
	    ptr += draw_primitives( 1, TRIANGLES, 0, desc->indices_num, ptr, Nv3D );
	    
	    cmd_size = ptr - fifo;
	    xdr_pool.alloc( cmd_size * 4 );
	    return true;
	}
	return false;

    }

};


class quad_geometry_t : public gpu_ring
{
public:    
    bool init( memory_pool &xdr_pool, size_t vb_offset )
    {
	uint32_t *ptr = (uint32_t *)xdr_pool.alloc( 0 );
	fifo = ptr;
	
	geom_desc_t pos, tx0, col;
	
	pos.stride = 24;
	pos.components = 3;
	pos.format = FLOAT;
	pos.type = POS0;
	pos.offset = 0;
	
	col.stride = 24;
	col.components = 4;
	col.format = HALF;
	col.type = COL0;
	col.offset = 16;
	
	tx0.stride = 24;
	tx0.components = 2;
	tx0.format = HALF;
	tx0.type = TEX0; 
	tx0.offset = 12;
	
	ptr += blend_enable( 1, ptr, Nv3D );
	
	blend_desc_t blend;
	blend.src_color_blend = ONE;
	blend.src_alpha_blend = ONE;
	blend.dst_color_blend = ONE;
	blend.dst_alpha_blend = ONE;
	
	
	ptr += set_blend_mode( &blend, ptr, Nv3D );    
	ptr += depth_enable( 0, 1, ptr, Nv3D );
	ptr += set_geometry_source( &pos, DDR, vb_offset, ptr, Nv3D );
	ptr += set_geometry_source( &col, DDR, vb_offset, ptr, Nv3D );
	ptr += set_geometry_source( &tx0, DDR, vb_offset, ptr, Nv3D );
	    
	cmd_size = ptr - fifo;
	xdr_pool.alloc( cmd_size * 4 );
	return true;
	
    }
};

vertex_shader_t mvp; 
pixel_shader_t custom;
texture_t troll_texture;
texture_t particle_texture;

geometry_t troll;
quad_geometry_t particles;



int setup_surfaces_with_offset( uint32_t *fifo, int off )
{
	uint32_t *ptr = fifo;
	setup_buffer_t setup;
	setup.pitchDepth = pitch;
	setup.pitchColor = pitch;
	setup.offsetDepth = width * height * 4 * 3;
	setup.offsetColor = width * height * 4 * off;
	setup.typeDepth = Z16;
	setup.width = width;
	setup.height = height;
	
    
	ptr += setup_buffers( &setup, ptr, Nv3D );
	return ptr - fifo;
}


uint8 data[1024 * 1024] __attribute__((aligned( 128 ) ));
memory_pool xdr_pool( data, sizeof( data ), 128 );
memory_pool fp_pool;
memory_pool tx_pool;
memory_pool vr_pool;
raw_data_t fx;
raw_data_t atlas;
void *dynamic;

int bind3d( uint32_t *fifo, uint32_t *fbmem, uint8_t *xdrmem, uint32_t obj, uint32_t jmp )
{

	uint32_t *ptr = fifo;
	xdrmem = xdrmem;
	mvp.init( "../../data/particle.vertex", xdr_pool );
	memory_pool gfx_pool( (uint8_t *)fbmem, 252 * 1024 * 1024, 1024 * 1024 );
	gfx_pool.alloc( 1024 * 1024 * 32 );
	fp_pool = gfx_pool.sub_pool( 1024 * 1024, 128 );
	custom.init( "../../data/particle.pixel", xdr_pool, fp_pool );
	tx_pool = gfx_pool.sub_pool( 1024 * 1024, 1024 );
	troll_texture.init( "../../data/troll.dxt3",  xdr_pool, tx_pool );
	particle_texture.init( "../../data/spell.dxt3",  xdr_pool, tx_pool );
	
	vr_pool = gfx_pool.sub_pool( 1024 * 1024 * 16, 1024 );
	
	raw_data_t vb;
	raw_data_t ib;

	vb.init( "../../data/troll.0.vb", vr_pool );
	ib.init( "../../data/troll.0.ib", vr_pool );	
	troll.init( "../../data/troll.0.model", xdr_pool, vb.get_offset(), ib.get_offset() );
	
	
	
	fx.init( "../../data/spell", xdr_pool );
	atlas.init( "../../data/spell.dxt3.atlas", xdr_pool );
	particles.init( xdr_pool, vr_pool.get_offset() );
	
	dynamic = vr_pool.alloc( 1024 * 1024 );
	
	
	ptr += setup_and_voodoo( 0x66604200, 0xfeed0000, obj, ptr, Nv3D ); 
	ptr += setup_surfaces_with_offset( ptr, 1 );
	ptr += troll_texture.set( ptr );
	ptr += mvp.set( ptr );
	ptr += set_mvp( ptr, 180.0f );
	ptr += set_cnst( ptr );
	ptr += custom.set( ptr );
	ptr += set_geometry_pipe( 0xfeed0000, 0xfeed0001, ptr, Nv3D );
	ptr += troll.set( ptr );
	ptr += jump_to_address( ptr, jmp );	
	
	return ptr - fifo;
}



extern spe_program_handle_t spu_handle;

struct job_t
{
	speid_t spe_id;
	program_data_t pd __attribute__((aligned(128)));
	job_t() : spe_id( 0 ){};

	void kick_job( )
	{
		
		if( spe_id == 0 )
		{
			spe_id = spe_create_thread( 0, &spu_handle, &pd, NULL, -1, 0 );
		}

	
		while( spe_stat_in_mbox(spe_id) == 0 );
		spe_write_in_mbox( spe_id, 1 );
	}

	void join()
	{
		while( spe_read_out_mbox( spe_id ) == (unsigned int)-1 );
	}

};

#define INUMBER 32
instance_data_t    insts[INUMBER]  __attribute__((aligned( 128 ) ));
uint32_t           dt[INUMBER];
float		   rnds[INUMBER][3];


float rnd( float beg, float end )
{
    return ( rand() / (float) RAND_MAX ) * ( end - beg ) + beg;
}

int gfx_step(  uint32_t *fifo,  uint32_t jmp, int off )
{

	static bool init_particles = false;
	
	if( !init_particles )
	{
	    init_particles = true;
	    
	    for( size_t i = 0; i < INUMBER; ++i )
	    {
		dt[i] = ( rand() & 63 ) + 32;
		rnds[i][0] = 360.0f * i / (float)( INUMBER );//rnd( 0.0f, 360.0f );
		rnds[i][1] = rnd( -0.5f, +0.5f );
		rnds[i][2] = rnd( -0.5f, +0.5f );
		
		insts[i].time 	= rand();
		insts[i].col[0] = rnd( 0.8f, 1.0f );
		insts[i].col[1] = rnd( 0.8f, 1.0f );
		insts[i].col[2] = rnd( 0.8f, 1.0f );
		insts[i].col[3] = 1.0f;				
	    }
	}
	
	
	for( size_t i = 0; i < INUMBER; ++i )
	{
	    insts[i].time += dt[i];
	    identity( insts[i].mat );
	
	    rotatef( 80.0f, 1.0f, 0.0f, 0.0f, insts[i].mat );
	    rotatef( off * 0.5f + rnds[i][0], 0.0f, 0.0f, 1.0f, insts[i].mat );
	    translatef( 1.6f, rnds[i][1], rnds[i][2], insts[i].mat );
	    
	    float d = insts[i].mat[11];
	    
	    d = ( 1.0f - 0.6f * d );
	    insts[i].col[0] = d;
	    insts[i].col[1] = 0.5f + d;
	    insts[i].col[2] = d;
	    
	    //translatef( 1.0f * sin( t * rnds[i][0] ), 1.0f * cos( t * rnds[i][0] ), 1.0f * sin( t * rnds[i][2] ), insts[i].mat );
	}
	
	
	
	clear_buffer_t clear;
	clear.clearR = clear.clearG = clear.clearB = clear.clearA = clear.clearD = 1;
	clear.rgba = 50 + ( 10 << 8 ) + ( 5 << 16 );
	clear.depth = 0xffff;

	static job_t job;
	
	static size_t curr = 0x0;

	job.pd.insts = (uint32_t)insts;
	job.pd.isize = INUMBER;
	job.pd.dynamic = (uint32_t)( dynamic ) + curr * 24;
	job.pd.fx = (uint32_t)fx.get_data();
	job.pd.atlas = (uint32_t)atlas.get_data();
	job.pd.asize = atlas.get_size();
	job.kick_job( );
	job.join();

	
	uint32_t *ptr = fifo;
	static float angle = 180.0f;
	ptr += setup_surfaces_with_offset( ptr, off % 3 );
	ptr += clear_buffers( &clear, ptr, Nv3D );
	
	ptr += set_mvp( ptr, angle );
	ptr += set_cnst( ptr );
	//ptr += troll_texture.set( ptr );
	//ptr += troll.set( ptr );
	
	//ptr += custom.set( ptr );
	ptr += particles.set( ptr );
	ptr += particle_texture.set( ptr );
	
	ptr += draw_primitives( 0, QUADS, curr, job.pd.quads * 4, ptr, Nv3D );
	
	curr += job.pd.quads * 4;// + 128;
	curr = ( curr + 16 ) & 2047;
	
	//printf( "%d \n", job.pd.quads );
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
	uint32_t *fifo = (uint32_t *)gpu->fifo.virt;
	uint32_t *ctrl = (uint32_t *)gpu->ctrl.virt;
	uint32_t *vram = (uint32_t *)gpu->vram.virt;
	uint8_t *xram = (uint8_t *)gpu->xram.virt;
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
	
	for( i = 0; i < 2000; ++i )
	{
	
	    //printf( "asd \n" );
	    memset( fifo, 0, gpu->fifo.len );
	    
	    uint32_t jmpt = jmp + 4 + 4 * ( i & 1 );
	    fifo[ ( old_jump - jmp ) / 4 ] = 0x20000000 | ( jmp + 0x10 );
	    old_jump = jmpt;
	    gfx_step( &fifo[0x10], jmpt, i );
	    wait( ctrl, jmpt );
	    flip_scrn( gpu, ( ( i + 2 ) % 3 ) * width * height * 4 );
	    sync_gpu( gpu );
	    //flip_scrn( gpu, ( ( i + 1 ) & 1 ) * width * height * 4 );
	    
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
