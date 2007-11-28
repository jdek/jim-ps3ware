/*
 * Copyright 2007 Peter Popov <IronSPeter@gmail.com>
 *
 */

typedef struct
{
geom_desc_t  position;
geom_desc_t  texcoord;
primitives_t indices;
uint16_t     vertices_num;
uint16_t     indices_num;
}model_desc_t;
