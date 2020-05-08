#pragma once

#include "Ring.h"
#include "Matching.h"

class Symmetry
{
public:
	Symmetry(SRing2 &base, SRing2 &match, Matching *matching, LookupT lookup);
	virtual ~Symmetry();
protected:
	SRing2 base, match;
	Matching *matching;
	LookupT lookup;
};