#pragma once

#include "Ring.h"
class CoWorker;
class QSemaphore;

struct Matching
{
	double quality;
	Matching *leftback, *rightback;
};

typedef Matching**** Lookup;

extern inline Lookup computeInvMatching(SRing2 &base, SRing2 &match, int nworkers, CoWorker** workers, QSemaphore *semaphore, volatile bool &aborted);
extern inline void deleteMatching(SRing2 &base, SRing2 &match, Lookup lookup);