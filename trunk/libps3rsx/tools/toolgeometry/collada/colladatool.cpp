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
	printf( "mini cache misses per tri %f \n", misses );
	
}


#define FLOAT 0
#define SHORT 1
#define BYTE 2

struct geom_type_t
{
	uint8 format;
	uint8 ncomp;	
};

struct spu_geometry_chunk_t
{
	geom_type_t coo;
	geom_type_t col;
	geom_type_t nor;
	geom_type_t tx0;
	
};


		
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
		printf( "%s %i %i\n", triangles[i]->GetEffect()->GetDiffuse()->GetFileName().ptr, vertices.size(), indices.size() );
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

