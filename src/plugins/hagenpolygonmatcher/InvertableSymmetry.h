#pragma once

#include "Symmetry.h"

class CoWorker;

#include <vector>

extern inline SymmetryMatches computeSymmetryMatchesInv(SRing2 &base, SRing2 &match, Matching *matching, LookupT lookup);
extern inline void findSymmetryMatchesGates(SymmetryMatches &matches, SRing2 &base, SRing2 &match, LookupT lookup, int nworkers, CoWorker** workers, QSemaphore *semaphore, volatile bool &aborted);
extern inline bool computeSymmetryBeginEndInv(SRing2 &base, SymmetryMatches &matches);
extern inline void orderSymmetryMatchesInv(SRing2 &base, SymmetryMatches &matches);
extern inline PointMatch *computeBestSymmetryInv(SRing2 &base, SymmetryMatches &matches, LookupT lookup);

extern inline SRing invertableSymmetry2Ring(SRing2 &base, PointMatch *pointmatch);

class InvertableSymmetry : public Symmetry
{
public:
	InvertableSymmetry(SRing2 &base, SRing2 &match, Matching *matching, LookupT lookup);
	~InvertableSymmetry() override;

private:

public:

	void filltriangles(std::vector<Triangle> *triangles);
	void fillmatchtriangles(std::vector<Triangle> *triangles);

};
