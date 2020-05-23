#pragma once

#include "Ring.h"
#include "Matching.h"

struct BaseMatch
{
	int base;
	BaseMatch *next, *prev;
};

typedef BaseMatch* SymmetryBase;

struct PointMatch
{
	BaseMatch base;
	int match;
	PointMatch *backptr;
	double quality;
	Matching *gate;
};

typedef PointMatch* SymmetryMatch;

extern inline void appendPointMatch(BaseMatch &base, PointMatch &match);
extern inline void leavePointMatch(PointMatch &match);

struct SymmetryMatches
{
	SymmetryBase base;
	SymmetryMatch match;
	Constraint constraint;
	int ie, ib;
};

extern inline void deleteSymmetryMatches(SymmetryMatches matches);

class Symmetry
{
public:
	Symmetry(SRing2 &base, SRing2 &match, MatchingResult &result, LookupT lookup);
	virtual ~Symmetry();
protected:
	SRing2 base, match;
	MatchingResult result;
	LookupT lookup;
};