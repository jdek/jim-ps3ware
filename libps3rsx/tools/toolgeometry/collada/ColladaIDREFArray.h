#pragma once

#include "XMLUtils.h"

class ÑColladaIDREFArray : public CAutoContainer<ÑColladaIDREFArray>
{
	std::vector<SStrng>  body;
	std::vector<xmlChar> content;
public:
	ÑColladaIDREFArray(void);
public:
	const std::vector<SStrng> &GetArray() const;
	virtual ~ÑColladaIDREFArray(void);
	void BuildContent(){};
	static const xmlChar *GetName() { return _X"IDREF_array"; };
	static ÑColladaIDREFArray *CreateInstance( xmlNode *node );
};
