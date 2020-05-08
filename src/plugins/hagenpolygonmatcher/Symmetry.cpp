#include "Symmetry.h"

Symmetry::Symmetry(SRing2 &base, SRing2 &match, Matching *matching, LookupT lookup) :
	base(base), match(match), matching(matching), lookup(lookup)
{
}

Symmetry::~Symmetry()
{
}