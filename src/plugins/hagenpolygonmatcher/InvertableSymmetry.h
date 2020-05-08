#pragma once

#include "Symmetry.h"

#include <vector>

class InvertableSymmetry : public Symmetry
{
public:
	InvertableSymmetry(SRing2 &base, SRing2 &match, Matching *matching, LookupT lookup);

	void fillmatchlines(std::vector<Line> *lines);

};
