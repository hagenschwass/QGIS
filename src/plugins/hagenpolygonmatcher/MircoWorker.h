#pragma once

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QSemaphore>

#include "Ring.h"

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

private:
	volatile bool *aborted;

	enum
	{
		jExit,
		jAdjustInvSymmetry
	} job;
	QMutex runmutex;
	QWaitCondition runwait;

	int loadindex;

	PointMatch **pointmatches;

	SRing match, matchout;

	QSemaphore *semaphore;

	void run() override;
};
