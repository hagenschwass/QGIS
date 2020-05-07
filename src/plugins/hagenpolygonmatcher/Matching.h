#pragma once

#include "Ring.h"
class CoWorker;
class QSemaphore;

struct Matching
{
	int base1, base2, match1, match2;
	double quality;
	Matching *leftback, *rightback;
};

struct Lookup
{
	Matching *matching;
	int begin, end;
};

typedef Lookup** LookupArg;
typedef LookupArg* LookupT;

extern inline LookupT computeInvMatching(SRing2 &base, SRing2 &match, double skiparea, int nworkers, CoWorker** workers, QSemaphore *semaphore, volatile bool &aborted);
extern inline void deleteMatching(SRing2 &base, SRing2 &match, LookupT lookup);