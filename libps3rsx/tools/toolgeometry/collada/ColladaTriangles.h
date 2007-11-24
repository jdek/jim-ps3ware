#pragma once

#include "XMLUtils.h"
#include "..//Math//AABox.h"

class CColladaSource;
class CColladaEffect;



class CColladaController;

class CColladaTriangles :  public CAutoContainer<CColladaTriangles>
{
	size_t positionsIB;
	size_t normalsIB;
	size_t texcoordsIB[4];

	CColladaSource *positions;
	CColladaSource *normals;
	CColladaSource *texcoords[4];

	std::vector<uint16>      body;
	size_t                   stride;
	const CColladaEffect    *effect;

	void InitInput( const  xmlNode * input );
	CColladaController *bindController;
public:
	CAABox            box;   
	CColladaTriangles(void);
public:
	void SetBindController( CColladaController * controller );
	CColladaController *GetBindController() const
	{
		return bindController;
	}


	~CColladaTriangles(void);
	static const xmlChar *GetName() { return _X"triangles"; };
	static CColladaTriangles *CreateInstance( const xmlNode *node );
	void GetFatVertices( std::vector<SFatVertex> *vertices, std::vector<uint16> *indices );
	const CColladaEffect *GetEffect();
};
