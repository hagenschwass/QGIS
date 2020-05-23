#pragma once

typedef int ConstraintArg;
typedef ConstraintArg* Constraint;

#include "Ring.h"
class CoWorker;
class SpecialWorker;
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

struct MatchingResult
{
	Matching *matching, *opposite;
	double quality, cost;
};

extern inline MatchingResult initMatchingResult();

extern inline LookupT computeInvMatching(SRing2 &base, SRing2 &match, double skiparea, MatchingResult &result, SpecialWorker *specialworker, QSemaphore *specialsemaphore, int nworkers, CoWorker** workers, QSemaphore *semaphore, volatile bool &aborted);

extern inline Matching* getmatchingmod(LookupT lookup, SRing2 &base, SRing2 &match, int base1, int match1, int base2mod, int match2mod);
//extern inline Matching* getoppositematching(LookupT lookup, SRing2 &base, SRing2 &match, Matching *matching);

extern inline void deleteMatching(SRing2 &base, SRing2 &match, LookupT lookup);
