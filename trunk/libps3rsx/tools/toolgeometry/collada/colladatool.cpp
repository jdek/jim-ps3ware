#include "StdAfx.h"

#include "XMLUtils.h"
#include "ColladaTriangles.h"
#include "ColladaImage.h"
#include "ColladaEffect.h"
#include "ColladaFloatArray.h"
#include "ColladaIDREFArray.h"
#include "ColladaSource.h"
#include "ColladaVertices.h"
#include "ColladaGeometry.h"
#include "ColladaController.h"
#include "ColladaMaterial.h"
#include "ColladaBindMaterial.h"
#include "ColladaNode.h"
#include "ColladaAnimation.h"

#include "../../../src/geometry/geometry.h"
#include "../../../src/geometry/model.h"

struct cache_opt_t
{
	uint16 entries[4 + 3];

	cache_opt_t( ) 
	{
		entries[0] = 0xffff;
		entries[1] = 0xffff;
		entries[2] = 0xffff;
		entries[3] = 0xffff;
	}

	int PushTriangle( uint16 *trn )
	{
		uint32 ptr = 4;

		for( size_t j = 0; j < 3; ++j )
		{
			for( size_t i = 0; i < 4; ++i )
			{
				if( entries[i] == trn[j] )
				{
					goto skip;
				}

			}



			entries[ptr++] = trn[j];

		skip:;
		}

		ptr -= 4;

		for( size_t i = 0; i < 4; ++i )
		{
			entries[i] = entries[i + ptr];

		}

		return ptr;

	}

	int GetTriangleQuality( uint16 *trn ) const
	{
		int res = 0;
		for( size_t i = 0; i < 4; ++i )
		{
			if( entries[i] == trn[0] )
			{
				++res;
			}
			if( entries[i] == trn[1] )
			{
				++res;
			}
			if( entries[i] == trn[2] )
			{
				++res;
			}
		}
		return res;
	}

};


void re_arrange( const std::vector<uint16> &_indices, std::vector<uint16> &dst )
{
	if( _indices.empty() )
	{
		return;
	}
	float misses = 0.0f;
	std::vector<uint16> indices = _indices;
	dst.clear();

	cache_opt_t cache;

	for( size_t j = 0; j < indices.size(); j += 3 )
	{
		int quality = -1;
		int index = -1;

		for( size_t i = 0; i < indices.size(); i += 3 )
		{
			if( indices[i] != 0xffff )
			{
				int newQuality = cache.GetTriangleQuality( &indices[i] );
				if( newQuality >= quality )
				{
					quality = newQuality;
					index = i;
				}
			}
		}
		if( index != -1 )
		{
			misses += cache.PushTriangle( &indices[index] );
			dst.push_back( indices[index + 0] );
			dst.push_back( indices[index + 1] );
			dst.push_back( indices[index + 2] );
			indices[index] = 0xffff;
		}
		else
		{
			printf( "bad logic \n" );
		}
	}

	

	misses = 3.0f * misses / (float)indices.size();
	printf( "cache misses per tri %f \n", misses );

}


#define CHUNK 64


struct MeshHeader
{
}

struct MatrixAffector
{
    uint8  bone;
    uint8  geomChunks[3];
    uint8  weights[3][4];
    
    MatrixAffector();
    
    MatrixAffector( uint8 _bone, uint8 chunk, uint8 *_weights ) : bone( _bone )
    {

	geomChunks[0] = 0xff;
	geomChunks[1] = 0xff;
	geomChunks[2] = 0xff;
	
	SetChunk( bone, chunk, _weights );
	
    }
    
    bool SetChunk( uint8 _bone, uint8 chunk, uint8 *_weights )
    {

	if( bone != _bone )
	{
	    return false;
	}
	
	for( size_t i = 0; i < 3; ++i )
	{
	    if( geomChunks[i] == 0xff )
	    {
		geomChunks[i] = chunk;
		memcpy( weights[i], weights, 4 );
		return true;
	    }
	}
	
	return false;
    }
};


void AddMatrix( size_t lastMark, std::vector<MatrixAffector> &chunks, uint8 bone, uint8 chunk, uint8 *weights )
{
    for( size_t i = lastMark; i < chunks.size(); ++i )
    {
	if( chunks[i].SetChunk( bone, chunk, weights ) )
	{
	    return;
	}
    } 
    
    chunks.push_back( MatrixAffector( bone, chunk, weights ) );
    
}

void remap_vertices( std::vector<SFatVertex> &vertices, std::vector<uint16> &indices )
{
	uint16 beg = 0;
	std::vector<uint16> remap;
	remap.resize( vertices.size(), 0xffff );
	std::vector<SFatVertex> tempVertices( vertices.size() );
	std::vector<uint16>	tempIndices( indices.size() );
	
	for( size_t i = 0; i < indices.size(); ++i )
	{
	    uint16 ind = indices[i];
	    if( remap[ind] == 0xffff )
	    {
		remap[ind] = beg++;
	    }
	    
	    tempIndices[i] = remap[ind];
	    tempVertices[remap[ind]] = vertices[ind];
	}
	
	vertices = tempVertices;
	indices = tempIndices;
	
	uint8 table[256];
	uint8 value[256][4];
	
	memset( table, 0, sizeof( table ) );
	memset( value, 0, sizeof( value ) );
	
	float matrices = 0.0f;
	float clusters = 0.0001f + vertices.size();
	
	std::vector<MatrixAffector> chunks;
	std::vector<size_t> marks;
	size_t lastChunk = 0;
	size_t lastMark = 0;
	
	for( size_t i = 0; i < vertices.size(); i += 4 )
	{
	    uint8 ptr = 0;
	    uint8 weights[16][4];
	    uint8 indices[16];
	    
	    
	    
	    for( size_t j = 0; j < 4 && ( i + j < vertices.size() ); ++j )
	    {
		for( size_t k = 0; k < 4; ++k )
		{
		    uint8 ind = vertices[i + j].indices[k];
		    value[ind][0] = 0;
		    value[ind][1] = 0;
		    value[ind][2] = 0;
		    value[ind][3] = 0;
		    
		    table[ind] = 0;
		}
	    }
	    for( size_t j = 0; j < 4 && ( i + j < vertices.size() ); ++j )
	    {
		for( size_t k = 0; k < 4; ++k )
		{
		    uint8 ind = vertices[i + j].indices[k];
		    uint8 w = vertices[i + j].weights[k];
		    table[ind] += w > 0 ? 1 : 0;
		    value[ind][j] += w;
		}
		
	    }    
	    
	    for( size_t j = 0; j < 4 && ( i + j < vertices.size() ); ++j )
	    {
		for( size_t k = 0; k < 4; ++k )
		{
		    uint8 ind = vertices[i + j].indices[k];
		    if( table[ind] > 0 )
		    {
			//printf( "%3d ", ind );
			for( size_t l = 0; l < 4; ++l )
			{
			    //printf( "%3d ", value[ind][l] );
			    weights[ptr][l] = value[ind][l];
			}
			indices[ptr] = ind;
			table[ind] = 0;
			++ptr;
			//printf( "\n" );
		    }
		}
		
		//printf( "%d matrices \n", ptr );
	    }
	    
	    
	    for( size_t j = 0; j < 4 && ( i + j < vertices.size() ); ++j )
	    {
		uint8 w = 0;
		for( size_t l = 0; l < ptr; ++l )
		{
		    w += weights[l][j];
		}
		
	    }
	    
	    for( size_t l = 0; l < ptr; ++l )
	    {
		AddMatrix( lastMark, chunks, table[l], i, &weights[l][0] );
	    }
		
	
		
	    //matrices += ptr;
	    if( ( i - lastChunk >= CHUNK - 4 ) || ( i + 4 >= vertices.size() ) )
	    {
		marks.push_back( lastMark );
		size_t s = chunks.size() - lastMark;
		//lastChunk = i;
		matrices += 3.0f * 4.0f * s;
		//printf( "chunk with %2d vertices and 3x %2d matrices \n", i - lastChunk + 4, s );
		lastMark = chunks.size();
		lastChunk = i;
	    }
	}
	
	printf( "%d matrix groups in %d chunks  \n", chunks.size(), marks.size() );
	printf( "%f matrices per vertex \n", matrices / clusters );
}

struct FOpen
{
	FILE *fp;
	FOpen( const char *file, bool read )
	{
		fp = fopen( file, read ? "rb" : "wb" );
	}
	~FOpen()
	{
		fclose( fp );
	}
};


struct local_vertex_t
{
	float coo[3];
	float tex[2];
}
;


void convert_model( const char *fileNameOut, const char *fileNameIn )
{
	xmlDoc *doc = NULL;
	xmlNode *root = NULL;
	doc = xmlReadFile( fileNameIn, NULL, 0 );

	if (doc == NULL)
	{
		printf("error: could not parse file %s \n", fileNameIn );
		return;
	}


	root = xmlDocGetRootElement(doc);
	CXMLLoader<CColladaImage> imageLoader;
	CXMLLoader<CColladaEffect> effectLoader;
	CXMLLoader<CColladaMaterial> materialLoader;
	CXMLLoader<CColladaBindMaterial> bindMaterialLoader;
	CXMLLoader<CColladaFloatArray> floatLoader;
	CXMLLoader<CColladaIDREFArray> idrefLoader;
	CXMLLoader<CColladaSource> sourceLoader;
	CXMLLoader<CColladaTriangles> triangleLoader;
	CXMLLoader<CColladaVertices> vertexLoader;
	CXMLLoader<CColladaGeometry> geometryLoader;
	CXMLLoader<CColladaController> controllerLoader;
	CXMLLoader<CColladaNode> nodeLoader;
	CXMLLoader<CColladaAnimation> animLoader;
	imageLoader.Load( root, false );
	effectLoader.Load( root, false );
	materialLoader.Load( root, false );
	bindMaterialLoader.Load( root, false );
	floatLoader.Load( root, false );
	idrefLoader.Load( root, true );
	sourceLoader.Load( root, false );
	vertexLoader.Load( root, false );
	geometryLoader.Load( root, false);
	triangleLoader.Load( root, false );
	nodeLoader.Load( root, true );
	animLoader.Load( root, false );
	controllerLoader.Load( root, false );


	const std::vector<CColladaTriangles *> &triangles = CColladaTriangles::GetArrayContainer();


	for( size_t i = 0; i < triangles.size(); ++i )
	{
		std::vector<SFatVertex> vertices;
		std::vector<uint16> indices;

		triangles[i]->GetFatVertices( &vertices, &indices );
		
		re_arrange( indices, indices );
		remap_vertices( vertices, indices );
		printf( "%s %i %i\n", triangles[i]->GetEffect()->GetDiffuse()->GetFileName().ptr, vertices.size(), indices.size() );



		{
			char buff[1024];
			memset( buff, 0, sizeof( buff ) );
			sprintf( buff, "%s.%d.model", fileNameOut, i );

			model_desc_t model;

			FOpen fp( buff, false );
			if( fp.fp )
			{
				model.position.format = FLOAT;
				model.position.type = POS0;
				model.position.components = 3;
				model.position.offset = 0;
				model.position.stride = sizeof( local_vertex_t );

				model.texcoord.format = FLOAT;
				model.texcoord.type = TEX0;
				model.texcoord.components = 2;
				model.texcoord.offset = 12;
				model.texcoord.stride = sizeof( local_vertex_t );

				model.indices = TRIANGLES;
				model.vertices_num = vertices.size();
				model.indices_num = indices.size();

				fwrite( &model, sizeof( model ), 1,  fp.fp );
			}

		}

		{
			char buff[1024];
			memset( buff, 0, sizeof( buff ) );
			sprintf( buff, "%s.%d.vb", fileNameOut, i );



			FOpen fp( buff, false );
			if( fp.fp )
			{
				std::vector<local_vertex_t> lvertices;
				lvertices.resize( vertices.size() );
				for( size_t j = 0; j < vertices.size(); ++j )
				{
					lvertices[j].coo[0] = vertices[j].coo[0];
					lvertices[j].coo[1] = vertices[j].coo[1];
					lvertices[j].coo[2] = vertices[j].coo[2];
					lvertices[j].tex[0] = vertices[j].tx0[0];
					lvertices[j].tex[1] = 1.0f - vertices[j].tx0[1];


				}
				fwrite( &lvertices[0], sizeof( local_vertex_t ) * lvertices.size(), 1, fp.fp );
			}


		}


		{
			char buff[1024];
			memset( buff, 0, sizeof( buff ) );
			sprintf( buff, "%s.%d.ib", fileNameOut, i );



			FOpen fp( buff, false );
			if( fp.fp )
			{
				fwrite( &indices[0], sizeof( uint16_t ) * indices.size(), 1, fp.fp );
			}


		}

	}



	CColladaBindMaterial::ClearInner();
	CColladaController::ClearInner();
	CColladaImage::Clear();
	CColladaEffect::Clear();
	CColladaMaterial::Clear();
	CColladaBindMaterial::Clear();
	CColladaFloatArray::Clear();
	CColladaIDREFArray::Clear();
	CColladaSource::Clear();
	CColladaVertices::Clear();
	CColladaGeometry::Clear();
	CColladaController::Clear();
	CColladaNode::Clear();
	CColladaAnimation::Clear();


	xmlFreeDoc(doc);
	xmlCleanupParser();

}

int main( int argc, const char *argv[2] )
{

	if( argc < 3 )
	{
		printf( "params: infile outfile\n" );
		return -1;
	}

	convert_model( argv[2], argv[1] );

	return 0;

}

