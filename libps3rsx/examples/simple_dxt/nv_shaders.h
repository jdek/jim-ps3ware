#ifndef __NV_SHADERS_H__
#define __NV_SHADERS_H__

#define NV_SHADER_MAX_PROGRAM_LENGTH 256

typedef struct nv_vshader {
	uint32_t hw_id;
	uint32_t size;
	
	uint32_t vp_in_reg;
	uint32_t vp_out_reg;
	
	uint32_t data[NV_SHADER_MAX_PROGRAM_LENGTH];
} nv_vshader_t;

typedef struct nv_pshader {
	uint32_t hw_id;
	uint32_t size;
	uint32_t num_regs;
	uint32_t data[NV_SHADER_MAX_PROGRAM_LENGTH];
} nv_pshader_t;

/*******************************************************************************
 * NV40/G70 vertex shaders
 */



static nv_vshader_t nv40_vp_mul = {

  .vp_in_reg  = 0x00000309,
  .vp_out_reg = 0x0000c001,
  .size = (5*4),
  .data = {
/* DP4 result.position.x, vertex.position, c0 */
0x401f9c6c, 0x01c0000d, 0x8186c083, 0x60411f80,
/* DP4 result.position.y, vertex.position, c1 */
0x401f9c6c, 0x01c0100d, 0x8186c083, 0x60409f80,
/* DP4 result.position.z, vertex.position, c2 */
0x401f9c6c, 0x01c0200d, 0x8186c083, 0x60405f80,
/* DP4 result.position.w, vertex.position, c3 */
0x401f9c6c, 0x01c0300d, 0x8186c083, 0x60403f80,
/* MOV result.texcoord[0], vertex.texcoord[0] */
0x401f9c6c, 0x0040080d, 0x8106c083, 0x6041ff9d,
}
};


/*******************************************************************************
 * NV30/NV40/G70 fragment shaders
 */

static nv_pshader_t nv30_fp = {
.num_regs = 2,
.size = (2*4),
.data = {
/* TEX R0, fragment.texcoord[0], texture[0], 2D */
0x17009e00, 0x1c9dc801, 0x0001c800, 0x3fe1c800,
/* MOV R0, R0 */
0x01401e81, 0x1c9dc800, 0x0001c800, 0x0001c800,
}
};



#endif
