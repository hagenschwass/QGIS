#pragma once

typedef int ConstraintArg;
typedef ConstraintArg* Constraint;

#include "Ring.h"
class CoWorker;
class SpecialWorker;
class MicroWorker;
class QSemaphore;

struct Matching
{
	int base1, /*base2, */match1/*, match2*/;
	double quality, cost;// , exitcost;
	Matching *leftback, *rightback;
};

struct Lookup
{
	Matching *matching;
	int begin, end;
};

typedef Lookup** LookupArg;
typedef LookupArg* LookupT;

struct FreeMatching
{
	int base, match;
	FreeMatching *left, *right;
	/*int leftcount, rightcount;*/
	double quality;
};

struct FreeMatchingTree
{
	FreeMatching *up, *down;
	int upcount, downcount;
	SRing2 base, match;
};

typedef FreeMatchingTree* FreeMatchingTrees;

struct MatchingResult
{
	Matching *matching, *opposite;
	double quality, cost;
};

extern inline MatchingResult initMatchingResult();
extern inline void deleteMatchingResult(MatchingResult &result);

extern inline LookupT computeInvMatching(SRing2 &base, SRing2 &match, double skiparea, MatchingResult &result, SpecialWorker *specialworker, QSemaphore *specialsemaphore, int nworkers, CoWorker** workers, QSemaphore *semaphore, volatile bool &aborted);
extern inline void deleteMatching(SRing2 &base, SRing2 &match, LookupT lookup);

extern inline Constraint createConstraint(SRing2 &base, MatchingResult &result);
extern inline void deleteConstraint(Constraint constraint);

extern inline FreeMatchingTree freeMatchingTree(Matching *up, Matching *down);
extern inline void adjustFreeMatchingTree(SRing2 &base, SRing2 &match, FreeMatchingTree &tree);
extern inline void deleteFreeMatchingTree(FreeMatchingTree &tree);

extern inline FreeMatchingTrees createFreeMatchingTrees(SRing2 &base, LookupT lookup, Constraint constraint, int nworker, MicroWorker **microworker, QSemaphore *semaphore, volatile bool &aborted);
extern inline void adjustFreeMatchingTrees(SRing2 &base, SRing2 &match, FreeMatchingTrees &trees);
extern inline void deleteFreeMatchingTrees(SRing2 &base, FreeMatchingTrees trees);