#include "CoWorker.h"
#include <cmath>

CoWorker::CoWorker(QSemaphore *semaphore) :
	semaphore(semaphore)
{
	moveToThread(&thread);
	connect(this, SIGNAL(initbaseperimeter(int, SRing2 *, SRing2 *, Lookup *, volatile bool *)), this, SLOT(initbaseperimeterslot(int, SRing2 *, SRing2 *, Lookup *, volatile bool *)));
	connect(this, SIGNAL(initmatchperimeter(int, SRing2 *, SRing2 *, Lookup *, volatile bool *)), this, SLOT(initmatchperimeterslot(int, SRing2 *, SRing2 *, Lookup *, volatile bool *)));
	connect(this, SIGNAL(matchinv(/**/int , int , SRing2 *, SRing2 *, Lookup *, volatile bool *)), this, SLOT(matchinvslot(/**/int , int , SRing2 *, SRing2 *, Lookup *, volatile bool *)));
	thread.start();
}

CoWorker::~CoWorker()
{
	thread.exit();
	thread.wait();
}

void CoWorker::initbaseperimeterslot(int basei, SRing2 *base, SRing2 *match, Lookup *lookup, volatile bool *aborted)
{
	Matching **basematching = (*lookup)[basei][(basei + 1) % base->ring.n];
	for (int matchi = 0; matchi < match->ring.n; matchi++)
	{
		double absarea = 0.0;
		Point &p1 = match->ring.ring[0];
		for (int matchj = matchi + 1, matchk = matchi + 2; matchk < matchi + match->ring.n; matchj = matchk, matchk++)
		{
			Point &p2 = match->ring.ring[matchj % match->ring.n], &p3 = match->ring.ring[matchk % match->ring.n];
			double &p1x = p1.x, &p1y = p1.y, &p2x = p2.x, &p2y = p2.y, &p3x = p3.x, &p3y = p3.y;
			double p1yp2y = p1y - p2y, p1xp3x = p1x - p3x, p1yp3y = p1y - p3y, p2xp1x = p2x - p1x;
			absarea -= abs(.5 * (p1yp2y * p1xp3x + p1yp3y * p2xp1x));
			basematching[matchi][matchk % match->ring.n].quality = 2.0 * absarea;
		}
		if (*aborted)
		{
			break;
		}
	}
	semaphore->release();
}

void CoWorker::initmatchperimeterslot(int matchi, SRing2 *base, SRing2 *match, Lookup *lookup, volatile bool *aborted)
{
	for (int basei = 0; basei < base->ring.n; basei++)
	{
		double absarea = 0.0;
		Point &p1 = base->ring.ring[0];
		for (int basej = basei + 1, basek = basei + 2; basek < basei + base->ring.n; basej = basek, basek++)
		{
			Point &p2 = base->ring.ring[basej % base->ring.n], &p3 = base->ring.ring[basek % base->ring.n];
			double &p1x = p1.x, &p1y = p1.y, &p2x = p2.x, &p2y = p2.y, &p3x = p3.x, &p3y = p3.y;
			double p1yp2y = p1y - p2y, p1xp3x = p1x - p3x, p1yp3y = p1y - p3y, p2xp1x = p2x - p1x;
			absarea -= abs(.5 * (p1yp2y * p1xp3x + p1yp3y * p2xp1x));
			(*lookup)[basei][basek % base->ring.n][matchi][(matchi + 1) % match->ring.n].quality = 2.0 * absarea;
		}
		if (*aborted)
		{
			break;
		}
	}
	semaphore->release();
}

void CoWorker::matchinvslot(/**/int basei, int basecut, SRing2 *base, SRing2 *match, Lookup *lookup, volatile bool *aborted)
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
					Point &pmatchi = match->ring.ring[matchi], &pmatchj = match->ring.ring[(matchi + matchcut) % match->ring.n];
					double matchdx = pmatchj.x - pmatchi.x, matchdy = pmatchj.y - pmatchi.y;
					double matchlength = sqrt(matchdx * matchdx + matchdy * matchdy);
					if (matchlength >= 1e-13)
					{
						double matchdxn = matchdx / matchlength, matchdyn = matchdy / matchlength;

						Matching &gate = (*lookup)[basei][(basei + basecut) % base->ring.n][matchi][(matchi + matchcut) % match->ring.n];

						for (int matchpeek = matchi + 1; matchpeek < matchi + matchcut; matchpeek++)
						{
							Point &pmatchpeek = match->ring.ring[matchpeek % match->ring.n];
							double matchp11yp21yleft = pmatchi.y - pmatchpeek.y, matchp21xp11xleft = pmatchpeek.x - pmatchi.x;
							double matchleft = (matchp21xp11xleft * matchdxn - matchp11yp21yleft * matchdyn);
							double matchp11yp21yright = pmatchj.y - pmatchpeek.y, matchp21xp11xright = pmatchpeek.x - pmatchj.x;
							double matchright = (matchp21xp11xright * matchdxn - matchp11yp21yright * matchdyn);
							double matchh = (matchp11yp21yleft * matchdxn + matchp21xp11xleft * matchdyn);
							double absmatchh = abs(matchh);

							double quality;
							if (signbit(baseh) != signbit(matchh))
							{
								quality = -absbaseh * (baseleft - baseright) - absmatchh * (matchleft - matchright);
							}
							else
							{
								quality = std::min(absbaseh, absmatchh) * (std::min(baseleft, matchleft) - std::max(baseright, matchright));
							}

							Matching &left = (*lookup)[basei][basepeek % base->ring.n][matchi][matchpeek % match->ring.n];
							Matching &right = (*lookup)[basepeek % base->ring.n][(basei + basecut) % base->ring.n][matchpeek % match->ring.n][(matchi + matchcut) % match->ring.n];

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