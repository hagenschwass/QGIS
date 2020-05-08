#include "CoWorker.h"
#include <cmath>

CoWorker::CoWorker(QSemaphore *semaphore, volatile bool *aborted) :
	semaphore(semaphore),
	aborted(aborted)
{
	moveToThread(&thread);
	connect(this, SIGNAL(initlookupinv(int, SRing2 *, SRing2 *, LookupArg *, double)), this, SLOT(initlookupinvslot(int, SRing2 *, SRing2 *, LookupArg *, double)));
	connect(this, SIGNAL(updateexitcosts(int , SRing2 *, SRing2 *, LookupArg *)), this, SLOT(updateexitcostsslot(int , SRing2 *, SRing2 *, LookupArg *)));
	connect(this, SIGNAL(matchinv(/**/int , int , SRing2 *, SRing2 *, LookupArg *)), this, SLOT(matchinvslot(/**/int , int , SRing2 *, SRing2 *, LookupArg *)));
	thread.start();
}

CoWorker::~CoWorker()
{
	thread.exit();
	thread.wait();
}

void CoWorker::initlookupinvslot(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup, double skiparea)
{
	if (*aborted == false)
	{
		Matching *tempmatching = new Matching[match->ring.n];
		for (int i = 0; i < match->ring.n; i++)
		{
			tempmatching[i].base1 = basei;
			tempmatching[i].leftback = nullptr;
			tempmatching[i].rightback = nullptr;
			tempmatching[i].exitcost = -2.0 * (base->area + match->area);
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


				double matcharea = 0.0;
				double matchareaabs = 0.0;
				Point &matchp1 = match->ring.ring[matchi];
				for (int matchj = matchi + 1, matchk = matchi + 2; ; matchj = matchk, matchk++)
				{
					Point &matchp2 = match->ring.ring[matchj % match->ring.n], &matchp3 = match->ring.ring[matchk % match->ring.n];
					double &matchp1x = matchp1.x, &matchp1y = matchp1.y, &matchp2x = matchp2.x, &matchp2y = matchp2.y, &matchp3x = matchp3.x, &matchp3y = matchp3.y;
					double matchp1yp2y = matchp1y - matchp2y, matchp1xp3x = matchp1x - matchp3x, matchp1yp3y = matchp1y - matchp3y, matchp2xp1x = matchp2x - matchp1x;
					double matchareal = -.5 * (matchp1yp2y * matchp1xp3x + matchp1yp3y * matchp2xp1x);
					matcharea += matchareal;
					if (abs(matcharea) > skiparea)
					{
						lookupend.matching = new Matching[matchk - matchi - 1];
						memcpy_s(lookupend.matching, sizeof(Matching) * (matchk - matchi - 1), tempmatching, sizeof(Matching) * (matchk - matchi - 1));
						lookupend.end = matchk - 1;
						break;
					}
					matchareaabs += 2.0 * abs(matchareal);
					tempmatching[matchk - matchi - 1].quality = -matchareaabs;
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

			double basearea = 0.0;
			double baseareaabs = 0.0;
			Point &basep1 = base->ring.ring[basei];
			for (int basej = basei + 1, basek = basei + 2; basek < basei + base->ring.n; basej = basek++)
			{
				Point &basep2 = base->ring.ring[basej % base->ring.n], &basep3 = base->ring.ring[basek % base->ring.n];
				double &basep1x = basep1.x, &basep1y = basep1.y, &basep2x = basep2.x, &basep2y = basep2.y, &basep3x = basep3.x, &basep3y = basep3.y;
				double basep1yp2y = basep1y - basep2y, basep1xp3x = basep1x - basep3x, basep1yp3y = basep1y - basep3y, basep2xp1x = basep2x - basep1x;
				double baseareal = .5 * (basep1yp2y * basep1xp3x + basep1yp3y * basep2xp1x);
				basearea += baseareal;
				baseareaabs += 2.0 * abs(baseareal);


				Lookup *&lookupmatch = lookupbase[basek % base->ring.n];
				lookupmatch = new Lookup[match->ring.n];

				for (int matchi = 0; matchi < match->ring.n; matchi++)
				{
					tempmatching[0].quality = -baseareaabs;
					tempmatching[0].match1 = matchi;
					tempmatching[0].match2 = matchi + 1;
					tempmatching[0].base2 = basek;
					int begin = abs(basearea) > skiparea ? -1 : matchi + 1;
					double matcharea = 0.0;
					double matchareaabs = 0.0;
					Point &matchp1 = match->ring.ring[matchi];
					for (int matchj = matchi + 1, matchk = matchi + 2; ; matchj = matchk, matchk++)
					{
						Point &matchp2 = match->ring.ring[matchj % match->ring.n], &matchp3 = match->ring.ring[matchk % match->ring.n];
						double &matchp1x = matchp1.x, &matchp1y = matchp1.y, &matchp2x = matchp2.x, &matchp2y = matchp2.y, &matchp3x = matchp3.x, &matchp3y = matchp3.y;
						double matchp1yp2y = matchp1y - matchp2y, matchp1xp3x = matchp1x - matchp3x, matchp1yp3y = matchp1y - matchp3y, matchp2xp1x = matchp2x - matchp1x;
						double matchareal = -.5 * (matchp1yp2y * matchp1xp3x + matchp1yp3y * matchp2xp1x);
						matcharea += matchareal;
						if (begin < 0 && abs(basearea - matcharea) < skiparea)
						{
							begin = matchk;
						}
						if (begin > -1 && abs(matcharea - basearea) > skiparea)
						{
							Lookup &lookupend = lookupmatch[matchi];
							lookupend.begin = begin;


							lookupend.matching = new Matching[matchk - begin];
							memcpy_s(lookupend.matching, sizeof(Matching) * (matchk - begin), &tempmatching[begin - matchi - 1], sizeof(Matching) * (matchk - begin));
							lookupend.end = matchk - 1;
							break;
						}
						matchareaabs += 2.0 * abs(matchareal);
						tempmatching[matchk - matchi - 1].quality = -matchareaabs - baseareaabs;
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

void CoWorker::updateexitcostsslot(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup)
{
	if (*aborted == false)
	{
		LookupArg &lookup1 = lookup[basei];
		for (int basej = basei + 1; basej < basei + base->ring.n; basej++)
		{
			Lookup *lookup2 = lookup1[basej % base->ring.n];
			Lookup *target2 = lookup[basej % base->ring.n][basei];
			for (int matchi = 0; matchi < match->ring.n; matchi++)
			{
				Lookup &lookup3 = lookup2[matchi];
				for (int matchj = lookup3.begin; matchj <= lookup3.end; matchj++)
				{
					Matching &matching = lookup3.matching[matchj - lookup3.begin];
					Lookup &target3 = target2[matchj % match->ring.n];
					if (matchi < target3.begin || matchi > target3.end % match->ring.n) continue;
					target3.matching[matchi - target3.begin].exitcost = matching.quality;
				}
			}

			if (*aborted)
			{
				break;
			}
		}
	}
	semaphore->release();
}

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
			double absbaseh = abs(baseh);


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
							double matchh = (matchp11yp21yleft * matchdxn + matchp21xp11xleft * matchdyn);
							double absmatchh = abs(matchh);

							double quality = -std::max(absbaseh, absmatchh) * (std::max(baseleft, matchleft) - std::min(baseright, matchright));
							if (signbit(baseh) != signbit(matchh))
							{
								quality = -absbaseh * (baseleft - baseright) - absmatchh * (matchleft - matchright);
							}
							else
							{
								quality = std::min(absbaseh, absmatchh) * (std::min(baseleft, matchleft) - std::max(baseright, matchright));
							}


							double dynq = quality + left.quality + right.quality;
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