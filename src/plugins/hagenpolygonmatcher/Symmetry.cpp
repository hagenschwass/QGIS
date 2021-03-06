#include "Symmetry.h"

inline void appendPointMatch(BaseMatch &base, PointMatch &match)
{
	base.prev->next = &match.base;
	match.base.prev = base.prev;
	base.prev = &match.base;
	match.base.next = &base;
}

inline void leavePointMatch(PointMatch &match)
{
	BaseMatch &base = match.base;
	base.prev->next = base.next;
	base.next->prev = base.prev;
}

inline void deleteSymmetryMatches(SymmetryMatches matches)
{
	delete[] matches.base;
	delete[] matches.match;
}

Symmetry::Symmetry(SRing2 &base, SRing2 &match, MatchingResult &result, LookupT lookup) :
	base(base), match(match), result(result), lookup(lookup)
{
}

Symmetry::~Symmetry()
{
}