#include <assert.h>

struct instance_data_t
{
    float    col[4];
    float    mat[16];
    uint32_t time;
    float    scale;
    float    padd[2];
};

struct program_data_t
{
    uint32_t  fx;
    uint32_t  dynamic;
    uint32_t  quads;
    uint32_t  atlas;
    uint32_t  asize;
    uint32_t  insts;
    uint32_t  isize;
    uint32_t  padd;    
};






