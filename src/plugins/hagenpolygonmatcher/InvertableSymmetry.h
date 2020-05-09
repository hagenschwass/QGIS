#pragma once

#include "Symmetry.h"

#include <vector>

class InvertableSymmetry : public Symmetry
{
public:
	InvertableSymmetry(SRing2 &base, SRing2 &match, Matching *matching, LookupT lookup);

	void fillmatchlines(std::vector<Line> *lines);
	void filltriangles(std::vector<Triangle> *triangles);
	void fillmatchtriangles(std::vector<Triangle> *triangles);

};
