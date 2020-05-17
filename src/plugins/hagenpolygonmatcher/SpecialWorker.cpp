#include "SpecialWorker.h"

SpecialWorker::SpecialWorker(QSemaphore *semaphore, volatile bool *aborted) :
	semaphore(semaphore),
	aborted(aborted)
{
	moveToThread(&thread);
	connect(this, SIGNAL(searchbestmatch(int, SRing2 *, SRing2 *, LookupArg *, double*, double*,Matching**)), this, SLOT(searchbestmatchslot(int, SRing2 *, SRing2 *, LookupArg *, double*, double*,Matching**)));
	thread.start();
}

SpecialWorker::~SpecialWorker()
{
	thread.exit();
	thread.wait();
}

/**/
void SpecialWorker::searchbestmatchslot(int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup, double *quality, double *cost, Matching **matching)
{
	for (int basei = 0; basei < base->ring.n; basei++)
	{
		Lookup *lookupbase = lookup[basei][(basei + basecut) % base->ring.n];
		Lookup *lookupbaseopposite = lookup[(basei + basecut) % base->ring.n][basei];
		for (int matchi = 0; matchi < match->ring.n; matchi++)
		{
			Lookup &lookupmatch = lookupbase[matchi];
			for (int matchj = lookupmatch.begin; matchj <= lookupmatch.end; matchj++)
			{
				double oppositequality, oppositecost;
				Lookup &lookupmatchopposite = lookupbaseopposite[matchj % match->ring.n];
				if (lookupmatchopposite.end >= match->ring.n)
				{
					if (lookupmatchopposite.begin >= match->ring.n)
					{
						int matchiup = matchi + match->ring.n;
						if (matchiup < lookupmatchopposite.begin || matchiup > lookupmatchopposite.end) continue;
						oppositequality = lookupmatchopposite.matching[matchiup - lookupmatchopposite.begin].quality;
						oppositecost = lookupmatchopposite.matching[matchiup - lookupmatchopposite.begin].cost;
					}
					else
					{
						if (matchi >= lookupmatchopposite.begin)
						{
							oppositequality = lookupmatchopposite.matching[matchi - lookupmatchopposite.begin].quality;
							oppositecost = lookupmatchopposite.matching[matchi - lookupmatchopposite.begin].cost;
						}
						else
						{
							int matchiup = matchi + match->ring.n;
							if (matchiup <= lookupmatchopposite.end)
							{
								oppositequality = lookupmatchopposite.matching[matchiup - lookupmatchopposite.begin].quality;
								oppositecost = lookupmatchopposite.matching[matchiup - lookupmatchopposite.begin].cost;
							}
							else
							{
								continue;
							}
						}
					}
				}
				else
				{
					if (matchi < lookupmatchopposite.begin || matchi > lookupmatchopposite.end) continue;
					oppositequality = lookupmatchopposite.matching[matchi - lookupmatchopposite.begin].quality;
					oppositecost = lookupmatchopposite.matching[matchi - lookupmatchopposite.begin].cost;
				}

				Matching &matchingl = lookupmatch.matching[matchj - lookupmatch.begin];

				double qualityl = matchingl.quality + oppositequality;
				double costl = matchingl.cost + oppositecost;
				if (qualityl + costl > *quality + *cost)
				{
					*quality = qualityl;
					*cost = costl;
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


/*
void SpecialWorker::searchbestmatchslot(int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup, double *quality, Matching **matching)
{
	for (int basei = 0; basei + basecut < base->ring.n; basei++)
	{
		Lookup *lookupbase = lookup[basei][basecut - 1];
		for (int matchi = 0; matchi < match->ring.n; matchi++)
		{
			Lookup &lookupmatch = lookupbase[matchi];
			for (int matchj = lookupmatch.begin; matchj <= lookupmatch.end; matchj++)
			{

				Matching &matchingl = lookupmatch.matching[matchj - lookupmatch.begin];

				double qualityl = matchingl.quality + (basecut == base->ring.n - 1 && matchi == matchj - 1 ? 0.0 : matchingl.exitcost);
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
*/