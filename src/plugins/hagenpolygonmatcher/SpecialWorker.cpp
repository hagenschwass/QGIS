#include "SpecialWorker.h"

SpecialWorker::SpecialWorker(QSemaphore *semaphore, volatile bool *aborted) :
	semaphore(semaphore),
	aborted(aborted)
{
	moveToThread(&thread);
	connect(this, SIGNAL(searchbestmatch(int, SRing2 *, SRing2 *, LookupArg *, double*,Matching**)), this, SLOT(searchbestmatchslot(int, SRing2 *, SRing2 *, LookupArg *, double*,Matching**)));
	thread.start();
}

SpecialWorker::~SpecialWorker()
{
	thread.exit();
	thread.wait();
}

void SpecialWorker::searchbestmatchslot(int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup, double *quality, Matching **matching)
{
	for (int basei = 0; basei < base->ring.n; basei++)
	{
		Lookup *lookupbase = lookup[basei][(basei + basecut) % base->ring.n];
		for (int matchi = 0; matchi < match->ring.n; matchi++)
		{
			Lookup &lookupmatch = lookupbase[matchi];
			for (int matchj = lookupmatch.begin; matchj <= lookupmatch.end; matchj++)
			{
				Matching &matchingl = lookupmatch.matching[matchj - lookupmatch.begin];
				double qualityl = matchingl.quality + matchingl.exitcost;
				if (qualityl > *quality)
				{
					*quality = qualityl;
					*matching = &matchingl;
				}
				if (*aborted)
				{
					break;
				}
			}
			if (*aborted)
			{
				break;
			}
		}
		if (*aborted)
		{
			break;
		}
	}
	semaphore->release();
}
