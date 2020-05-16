#include "CoWorker.h"
#include <cmath>

CoWorker::CoWorker(QSemaphore *semaphore, volatile bool *aborted) :
	semaphore(semaphore),
	aborted(aborted)
{
	moveToThread(&thread);
	connect(this, SIGNAL(initlookupinvforbase(int, SRing2 *, SRing2 *, LookupArg *, double)), this, SLOT(initlookupinvforbaseslot(int, SRing2 *, SRing2 *, LookupArg *, double)));
	connect(this, SIGNAL(initshortcutsforbase(int, SRing2 *, SRing2 *, LookupArg *)), this, SLOT(initshortcutsforbaseslot(int, SRing2 *, SRing2 *, LookupArg *)));
	connect(this, SIGNAL(initshortcutsformatch(int, SRing2 *, SRing2 *, LookupArg *)), this, SLOT(initshortcutsformatchslot(int, SRing2 *, SRing2 *, LookupArg *)));
	connect(this, SIGNAL(computeshortcutsinv(int, int, SRing2 *, SRing2 *, LookupArg *)), this, SLOT(computeshortcutsinvslot(int, int, SRing2 *, SRing2 *, LookupArg *)));
	connect(this, SIGNAL(updateexitcosts(int , SRing2 *, SRing2 *, LookupArg *)), this, SLOT(updateexitcostsslot(int , SRing2 *, SRing2 *, LookupArg *)));
	connect(this, SIGNAL(matchinv(/**/int , int , SRing2 *, SRing2 *, LookupArg *)), this, SLOT(matchinvslot(/**/int , int , SRing2 *, SRing2 *, LookupArg *)));
	thread.start();
}

CoWorker::~CoWorker()
{
	thread.exit();
	thread.wait();
}

/**/
void CoWorker::initlookupinvforbaseslot(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup, double skiparea)
{
	if (*aborted == false)
	{
		Matching *tempmatching = new Matching[match->ring.n];
		for (int i = 0; i < match->ring.n; i++)
		{
			tempmatching[i].base1 = basei;
			tempmatching[i].leftback = nullptr;
			tempmatching[i].rightback = nullptr;
		}

		LookupArg &lookupbase = lookup[basei];
		lookupbase = new Lookup*[base->ring.n];
		lookupbase[basei] = nullptr;

		{
			Lookup *&lookupmatch = lookupbase[(basei + 1) % base->ring.n];
			lookupmatch = new Lookup[match->ring.n];

			for (int matchi = 0; matchi < match->ring.n; matchi++)
			{
				tempmatching[0].quality = 0.0;
				tempmatching[0].match1 = matchi;
				tempmatching[0].match2 = matchi + 1;
				tempmatching[0].base2 = basei + 1;
				Lookup &lookupend = lookupmatch[matchi];
				lookupend.begin = matchi + 1;

				double matchareaabs = 0.0;
				Point &matchp1 = match->ring.ring[matchi];
				for (int matchj = matchi + 1, matchk = matchi + 2; ; matchj = matchk, matchk++)
				{
					Point &matchp2 = match->ring.ring[matchj % match->ring.n], &matchp3 = match->ring.ring[matchk % match->ring.n];
					double &matchp1x = matchp1.x, &matchp1y = matchp1.y, &matchp2x = matchp2.x, &matchp2y = matchp2.y, &matchp3x = matchp3.x, &matchp3y = matchp3.y;
					double matchp1yp2y = matchp1y - matchp2y, matchp1xp3x = matchp1x - matchp3x, matchp1yp3y = matchp1y - matchp3y, matchp2xp1x = matchp2x - matchp1x;
					double matchareal = -.5 * (matchp1yp2y * matchp1xp3x + matchp1yp3y * matchp2xp1x);
					matchareaabs += abs(matchareal);
					if (matchareaabs > skiparea)
					{
						lookupend.matching = new Matching[matchk - matchi - 1];
						memcpy_s(lookupend.matching, sizeof(Matching) * (matchk - matchi - 1), tempmatching, sizeof(Matching) * (matchk - matchi - 1));
						lookupend.end = matchk - 1;
						break;
					}
					//tempmatching[matchk - matchi - 1].quality = -2.0 * matchareaabs;
					tempmatching[matchk - matchi - 1].match1 = matchi;
					tempmatching[matchk - matchi - 1].match2 = matchk;
					tempmatching[matchk - matchi - 1].base2 = basei + 1;
					if (matchk == matchi + match->ring.n - 1)
					{
						lookupend.matching = new Matching[matchk - matchi];
						memcpy_s(lookupend.matching, sizeof(Matching) * (matchk - matchi), tempmatching, sizeof(Matching) * (matchk - matchi));
						lookupend.end = matchk;
						break;
					}
				}




				if (*aborted)
				{
					for (matchi++; matchi < match->ring.n; matchi++)
					{
						lookupmatch[matchi].matching = nullptr;
					}
					break;
				}
			}

		}


		if (*aborted == false)
		{
			double baseareaabs = 0.0;
			Point &basep1 = base->ring.ring[basei];
			for (int basej = basei + 1, basek = basei + 2; basek < basei + base->ring.n; basej = basek++)
			{
				Point &basep2 = base->ring.ring[basej % base->ring.n], &basep3 = base->ring.ring[basek % base->ring.n];
				double &basep1x = basep1.x, &basep1y = basep1.y, &basep2x = basep2.x, &basep2y = basep2.y, &basep3x = basep3.x, &basep3y = basep3.y;
				double basep1yp2y = basep1y - basep2y, basep1xp3x = basep1x - basep3x, basep1yp3y = basep1y - basep3y, basep2xp1x = basep2x - basep1x;
				double baseareal = .5 * (basep1yp2y * basep1xp3x + basep1yp3y * basep2xp1x);
				baseareaabs += abs(baseareal);

				Lookup *&lookupmatch = lookupbase[basek % base->ring.n];
				lookupmatch = new Lookup[match->ring.n];

				for (int matchi = 0; matchi < match->ring.n; matchi++)
				{
					//tempmatching[0].quality = -2.0 * baseareaabs;
					tempmatching[0].match1 = matchi;
					tempmatching[0].match2 = matchi + 1;
					tempmatching[0].base2 = basek;
					int begin = baseareaabs > skiparea ? -1 : matchi + 1;
					double matchareaabs = 0.0;
					Point &matchp1 = match->ring.ring[matchi];
					for (int matchj = matchi + 1, matchk = matchi + 2; ; matchj = matchk, matchk++)
					{
						Point &matchp2 = match->ring.ring[matchj % match->ring.n], &matchp3 = match->ring.ring[matchk % match->ring.n];
						double &matchp1x = matchp1.x, &matchp1y = matchp1.y, &matchp2x = matchp2.x, &matchp2y = matchp2.y, &matchp3x = matchp3.x, &matchp3y = matchp3.y;
						double matchp1yp2y = matchp1y - matchp2y, matchp1xp3x = matchp1x - matchp3x, matchp1yp3y = matchp1y - matchp3y, matchp2xp1x = matchp2x - matchp1x;
						double matchareal = -.5 * (matchp1yp2y * matchp1xp3x + matchp1yp3y * matchp2xp1x);
						matchareaabs += abs(matchareal);
						if (begin < 0 && baseareaabs - matchareaabs < skiparea)
						{
							begin = matchk;
						}
						if (begin > -1 && (matchareaabs - baseareaabs) > skiparea)
						{
							Lookup &lookupend = lookupmatch[matchi];
							lookupend.begin = begin;


							lookupend.matching = new Matching[matchk - begin];
							memcpy_s(lookupend.matching, sizeof(Matching) * (matchk - begin), &tempmatching[begin - matchi - 1], sizeof(Matching) * (matchk - begin));
							lookupend.end = matchk - 1;
							break;
						}
						//tempmatching[matchk - matchi - 1].quality = -2.0 * matchareaabs - 2.0 * baseareaabs;
						tempmatching[matchk - matchi - 1].match1 = matchi;
						tempmatching[matchk - matchi - 1].match2 = matchk;
						tempmatching[matchk - matchi - 1].base2 = basek;
						if (matchk == matchi + match->ring.n - 1)
						{
							Lookup &lookupend = lookupmatch[matchi];
							if (begin < 0)
							{
								lookupend.begin = 0;
								lookupend.end = -1;
								lookupend.matching = nullptr;
							}
							else
							{
								lookupend.begin = begin;
								lookupend.matching = new Matching[matchk - begin + 1];
								memcpy_s(lookupend.matching, sizeof(Matching) * (matchk - begin + 1), &tempmatching[begin - matchi - 1], sizeof(Matching) * (matchk - begin + 1));
								lookupend.end = matchk;
							}
							break;
						}
					}



					if (*aborted)
					{
						for (matchi++; matchi < match->ring.n; matchi++)
						{
							lookupmatch[matchi].matching = nullptr;
						}
						break;
					}

				}



				if (*aborted)
				{
					for (basek++; basek < basei + base->ring.n; basek++)
					{
						lookupbase[basek % base->ring.n] = nullptr;
					}
				}
			}//basek

		}


		delete[] tempmatching;
	}


	semaphore->release();
}


/*
void CoWorker::initlookupinvforbaseslot(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup, double skiparea)
{
	if (*aborted == false)
	{
		Matching *tempmatching = new Matching[match->ring.n];
		for (int i = 0; i < match->ring.n; i++)
		{
			tempmatching[i].base1 = basei;
			tempmatching[i].leftback = nullptr;
			tempmatching[i].rightback = nullptr;
			tempmatching[i].exitcost = -DBL_MAX;
		}

		LookupArg lookupbase = lookup[basei];

		{
			Lookup *lookupmatch = lookupbase[0];

			Point &basep1 = base->ring.ring[basei], &basep2 = base->ring.ring[basei + 1];
			double basedx = basep2.x - basep1.x, basedy = basep2.y - basep1.y;
			double basesq = basedx * basedx + basedy * basedy;

			for (int matchi = 0; matchi < match->ring.n; matchi++)
			{
				tempmatching[0].quality = 0.0;
				tempmatching[0].match1 = matchi;
				tempmatching[0].match2 = matchi + 1;
				tempmatching[0].base2 = basei + 1;

				Lookup &lookupend = lookupmatch[matchi];
				lookupend.begin = matchi + 1;

				double matchareaabs = 0.0;
				Point &matchp1 = match->ring.ring[matchi];
				for (int matchj = matchi + 1, matchk = matchi + 2; ; matchj = matchk, matchk++)
				{
					Point &matchp2 = match->ring.ring[matchj % match->ring.n], &matchp3 = match->ring.ring[matchk % match->ring.n];
					double &matchp1x = matchp1.x, &matchp1y = matchp1.y, &matchp2x = matchp2.x, &matchp2y = matchp2.y, &matchp3x = matchp3.x, &matchp3y = matchp3.y;
					double matchp1yp2y = matchp1y - matchp2y, matchp1xp3x = matchp1x - matchp3x, matchp1yp3y = matchp1y - matchp3y, matchp2xp1x = matchp2x - matchp1x;
					double matchareal = -.5 * (matchp1yp2y * matchp1xp3x + matchp1yp3y * matchp2xp1x);
					matchareaabs += abs(matchareal);
					if (matchareaabs > skiparea)
					{
						lookupend.matching = new Matching[matchk - matchi - 1];
						memcpy_s(lookupend.matching, sizeof(Matching) * (matchk - matchi - 1), tempmatching, sizeof(Matching) * (matchk - matchi - 1));
						lookupend.end = matchk - 1;
						break;
					}
					double matchdx = matchp3.x - matchp1.x, matchdy = matchp3.y - matchp1.y;
					double matchsq = matchdx * matchdx + matchdy * matchdy;
					double &quality = tempmatching[matchk - matchi - 1].quality;
					quality = 0.0;
					for (int matchpeek = matchi + 1; matchpeek < matchk; matchpeek++)
					{
						Point &matchpeekp = match->ring.ring[matchpeek % match->ring.n];
						double matchleftdx = matchpeekp.x - matchp1.x, matchleftdy = matchpeekp.y - matchp1.y;
						double matchleftsq = matchleftdx * matchleftdx + matchleftdy * matchleftdy;
						double matchrightdx = matchp3.x - matchpeekp.x, matchrightdy = matchp3.y - matchpeekp.y;
						double matchrightsq = matchrightdx * matchrightdx + matchrightdy * matchrightdy;
						double cost = -2.0 * (matchleftsq + matchrightsq);
						if (cost < quality) quality = cost;
					}
					quality -= matchsq > basesq ? basesq - matchsq : matchsq - basesq;
					tempmatching[matchk - matchi - 1].match1 = matchi;
					tempmatching[matchk - matchi - 1].match2 = matchk;
					tempmatching[matchk - matchi - 1].base2 = basei + 1;
					if (matchk == matchi + match->ring.n - 1)
					{
						lookupend.matching = new Matching[matchk - matchi];
						memcpy_s(lookupend.matching, sizeof(Matching) * (matchk - matchi), tempmatching, sizeof(Matching) * (matchk - matchi));
						lookupend.end = matchk;
						break;
					}
				}


				if (*aborted)
				{
					for (matchi++; matchi < match->ring.n; matchi++)
					{
						lookupmatch[matchi].matching = nullptr;
					}
					break;
				}
			}
		}

		if (*aborted == false)
		{
			double baseareaabs = 0.0;
			Point &basep1 = base->ring.ring[basei];

			for (int basej = basei + 1, basek = basei + 2; basek < base->ring.n; basej = basek++)
			{
				Point &basep2 = base->ring.ring[basej], &basep3 = base->ring.ring[basek];
				double &basep1x = basep1.x, &basep1y = basep1.y, &basep2x = basep2.x, &basep2y = basep2.y, &basep3x = basep3.x, &basep3y = basep3.y;
				double basep1yp2y = basep1y - basep2y, basep1xp3x = basep1x - basep3x, basep1yp3y = basep1y - basep3y, basep2xp1x = basep2x - basep1x;
				double baseareal = .5 * (basep1yp2y * basep1xp3x + basep1yp3y * basep2xp1x);
				baseareaabs += abs(baseareal);
				double basedx = basep3.x - basep1.x, basedy = basep3.y - basep1.y;
				double basesq = basedx * basedx + basedy * basedy;

				Lookup *lookupmatch = lookupbase[basek - basei - 1];

				for (int matchi = 0; matchi < match->ring.n; matchi++)
				{
					Point &matchp1 = match->ring.ring[matchi], &matchendp = match->ring.ring[(matchi + 1) % match->ring.n];
					double matchdx = matchendp.x - matchp1.x, matchdy = matchendp.y - matchp1.y;
					double matchsq = matchdx * matchdx + matchdy * matchdy;
					double &quality = tempmatching[0].quality;
					quality = 0.0;
					for (int basepeek = basei + 1; basepeek < basek; basepeek++)
					{
						Point &basepeekp = base->ring.ring[basepeek % base->ring.n];
						double baseleftdx = basepeekp.x - basep1.x, baseleftdy = basepeekp.y - basep1.y;
						double baseleftsq = baseleftdx * baseleftdx + baseleftdy * baseleftdy;
						double baserightdx = basep3.x - basepeekp.x, baserightdy = basep3.y - basepeekp.y;
						double baserightsq = baserightdx * baserightdx + baserightdy * baserightdy;
						double cost = -2.0 * (baseleftsq + baserightsq);
						if (cost < quality) quality = cost;
					}
					quality -= matchsq > basesq ? basesq - matchsq : matchsq - basesq;
					tempmatching[0].match1 = matchi;
					tempmatching[0].match2 = matchi + 1;
					tempmatching[0].base2 = basek;


					int begin = baseareaabs > skiparea ? -1 : matchi + 1;
					double matchareaabs = 0.0;
					for (int matchj = matchi + 1, matchk = matchi + 2; ; matchj = matchk, matchk++)
					{
						Point &matchp2 = match->ring.ring[matchj % match->ring.n], &matchp3 = match->ring.ring[matchk % match->ring.n];
						double &matchp1x = matchp1.x, &matchp1y = matchp1.y, &matchp2x = matchp2.x, &matchp2y = matchp2.y, &matchp3x = matchp3.x, &matchp3y = matchp3.y;
						double matchp1yp2y = matchp1y - matchp2y, matchp1xp3x = matchp1x - matchp3x, matchp1yp3y = matchp1y - matchp3y, matchp2xp1x = matchp2x - matchp1x;
						double matchareal = -.5 * (matchp1yp2y * matchp1xp3x + matchp1yp3y * matchp2xp1x);
						matchareaabs += abs(matchareal);
						if (begin < 0 && baseareaabs - matchareaabs < skiparea)
						{
							begin = matchk;
						}
						if (begin > -1 && (matchareaabs - baseareaabs) > skiparea)
						{
							Lookup &lookupend = lookupmatch[matchi];
							lookupend.begin = begin;


							lookupend.matching = new Matching[matchk - begin];
							memcpy_s(lookupend.matching, sizeof(Matching) * (matchk - begin), &tempmatching[begin - matchi - 1], sizeof(Matching) * (matchk - begin));
							lookupend.end = matchk - 1;
							break;
						}
						tempmatching[matchk - matchi - 1].quality = -DBL_MAX;
						tempmatching[matchk - matchi - 1].match1 = matchi;
						tempmatching[matchk - matchi - 1].match2 = matchk;
						tempmatching[matchk - matchi - 1].base2 = basek;
						if (matchk == matchi + match->ring.n - 1)
						{
							Lookup &lookupend = lookupmatch[matchi];
							if (begin < 0)
							{
								lookupend.begin = 0;
								lookupend.end = -1;
								lookupend.matching = nullptr;
							}
							else
							{
								lookupend.begin = begin;
								lookupend.matching = new Matching[matchk - begin + 1];
								memcpy_s(lookupend.matching, sizeof(Matching) * (matchk - begin + 1), &tempmatching[begin - matchi - 1], sizeof(Matching) * (matchk - begin + 1));
								lookupend.end = matchk;
							}
							break;
						}
					}



					if (*aborted)
					{
						for (matchi++; matchi < match->ring.n; matchi++)
						{
							lookupmatch[matchi].matching = nullptr;
						}
						break;
					}

				}



				if (*aborted)
				{
					for (basek++; basek < basei + base->ring.n; basek++)
					{
						Lookup *lookupmatch = lookupbase[basek - basei - 1];
						for (int matchi = 0; matchi < match->ring.n; matchi++)
						{
							lookupmatch[matchi].matching = nullptr;
						}
					}
				}
			}//basek

		}


		delete[] tempmatching;
	}


	semaphore->release();
}
*/

void CoWorker::computeshortcutsinvslot(int basei, int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup)
{

	Point &pbasei = base->ring.ring[basei], &pbasej = base->ring.ring[(basei + basecut) % base->ring.n];



	for (int matchcut = 2; matchcut < match->ring.n; matchcut++)
	{
		for (int matchi = 0; matchi < match->ring.n; matchi++)
		{
			Lookup &lookupl = lookup[basei][(basei + basecut) % base->ring.n][matchi];
			if (lookupl.begin > matchi + matchcut || lookupl.end < matchi + matchcut) continue;

			Point &pmatchi = match->ring.ring[matchi], &pmatchj = match->ring.ring[(matchi + matchcut) % match->ring.n];

			Matching &gate = lookupl.matching[matchi + matchcut - lookupl.begin];
			double &quality = gate.quality;
			quality = -DBL_MAX;

			for (int basepeek = basei + 1; basepeek < basei + basecut; basepeek++)
			{
				Point &pbasepeek = base->ring.ring[basepeek % base->ring.n];
				double &basep1x = pbasei.x, &basep1y = pbasei.y, &basep2x = pbasepeek.x, &basep2y = pbasepeek.y, &basep3x = pbasej.x, &basep3y = pbasej.y;
				double basep1yp2y = basep1y - basep2y, basep1xp3x = basep1x - basep3x, basep1yp3y = basep1y - basep3y, basep2xp1x = basep2x - basep1x;
				double basearealabs = abs(basep1yp2y * basep1xp3x + basep1yp3y * basep2xp1x);


				Lookup &lookupleft = lookup[basei][basepeek % base->ring.n][matchi];
				int matchpeekend = std::min(lookupleft.end, matchi + matchcut - 1);
				for (int matchpeek = lookupleft.begin; matchpeek <= matchpeekend; matchpeek++)
				{
					Lookup &lookupright = lookup[basepeek % base->ring.n][(basei + basecut) % base->ring.n][matchpeek % match->ring.n];
					int matchindexright = matchpeek >= match->ring.n ? matchi + matchcut - match->ring.n : matchi + matchcut;
					if (matchindexright < lookupright.begin || matchindexright > lookupright.end) continue;


					Matching &right = lookupright.matching[matchindexright - lookupright.begin];
					Matching &left = lookupleft.matching[matchpeek - lookupleft.begin];

					Point &pmatchpeek = match->ring.ring[matchpeek % match->ring.n];
					double &matchp1x = pmatchi.x, &matchp1y = pmatchi.y, &matchp2x = pmatchpeek.x, &matchp2y = pmatchpeek.y, &matchp3x = pmatchj.x, &matchp3y = pmatchj.y;
					double matchp1yp2y = matchp1y - matchp2y, matchp1xp3x = matchp1x - matchp3x, matchp1yp3y = matchp1y - matchp3y, matchp2xp1x = matchp2x - matchp1x;
					double matchareal = matchp1yp2y * matchp1xp3x + matchp1yp3y * matchp2xp1x;


					double dynq = left.quality + right.quality - abs(matchareal) - basearealabs;
					if (dynq > quality)
					{
						quality = dynq;
					}

					if (*aborted)
					{
						break;
					}
				}//matchpeek


				if (*aborted)
				{
					break;
				}
			}//basepeek

			if (*aborted)
			{
				break;
			}
		}//matchi

		if (*aborted)
		{
			break;
		}
	}//matchcut


	semaphore->release();
}

void CoWorker::initshortcutsforbaseslot(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup)
{
	if (*aborted == false)
	{
		Lookup *lookupbase = lookup[basei][(basei + 1) % base->ring.n];

		for (int matchcut = 2; matchcut < match->ring.n; matchcut++)
		{
			for (int matchi = 0; matchi < match->ring.n; matchi++)
			{
				Lookup &lookupmatch = lookupbase[matchi];

				int matchj = matchi + matchcut;
				if (matchj > lookupmatch.end) continue;

				Point &matchp1 = match->ring.ring[matchi], &matchp3 = match->ring.ring[matchj % match->ring.n];

				Matching &gate = lookupmatch.matching[matchj - lookupmatch.begin];
				double &quality = gate.quality;
				quality = -DBL_MAX;

				for (int matchpeek = matchi + 1; matchpeek < matchj; matchpeek++)
				{
					Lookup &lookupmatchright = lookupbase[matchpeek % match->ring.n];
					int iright;
					if (matchpeek < match->ring.n)
					{
						if (matchj < lookupmatchright.begin || matchj > lookupmatchright.end) continue;
						iright = matchj;
					}
					else
					{
						int matchjdown = matchj - match->ring.n;
						if (matchjdown < lookupmatchright.begin || matchjdown > lookupmatchright.end) continue;
						iright = matchjdown;
					}

					Matching &right = lookupmatchright.matching[iright - lookupmatchright.begin];
					Matching &left = lookupmatch.matching[matchpeek - lookupmatch.begin];

					Point &matchp2 = match->ring.ring[matchpeek % match->ring.n];
					double &matchp1x = matchp1.x, &matchp1y = matchp1.y, &matchp2x = matchp2.x, &matchp2y = matchp2.y, &matchp3x = matchp3.x, &matchp3y = matchp3.y;
					double matchp1yp2y = matchp1y - matchp2y, matchp1xp3x = matchp1x - matchp3x, matchp1yp3y = matchp1y - matchp3y, matchp2xp1x = matchp2x - matchp1x;
					double matchareal = matchp1yp2y * matchp1xp3x + matchp1yp3y * matchp2xp1x;

					double dynq = right.quality + left.quality - abs(matchareal);
					if (dynq > quality) quality = dynq;
				}


				if (*aborted)
				{
					break;
				}
			}//matchi

			if (*aborted)
			{
				break;
			}
		}//matchcut

	}

	semaphore->release();
}

void CoWorker::initshortcutsformatchslot(int matchi, SRing2 *base, SRing2 *match, LookupArg *lookup)
{
	if (*aborted == false)
	{
		int matchj = matchi + 1;
		for (int basecut = 2; basecut < base->ring.n; basecut++)
		{
			for (int basei = 0; basei < base->ring.n; basei++)
			{
				Lookup &lookupmatch = lookup[basei][(basei + basecut) % base->ring.n][matchi];
				if (matchj < lookupmatch.begin || matchj > lookupmatch.end) continue;

				Point &basep1 = base->ring.ring[basei], &basep3 = base->ring.ring[(basei + basecut) % base->ring.n];

				Matching &gate = lookupmatch.matching[matchj - lookupmatch.begin];
				double &quality = gate.quality;
				quality = -DBL_MAX;

				for (int basepeek = basei + 1; basepeek < basei + basecut; basepeek++)
				{
					Lookup &lookupleft = lookup[basei][basepeek % base->ring.n][matchi];
					if (lookupleft.begin > matchj || lookupleft.end < matchj) continue;

					Lookup &lookupright = lookup[basepeek % base->ring.n][(basei + basecut) % base->ring.n][matchi];
					if (lookupright.begin > matchj || lookupleft.end < matchj) continue;

					Matching &left = lookupleft.matching[matchj - lookupleft.begin];
					Matching &right = lookupright.matching[matchj - lookupright.begin];

					Point &basep2 = base->ring.ring[basepeek % base->ring.n];
					double &basep1x = basep1.x, &basep1y = basep1.y, &basep2x = basep2.x, &basep2y = basep2.y, &basep3x = basep3.x, &basep3y = basep3.y;
					double basep1yp2y = basep1y - basep2y, basep1xp3x = basep1x - basep3x, basep1yp3y = basep1y - basep3y, basep2xp1x = basep2x - basep1x;
					double baseareal = basep1yp2y * basep1xp3x + basep1yp3y * basep2xp1x;

					double dynq = left.quality + right.quality - abs(baseareal);
					if (dynq > quality) quality = dynq;
				}

				if (*aborted)
				{
					break;
				}
			}//basei

			if (*aborted)
			{
				break;
			}
		}//basecut
	}

	semaphore->release();
}

void CoWorker::updateexitcostsslot(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup)
{
	if (*aborted == false)
	{
		Point &pbasei = base->ring.ring[basei];
		LookupArg &lookup1 = lookup[basei];


		{
			int basej = basei + 1;
			Point &pbasej = base->ring.ring[basej];
			double basedx = pbasej.x - pbasei.x, basedy = pbasej.y - pbasei.y;
			double basesq = basedx * basedx + basedy * basedy;

			Lookup *lookup2 = lookup1[0];

			for (int matchi = 0; matchi < match->ring.n; matchi++)
			{
				Lookup &lookup3 = lookup2[matchi];
				Point &pmatchi = match->ring.ring[matchi];

				for (int matchj = lookup3.begin; matchj <= lookup3.end; matchj++)
				{

					Point &pmatchj = match->ring.ring[matchj % match->ring.n];
					double matchdx = pmatchj.x - pmatchi.x, matchdy = pmatchj.y - pmatchi.y;
					double matchsq = matchdx * matchdx + matchdy * matchdy;

					Matching &matching = lookup3.matching[matchj - lookup3.begin];
					//double &exitcost = matching.exitcost;
					//exitcost = 0.0;

					for (int matchpeek = matchi + 1; matchpeek < matchj; matchpeek++)
					{
						Point &pmatchpeek = match->ring.ring[matchpeek % match->ring.n];
						double matchleftdx = pmatchpeek.x - pmatchi.x, matchleftdy = pmatchpeek.y - pmatchi.y;
						double matchleftsq = matchleftdx * matchleftdx + matchleftdy * matchleftdy;
						double matchrightdx = pmatchj.x - pmatchpeek.x, matchrightdy = pmatchj.y - pmatchpeek.y;
						double matchrightsq = matchrightdx * matchrightdx + matchrightdy * matchrightdy;
						double cost = -2.0 * (matchleftsq + matchrightsq);
						//if (cost < exitcost) exitcost = cost;
					}

					//exitcost += basesq < matchsq ? basesq - matchsq : matchsq - basesq;


					if (*aborted)
					{
						break;
					}
				}//matchj


				if (*aborted)
				{
					break;
				}
			}//matchi
		}

		if (*aborted == false)
		{
			for (int basej = basei + 2; basej < base->ring.n; basej++)
			{
				Point &pbasej = base->ring.ring[basej];
				double basedx = pbasej.x - pbasei.x, basedy = pbasej.y - pbasei.y;
				double basesq = basedx * basedx + basedy * basedy;

				Lookup *lookup2 = lookup1[basej - basei - 1];

				for (int matchi = 0; matchi < match->ring.n; matchi++)
				{
					Lookup &lookup3 = lookup2[matchi];
					int matchj = matchi + match->ring.n - 1;
					if (matchj  < lookup3.begin || matchj > lookup3.end) continue;

					Point &pmatchi = match->ring.ring[matchi], &pmatchj = match->ring.ring[matchj % match->ring.n];
					double matchdx = pmatchj.x - pmatchi.x, matchdy = pmatchj.y - pmatchi.y;
					double matchsq = matchdx * matchdx + matchdy * matchdy;

					Matching &matching = lookup3.matching[matchj - lookup3.begin];
					//double &exitcost = matching.exitcost;
					//exitcost = 0.0;

					for (int basepeek = basej + 1; basepeek < base->ring.n + basei - 1; basepeek++)
					{
						Point &pbasepeek = base->ring.ring[basepeek % base->ring.n];
						double baseleftdx = pbasepeek.x - pbasej.x, baseleftdy = pbasepeek.y - pbasej.y;
						double baseleftsq = baseleftdx * baseleftdx + baseleftdy * baseleftdy;
						double baserightdx = pbasei.y - pbasepeek.x, baserightdy = pbasei.y - pbasepeek.y;
						double baserightsq = baserightdx * baserightdx + baserightdy * baserightdy;
						double basecost = -2.0 * (baseleftsq + baserightsq);
						//if (basecost < exitcost) exitcost = basecost;

					}//basepeek

					//exitcost += basesq < matchsq ? basesq - matchsq : matchsq - basesq;


					if (*aborted)
					{
						break;
					}
				}//matchi




				if (*aborted)
				{
					break;
				}
			}//basej
		}
	}
	semaphore->release();
}

#define RELUCTANCE		.5

void CoWorker::matchinvslot(/**/int basei, int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup)
{

	Point &pbasei = base->ring.ring[basei], &pbasej = base->ring.ring[(basei + basecut) % base->ring.n];
	double basedx = pbasej.x - pbasei.x, basedy = pbasej.y - pbasei.y;
	double baselength = sqrt(basedx * basedx + basedy * basedy);

	if (baselength >= 1e-13)
	{
		double basedxn = basedx / baselength, basedyn = basedy / baselength;
		for (int basepeek = basei + 1; basepeek < basei + basecut; basepeek++)
		{
			Point &pbasepeek = base->ring.ring[basepeek % base->ring.n];
			double basep11yp21yleft = pbasei.y - pbasepeek.y, basep21xp11xleft = pbasepeek.x - pbasei.x;
			double baseleft = (basep21xp11xleft * basedxn - basep11yp21yleft * basedyn);
			double basep11yp21yright = pbasej.y - pbasepeek.y, basep21xp11xright = pbasepeek.x - pbasej.x;
			double baseright = (basep21xp11xright * basedxn - basep11yp21yright * basedyn);
			double baseh = -(basep11yp21yleft * basedxn + basep21xp11xleft * basedyn);
			double basehabs = abs(baseh);
			if (basehabs < 1e-13) continue;

			for (int matchcut = 2; matchcut < match->ring.n; matchcut++)
			{
				for (int matchi = 0; matchi < match->ring.n; matchi++)
				{
					Lookup &lookupl = lookup[basei][(basei + basecut) % base->ring.n][matchi];
					if (lookupl.begin > matchi + matchcut || lookupl.end < matchi + matchcut) continue;
					Lookup &lookupleft = lookup[basei][basepeek % base->ring.n][matchi];

					Point &pmatchi = match->ring.ring[matchi], &pmatchj = match->ring.ring[(matchi + matchcut) % match->ring.n];
					double matchdx = pmatchj.x - pmatchi.x, matchdy = pmatchj.y - pmatchi.y;
					double matchlength = sqrt(matchdx * matchdx + matchdy * matchdy);
					if (matchlength >= 1e-13)
					{
						double matchdxn = matchdx / matchlength, matchdyn = matchdy / matchlength;

						Matching &gate = lookupl.matching[matchi + matchcut - lookupl.begin];

						int matchpeekend = std::min(lookupleft.end, matchi + matchcut - 1);
						for (int matchpeek = lookupleft.begin; matchpeek <= matchpeekend; matchpeek++)
						{
							Lookup &lookupright = lookup[basepeek % base->ring.n][(basei + basecut) % base->ring.n][matchpeek % match->ring.n];
							int matchindexright = matchpeek >= match->ring.n ? matchi + matchcut - match->ring.n : matchi + matchcut;
							if (matchindexright < lookupright.begin || matchindexright > lookupright.end) continue;
							Matching &right = lookupright.matching[matchindexright - lookupright.begin];

							Matching &left = lookupleft.matching[matchpeek - lookupleft.begin];

							Point &pmatchpeek = match->ring.ring[matchpeek % match->ring.n];
							double matchp11yp21yleft = pmatchi.y - pmatchpeek.y, matchp21xp11xleft = pmatchpeek.x - pmatchi.x;
							double matchleft = (matchp21xp11xleft * matchdxn - matchp11yp21yleft * matchdyn);
							double matchp11yp21yright = pmatchj.y - pmatchpeek.y, matchp21xp11xright = pmatchpeek.x - pmatchj.x;
							double matchright = (matchp21xp11xright * matchdxn - matchp11yp21yright * matchdyn);

							double hormin = std::min(baseleft, matchleft) - std::max(baseright, matchright);
							if (hormin < 1e-13) continue;

							double matchh = (matchp11yp21yleft * matchdxn + matchp21xp11xleft * matchdyn);
							double matchhabs = abs(matchh);
							if (matchhabs < 1e-13) continue;

							double dh1 = matchh / baseh;
							if (dh1 < 0.0) continue;

							double hormax = std::max(baseleft, matchleft) - std::min(baseright, matchright);
							double vertmax, vertmin;
							if (basehabs > matchhabs)
							{
								vertmax = basehabs;
								vertmin = matchhabs;
							}
							else
							{
								vertmin = basehabs;
								vertmax = matchhabs;
							}
							double quality = -hormax * vertmax;
							quality += (vertmin + RELUCTANCE * vertmax ) / (1.0 + RELUCTANCE) * (hormin + RELUCTANCE * hormax) / (1.0 + RELUCTANCE);



							double dynq = (quality + left.quality + right.quality);
							if (dynq > gate.quality)
							{
								gate.quality = dynq;
								gate.leftback = &left;
								gate.rightback = &right;
							}

							if (*aborted)
							{
								break;
							}
						}//matchpeek

					}


					if (*aborted)
					{
						break;
					}
				}//matchi

				if (*aborted)
				{
					break;
				}
			}//matchcut


			if (*aborted)
			{
				break;
			}
		}//basepeek

	}


	semaphore->release();
}

/*
void CoWorker::matchinvslot(int basei, int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup)
{

	Point &pbasei = base->ring.ring[basei], &pbasej = base->ring.ring[(basei + basecut)];
	double basedx = pbasej.x - pbasei.x, basedy = pbasej.y - pbasei.y;
	double basesq = basedx * basedx + basedy * basedy;

	for (int basepeek = basei + 1; basepeek < basei + basecut; basepeek++)
	{
		Point &pbasepeek = base->ring.ring[basepeek];
		double baseleftdx = pbasepeek.x - pbasei.x, baseleftdy = pbasepeek.y - pbasei.y;
		double baseleftsq = baseleftdx * baseleftdx + baseleftdy * baseleftdy;
		double baserightdx = pbasej.x - pbasepeek.x, baserightdy = pbasej.y - pbasepeek.y;
		double baserightsq = baserightdx * baserightdx + baserightdy * baserightdy;


		double &basep1x = pbasei.x, &basep1y = pbasei.y, &basep2x = pbasepeek.x, &basep2y = pbasepeek.y, &basep3x = pbasej.x, &basep3y = pbasej.y;
		double basep1yp2y = basep1y - basep2y, basep1xp3x = basep1x - basep3x, basep1yp3y = basep1y - basep3y, basep2xp1x = basep2x - basep1x;
		double baseareal = basep1yp2y * basep1xp3x + basep1yp3y * basep2xp1x;
		bool basesign = signbit(baseareal);


		for (int matchcut = 2; matchcut < match->ring.n; matchcut++)
		{
			for (int matchi = 0; matchi < match->ring.n; matchi++)
			{
				Lookup &lookupl = lookup[basei][basecut - 1][matchi];
				if (lookupl.begin > matchi + matchcut || lookupl.end < matchi + matchcut) continue;
				Lookup &lookupleft = lookup[basei][basepeek - basei - 1][matchi];

				Point &pmatchi = match->ring.ring[matchi], &pmatchj = match->ring.ring[(matchi + matchcut) % match->ring.n];
				double matchdx = pmatchj.x - pmatchi.x, matchdy = pmatchj.y - pmatchi.y;
				double matchsq = matchdx * matchdx + matchdy * matchdy;

				Matching &gate = lookupl.matching[matchi + matchcut - lookupl.begin];

				int matchpeekend = std::min(lookupleft.end, matchi + matchcut - 1);
				for (int matchpeek = lookupleft.begin; matchpeek <= matchpeekend; matchpeek++)
				{
					Lookup &lookupright = lookup[basepeek][basei + basecut - basepeek - 1][matchpeek % match->ring.n];
					int matchindexright = matchpeek >= match->ring.n ? matchi + matchcut - match->ring.n : matchi + matchcut;
					if (matchindexright < lookupright.begin || matchindexright > lookupright.end) continue;

					Point &pmatchpeek = match->ring.ring[matchpeek % match->ring.n];

					double &matchp1x = pmatchi.x, &matchp1y = pmatchi.y, &matchp2x = pmatchpeek.x, &matchp2y = pmatchpeek.y, &matchp3x = pmatchj.x, &matchp3y = pmatchj.y;
					double matchp1yp2y = matchp1y - matchp2y, matchp1xp3x = matchp1x - matchp3x, matchp1yp3y = matchp1y - matchp3y, matchp2xp1x = matchp2x - matchp1x;
					double matchareal = matchp1yp2y * matchp1xp3x + matchp1yp3y * matchp2xp1x;
					bool matchsign = signbit(matchareal);


					Matching &right = lookupright.matching[matchindexright - lookupright.begin];
					Matching &left = lookupleft.matching[matchpeek - lookupleft.begin];

					double matchleftdx = pmatchpeek.x - pmatchi.x, matchleftdy = pmatchpeek.y - pmatchi.y;
					double matchleftsq = matchleftdx * matchleftdx + matchleftdy * matchleftdy;

					double matchrightdx = pmatchj.x - pmatchpeek.x, matchrightdy = pmatchj.y - pmatchpeek.y;
					double matchrightsq = matchrightdx * matchrightdx + matchrightdy * matchrightdy;

					double quality;
					if (basesign == matchsign)
					{
						quality = -basesq - matchsq;
						quality -= baseleftsq + matchleftsq;
						quality -= baserightsq + matchrightsq;
					}
					else
					{
						quality = basesq < matchsq ? basesq - matchsq : matchsq - basesq;
						quality += baseleftsq < matchleftsq ? baseleftsq - matchleftsq : matchleftsq - baseleftsq;
						quality += baserightsq < matchrightsq ? baserightsq - matchrightsq : matchrightsq - baserightsq;
					}

					double dynq = quality + left.quality + right.quality;
					if (dynq > gate.quality)
					{
						gate.quality = dynq;
						gate.leftback = &left;
						gate.rightback = &right;
					}

				}//matchpeek

				if (*aborted)
				{
					break;
				}
			}//matchi

			if (*aborted)
			{
				break;
			}
		}//matchcut


		if (*aborted)
		{
			break;
		}
	}//basepeek

	semaphore->release();
}
*/