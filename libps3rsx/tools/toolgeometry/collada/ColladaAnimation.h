#pragma once
#include "XMLUtils.h"

class CColladaSource;

class CColladaAnimation : public CAutoContainer<CColladaAnimation>
{
	CColladaSource *matrixAnim;
	void InitInput( const  xmlNode * input );
public:
	CColladaAnimation(void);
public:
	virtual ~CColladaAnimation(void);
  static const xmlChar *GetName() { return _X"animation"; };
	static CColladaAnimation *CreateInstance( const xmlNode *node );
	static void Freeeezzzze( const xmlChar *animName );
};
