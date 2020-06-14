#pragma once

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QSemaphore>

#include "Ring.h"
#include "Matching.h"

struct PointMatch;

class MicroWorker : private QThread
{
public:
	MicroWorker(volatile bool *aborted, QSemaphore *semaphore);
	~MicroWorker() override;

	void setupAdjustInvSymmetry(int capacity, QSemaphore *semaphore);
	void loadAdjustInvSymmetry(PointMatch *pointmatch);
	void runAdjustInvSymmetry(SRing &match, SRing &matchout);
	void clearAdjustInvSymmetry();

	void setupFreeMatchingTrees(SRing2 &base, LookupT lookup, Constraint constraint, int capacity, QSemaphore *semaphore);
	void loadFreeMatchingTrees(FreeMatchingTree *tree, int basei);
	void runFreeMatchingTrees();
	void clearFreeMatchingTrees();

private:
	volatile bool *aborted;

	enum
	{
		jExit,
		jAdjustInvSymmetry,
		jFreeMatchingTrees
	} job;
	QMutex runmutex;
	QWaitCondition runwait;

	int loadindex;

	PointMatch **pointmatches;

	SRing match, matchout;

	struct FreeMatchingTreeMagazin
	{
		FreeMatchingTree *tree;
		int basei;
	} *trees;

	SRing2 base;
	LookupT lookup;
	Constraint constraint;

	QSemaphore *semaphore;

	void run() override;

};
