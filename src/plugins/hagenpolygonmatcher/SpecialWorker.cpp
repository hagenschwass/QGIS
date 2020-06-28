#include "SpecialWorker.h"

SpecialWorker::SpecialWorker(QSemaphore *semaphore, volatile bool *aborted) :
	semaphore(semaphore),
	aborted(aborted)
{
	moveToThread(&thread);
	connect(this, SIGNAL(searchbestmatch(int, SRing2 *, SRing2 *, FuncsArg *, LookupArg *, MatchingResult*)), this, SLOT(searchbestmatchslot(int, SRing2 *, SRing2 *, FuncsArg *, LookupArg *, MatchingResult*)));
	thread.start();
	thread.setPriority(QThread::Priority::LowPriority);
}

SpecialWorker::~SpecialWorker()
{
	thread.exit();
	thread.wait();
}

inline Matching* SpecialWorkergettopmatching(Matching *top)
{
	if (top->leftback != nullptr)
	{
		for (Matching *topleftback = top->leftback;; top = topleftback, topleftback = top->leftback)
		{
			if (topleftback->leftback == nullptr)
			{
				return top->rightback;
			}
		}
	}
	return nullptr;
}

inline Matching* SpecialWorkergetbottommatching(Matching *bottom)
{
	for (;; bottom = bottom->rightback)
	{
		if (bottom->rightback == nullptr) return bottom;
	}
	return nullptr;
}

/**/
void SpecialWorker::searchbestmatchslot(int basecut, SRing2 *base, SRing2 *match, FuncsArg *atans, LookupArg *lookup, MatchingResult *result)
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
				Lookup &lookupmatchopposite = lookupbaseopposite[matchj % match->ring.n];
				Matching *opposite;
				if (lookupmatchopposite.end >= match->ring.n)
				{
					if (lookupmatchopposite.begin >= match->ring.n)
					{
						int matchiup = matchi + match->ring.n;
						if (matchiup < lookupmatchopposite.begin || matchiup > lookupmatchopposite.end) continue;
						opposite = &lookupmatchopposite.matching[matchiup - lookupmatchopposite.begin];
					}
					else
					{
						if (matchi >= lookupmatchopposite.begin)
						{
							opposite = &lookupmatchopposite.matching[matchi - lookupmatchopposite.begin];
						}
						else
						{
							int matchiup = matchi + match->ring.n;
							if (matchiup <= lookupmatchopposite.end)
							{
								opposite = &lookupmatchopposite.matching[matchiup - lookupmatchopposite.begin];
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
					opposite = &lookupmatchopposite.matching[matchi - lookupmatchopposite.begin];
				}

				Matching &matchingl = lookupmatch.matching[matchj - lookupmatch.begin];
				/**/
				if (basei == match->ring.n - (matchi % match->ring.n) - 1)
				{
					{
						Matching *top = SpecialWorkergettopmatching(&matchingl);
						if (top == nullptr)
						{
							double atan2basewing = atans[basei][opposite->base1];
							double atan2matchwing = atans[basei][match->ring.n - opposite->match1 - 1];
							double delta = abs(atan2basewing - atan2matchwing);
							if (delta < MINANGLE || delta > H_2_PI - MINANGLE) 
								continue;
						}
						else
						{
							double atan2basewing = atans[basei][top->base1];
							double atan2matchwing = atans[basei][match->ring.n - top->match1 - 1];
							double delta = abs(atan2basewing - atan2matchwing);
							if (delta < MINANGLE || delta > H_2_PI - MINANGLE) 
								continue;
						}
					}
					{
						Matching *bottom = SpecialWorkergetbottommatching(opposite);
						double atan2basewing = atans[basei][bottom->base1];
						double atan2matchwing = atans[basei][match->ring.n - bottom->match1 - 1];
						double delta = abs(atan2basewing - atan2matchwing);
						if (delta < MINANGLE || delta > H_2_PI - MINANGLE) 
							continue;
					}
				}
				if (opposite->base1 == match->ring.n - (opposite->match1) - 1)
				{
					Point &peak = base->ring.ring[opposite->base1];
					{
						Matching *top = SpecialWorkergettopmatching(opposite);
						if (top == nullptr)
						{
							double atan2basewing = atans[opposite->base1][matchingl.base1];
							double atan2matchwing = atans[opposite->base1][match->ring.n - matchingl.match1 - 1];
							double delta = abs(atan2basewing - atan2matchwing);
							if (delta < MINANGLE || delta > H_2_PI - MINANGLE) 
								continue;
						}
						else
						{
							double atan2basewing = atans[opposite->base1][top->base1];
							double atan2matchwing = atans[opposite->base1][match->ring.n - top->match1 - 1];
							double delta = abs(atan2basewing - atan2matchwing);
							if (delta < MINANGLE || delta > H_2_PI - MINANGLE) 
								continue;
						}
					}
					{
						Matching *bottom = SpecialWorkergetbottommatching(&matchingl);
						double atan2basewing = atans[opposite->base1][bottom->base1];
						double atan2matchwing = atans[opposite->base1][match->ring.n - bottom->match1 - 1];
						double delta = abs(atan2basewing - atan2matchwing);
						if (delta < MINANGLE || delta > H_2_PI - MINANGLE) 
							continue;
					}
				}

				double qualityl = matchingl.quality + opposite->quality;
				double costl = matchingl.cost + opposite->cost;
				if (qualityl + costl > (*result).quality + (*result).cost)
				{
					(*result).quality = qualityl;
					(*result).cost = costl;
					(*result).matching = &matchingl;
					(*result).opposite = opposite;
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