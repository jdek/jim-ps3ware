#pragma once
#include "../../common/XMLUtils.h"

class CColladaSource;

class CColladaVertices :  public CAutoContainer<CColladaVertices>
{
	CColladaSource *positions;
	CColladaSource *normals;

	void InitInput( const  xmlNode * input );
public:
	CColladaVertices(void);
public:
	virtual ~CColladaVertices(void);
	
	static CColladaVertices *CreateInstance( const xmlNode * node );
	static const xmlChar *GetName() { return _X"vertices"; };
	CColladaSource *GetPositions() const
	{
		return positions;
	}
	CColladaSource *GetNormals() const
	{
		return normals;
	}
};
