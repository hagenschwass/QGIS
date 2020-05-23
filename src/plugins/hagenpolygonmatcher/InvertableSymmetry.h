#pragma once

#include "Symmetry.h"

class CoWorker;
class MicroWorker;

#include <vector>

extern inline SymmetryMatches computeSymmetryMatchesInv(SRing2 &base, SRing2 &match, MatchingResult &result, LookupT lookup);
extern inline void findSymmetryMatchesGates(SymmetryMatches &matches, SRing2 &base, SRing2 &match, LookupT lookup, int nworkers, CoWorker** workers, QSemaphore *semaphore, volatile bool &aborted);
extern inline bool computeSymmetryBeginEndInv(SRing2 &base, SymmetryMatches &matches);
extern inline void orderSymmetryMatchesInv(SRing2 &base, SymmetryMatches &matches);
extern inline PointMatch *computeBestSymmetryInv(SRing2 &base, SymmetryMatches &matches, double quality);
extern inline void adjustSymmetryInv(SRing2 &match, PointMatch *pointmatch, double epsilon, int nworker, MicroWorker **microworker, QSemaphore *semaphore, volatile bool &aborted);
extern inline void updateTurnedIndexesInv(SymmetryMatches &matches);
extern inline SRing invertableSymmetry2Ring(SRing2 &match, PointMatch *pointmatch);

class InvertableSymmetry : public Symmetry
{
public:
	InvertableSymmetry(SRing2 &base, SRing2 &match, MatchingResult &result, LookupT lookup);
	~InvertableSymmetry() override;

private:

public:

	void filltriangles(std::vector<Triangle> *triangles);
	void fillmatchtriangles(std::vector<Triangle> *triangles);

};
