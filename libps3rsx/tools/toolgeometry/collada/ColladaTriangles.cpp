#include "StdAfx.h"
#include "XMLUtils.h"
#include "ColladaTriangles.h"
#include "ColladaSource.h"
#include "ColladaVertices.h"
#include "ColladaGeometry.h"
#include "ColladaImage.h"
#include "ColladaEffect.h"
#include "ColladaBindMaterial.h"
#include "ColladaController.h"



CColladaTriangles::CColladaTriangles(void) : bindController( 0 )
{
	positionsIB = 0;
	normalsIB = 0;
	positions = 0;
	normals = 0;
	for( size_t i = 0; i < LENGTH( texcoordsIB ); ++i )
	{
		texcoordsIB[i] = 0;
		texcoords[i] = 0;
	}
	stride = 0;
}

CColladaTriangles::~CColladaTriangles(void)
{

}

void CColladaTriangles::SetBindController( CColladaController * controller )
{
	bindController = controller;
}


void CColladaTriangles::InitInput( const  xmlNode * input )
{
	const xmlChar *semantic;
	const xmlChar *source;
	const xmlChar *offset;

	if
	( 
		( semantic = LoadStrAttr( input, _X"semantic" ) ) &&
		( source = LoadStrAttr( input, _X"source" ) ) &&
		( offset = LoadStrAttr( input, _X"offset" ) ) 
	)
	{
		CColladaSource *sourceArray = CColladaSource::Get( source );
		size_t numslice = atoi((const char *)offset);
		if( sourceArray != 0 )
		{
			if( !xmlStrcmp( semantic, _X"NORMAL" ) )
			{
				sourceArray->BuildContent();
				normals = sourceArray;
				normalsIB = numslice;

			}
			if( !xmlStrcmp( semantic, _X"POSITION" ) )
			{
				assert( 0 );
			}
			if( !xmlStrcmp( semantic, _X"TEXCOORD" ) )
			{
				const xmlChar *set = LoadStrAttr( input, _X"set" );
				if( set )
				{
					size_t index = atoi((const char * )set);
					texcoords[index] = sourceArray;
					sourceArray->BuildContent();
					texcoordsIB[index] = numslice;
				}
			}
		}

		if( !xmlStrcmp( semantic, _X"VERTEX" ) )
		{
			CColladaVertices *vertices = CColladaVertices::Get( source );
			if( vertices && vertices->GetPositions() )
			{
				positions = vertices->GetPositions();
				positions->BuildContent();
				positionsIB = numslice;
			}
			if( vertices && vertices->GetNormals() )
			{
				normals = vertices->GetNormals();
				normals->BuildContent();
				normalsIB = numslice;
			}
		}

		if( numslice + 1 > stride )
		{
			stride = numslice + 1;
		}
	}
}

uint64 Cook( uint16 pos, uint16 nor, uint16 tex, uint16 aux = 0 )
{
	return ( uint64( pos ) << 0  ) +
		   ( uint64( nor ) << 16 ) +
		   ( uint64( tex ) << 32 ) +
		   ( uint64( aux ) << 48 );
};

void FillPointer( CColladaSource *source, size_t stride, uint16 pos, float *dst  )
{
	if( source )
	{
		assert( source->GetFloatStride() == stride );
		{
			const std::vector<float> &arr = source->GetFloatArray();
			size_t realPos = stride * pos;
			assert( arr.size() > (size_t)( realPos + stride - 1 ) );
			if(  arr.size() > (size_t)( realPos + stride - 1 ) )
			{
				for( size_t i = 0; i < stride; ++ i )
				{
					dst[i] = arr[ realPos + i ];
				}
			}
		}
	}
}

const CColladaEffect *CColladaTriangles::GetEffect()
{
	return effect;
}
void CColladaTriangles::GetFatVertices( std::vector<SFatVertex> *vertices, std::vector<uint16> *indices )
{
	std::map< uint64, SAutoInd > bind;

	if( !vertices || !indices )
	{
		return;
	}

	vec3 minV( +10000.0f, +10000.0f, +10000.0f );
	vec3 maxV( -10000.0f, -10000.0f, -10000.0f );

	for( size_t i = 0; i < body.size() / stride; ++i )
	{
		uint16   *ptr = &body[ i * stride ];
		uint16    pos =  ptr[ positionsIB ];
		uint16    nor =  ptr[ normalsIB ];
		uint16    tx0 =  ptr[ texcoordsIB[0] ];
		uint64    lookup = Cook( pos, nor, tx0 );

		SAutoInd &index  = bind[ lookup ];

		if( index.value == SAutoInd::GetBadValue() )
		{
			index.value = (uint16)vertices->size();
			assert( (size_t)index.value == vertices->size() );
			if( (size_t)index.value != vertices->size() )
			{
				return;
			}

			SFatVertex vertex = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } };
			FillPointer( positions, 3, pos, vertex.coo );
			FillPointer( normals, 3, nor, vertex.nor );
			FillPointer( texcoords[0], 2, tx0, vertex.tx0 );

		    SWeightInfo info;

			if( positions && bindController )
			{
				const std::vector<SWeightInfo> &weights = bindController->GetWeights();
				assert( weights.size() == positions->GetFloatArray().size() / 3 );
				info = weights[pos];

				float dst[3];
				const mat3x4 &mat = bindController->GetMatrix();

				for( size_t j = 0; j < 3; ++j )
				{
					dst[j] = vertex.coo[j];
				}
				for( size_t j = 0; j < 3; ++j )
				{
					vertex.coo[j] = mat.vec[j][0] * dst[0] + mat.vec[j][1] * dst[1] + mat.vec[j][2] * dst[2] + mat.vec[j][3]; 
				}
			};

			for( size_t j = 0; j < 4; ++j )
			{
				vertex.indices[j] = (uint8) ( 3 * info.indices[j] );
				vertex.weights[j] = (uint8) ( 255.0f * info.weights[j] );
			}

			for( size_t k = 0; k < 4; ++k )
			{
				for( size_t j = 0; j < 3; ++j )
				{
					if( vertex.weights[j] < vertex.weights[j + 1] )
					{
						std::swap(  vertex.weights[j], vertex.weights[j + 1] );
						std::swap(  vertex.indices[j], vertex.indices[j + 1] );
					}
				}
			}

			vec3 coo( vertex.coo[0], vertex.coo[1], vertex.coo[2] );
			minV.minimize( coo );
			maxV.maximize( coo );

			vertex.tx0[1] = 1.0f - vertex.tx0[1];
			
			
			///std::swap( vertex.tx0[0], vertex.tx0[1] );
			vertices->push_back( vertex );
		}
		indices->push_back( index.value );
	}

	box.center  = ( maxV + minV ) * 0.5f;
	box.extends = ( maxV - minV ) * 0.5f;
}

CColladaTriangles *CColladaTriangles::CreateInstance( const xmlNode * root )
{
	const xmlNode *input = root->children;
	const xmlNode *p;
	const xmlChar *count;
	const xmlChar *material;
	CColladaTriangles *triangles = new CColladaTriangles();
	while( input )
	{
		input = Find( input, _X"input" );
		if( input )
		{
			triangles->InitInput( input );
			input = input->next;
		}
	}

	p = Find( root->children, _X"p" );
	count = LoadStrAttr( root, _X"count" ) ;
	
	if( p && count )
	{
		size_t nCount = atoi((char *)count) * triangles->stride * 3;
		triangles->body.resize( nCount );

		const xmlChar *v = p->children->content;
		for( size_t i = 0; i < nCount; ++i )
		{
			while( IsSpace(*v) )v++; 
			int val = atoi( (const char *)v );
			while( !IsSpace(*v) )v++; 
			triangles->body[i] = (uint16)val;
		}

	}

	material = LoadStrAttr( root, _X"material" );
	triangles->effect = CColladaBindMaterial::GetEffectBySymbol( material );
	const xmlChar *ParentId = LoadStrAttr( root->parent->parent, _X"id" );
	CColladaGeometry *parent = CColladaGeometry::Get( ParentId );
	if( parent )
	{
		parent->AddMesh( triangles );
		return triangles;
	}

	delete triangles;
	return 0;

}
