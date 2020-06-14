#include "MircoWorker.h"
#include "Symmetry.h"
#include "Matching.h"

MicroWorker::MicroWorker(volatile bool *aborted, QSemaphore *semaphore) :
	aborted(aborted),
	loadindex(0),
	semaphore(semaphore)
{
	start();
}

MicroWorker::~MicroWorker()
{
	runmutex.lock();
	job = jExit;
	runmutex.unlock();
	runwait.wakeOne();
	wait();
}

void MicroWorker::setupAdjustInvSymmetry(int capacity, QSemaphore *semaphore)
{
	this->semaphore = semaphore;
	pointmatches = new PointMatch*[capacity];
}

void MicroWorker::loadAdjustInvSymmetry(PointMatch *pointmatch)
{
	pointmatches[loadindex++] = pointmatch;
}

void MicroWorker::runAdjustInvSymmetry(SRing &match, SRing &matchout)
{
	this->match = match;
	this->matchout = matchout;
	runmutex.lock();
	job = jAdjustInvSymmetry;
	runmutex.unlock();
	runwait.wakeOne();
}

void MicroWorker::clearAdjustInvSymmetry()
{
	delete[] pointmatches;
}

void MicroWorker::setupFreeMatchingTrees(SRing2 &base, LookupT lookup, Constraint constraint, int capacity, QSemaphore *semaphore)
{
	this->base = base;
	this->constraint = constraint;
	this->lookup = lookup;
	this->semaphore = semaphore;
	trees = new FreeMatchingTreeMagazin[capacity];
}

void MicroWorker::loadFreeMatchingTrees(FreeMatchingTree *tree, int basei)
{
	trees[loadindex++] = { tree, basei };
}

void MicroWorker::runFreeMatchingTrees()
{
	runmutex.lock();
	job = jFreeMatchingTrees;
	runmutex.unlock();
	runwait.wakeOne();
}

void MicroWorker::clearFreeMatchingTrees()
{
	delete[] trees;
}

void MicroWorker::run()
{
	runmutex.lock();
	semaphore->release();
	for (;;)
	{
		runwait.wait(&runmutex);
		switch (job)
		{
		case jExit:
			runmutex.unlock();
			return;
		case jAdjustInvSymmetry:
		{
			runmutex.unlock();
			int loadeds = loadindex;
			loadindex = 0;
			for (int i = 0; i < loadeds; i++)
			{
				PointMatch *pm = pointmatches[i];
				/*Matching *gate = pm->gate;

				
				Point &pbasei = match.ring[match.n - gate->base1 - 1], &pbasej = match.ring[match.n - (gate->base2 % match.n) - 1];
				double basedx = pbasej.x - pbasei.x, basedy = pbasej.y - pbasei.y;
				double baselength = sqrt(basedx * basedx + basedy * basedy);
				double basedxn = basedx / baselength, basedyn = basedy / baselength;

				Point &pbasepeek = match.ring[match.n - gate->rightback->base1 - 1];
				double basep11yp21yleft = pbasei.y - pbasepeek.y, basep21xp11xleft = pbasepeek.x - pbasei.x;
				double baseleft = (basep21xp11xleft * basedxn - basep11yp21yleft * basedyn);
				double basep11yp21yright = pbasej.y - pbasepeek.y, basep21xp11xright = pbasepeek.x - pbasej.x;
				double baseright = (basep21xp11xright * basedxn - basep11yp21yright * basedyn);
				double baseh = -(basep11yp21yleft * basedxn + basep21xp11xleft * basedyn);

				Point &pmatchi = match.ring[gate->match1], &pmatchj = match.ring[gate->match2 % match.n];
				double matchdx = pmatchj.x - pmatchi.x, matchdy = pmatchj.y - pmatchi.y;
				double matchlength = sqrt(matchdx * matchdx + matchdy * matchdy);
				double matchdxn = matchdx / matchlength, matchdyn = matchdy / matchlength;

				Point &pmatchpeek = match.ring[gate->rightback->match1];
				double matchp11yp21yleft = pmatchi.y - pmatchpeek.y, matchp21xp11xleft = pmatchpeek.x - pmatchi.x;
				double matchleft = (matchp21xp11xleft * matchdxn - matchp11yp21yleft * matchdyn);
				double matchp11yp21yright = pmatchj.y - pmatchpeek.y, matchp21xp11xright = pmatchpeek.x - pmatchj.x;
				double matchright = (matchp21xp11xright * matchdxn - matchp11yp21yright * matchdyn);
				double matchh = (matchp11yp21yleft * matchdxn + matchp21xp11xleft * matchdyn);
*/
/*
				double basehor = baseleft - baseright, matchhor = matchleft - matchright;
				double baseleftnew = .5 * (baseleft + matchleft * basehor / matchhor);
				double matchleftnew = .5 * (matchleft + baseleft * matchhor / basehor);

				
				double hrat = .5 * (baseh / baseleft + matchh / matchleft);
				double basehnew = .5 *( baseh + matchh);// hrat * baseleftnew;
				double matchhnew = .5 * (baseh + matchh);// hrat * matchleftnew;
*/
				/*
				double leftnew = .5 * (matchleft + baseleft);
				double rightnew = .5 * (matchright + baseright);
				Point pbaseleft = pbasei + (leftnew * Point{ basedxn, basedyn });
				Point pbaseright = pbasej + (rightnew * Point{ basedxn, basedyn });
				Point pmatchleft = pmatchi + (leftnew * Point{ matchdxn, matchdyn });
				Point pmatchright = pmatchj + (rightnew * Point{ matchdxn, matchdyn });

				double hnew = .5 * (baseh + matchh);

				Point pbasenew = (.5 * (pbaseleft + pbaseright)) + (hnew * Point{ -basedyn, basedxn });
				Point pmatchnew = (.5 * (pmatchleft + pmatchright)) + (hnew * Point{ matchdyn, -matchdxn });

				if (pm->base.base == pm->match)
				{

					matchout.ring[match.n - gate->rightback->base1 - 1] = .5 * (pbasenew + pmatchnew);

				}
				else
				{

					matchout.ring[gate->rightback->match1] = pmatchnew;
					matchout.ring[match.n - gate->rightback->base1 - 1] = pbasenew;

				}*/
			}
			runmutex.lock();
			semaphore->release(loadeds + 1);
			break;
		}
		case jFreeMatchingTrees:
		{
			runmutex.unlock();
			int loadeds = loadindex;
			loadindex = 0;
			for (int i = 0; i < loadeds; i++)
			{
				FreeMatchingTreeMagazin &mag = trees[i];
				Matching *matching = nullptr, *opposite = nullptr;
				double maxq = -DBL_MAX;
				if (constraint[mag.basei] > -1 && *aborted == false)
				{
					for (int basej = 0; basej < base.ring.n; basej++)
					{
						if (constraint[basej] > -1 && basej != mag.basei)
						{
							Lookup &l1 = lookup[mag.basei][basej][constraint[mag.basei]];
							int matchj = constraint[basej];
							if (matchj < l1.begin) matchj += base.ring.n;
							if (matchj <= l1.end)
							{
								Lookup &o1 = lookup[basej][mag.basei][constraint[basej]];
								int matchi = constraint[mag.basei];
								if (matchi < o1.begin) matchi += base.ring.n;
								if (matchi <= o1.end)
								{
									Matching &matchingl = l1.matching[matchj - l1.begin];
									Matching &oppositel = o1.matching[matchi - o1.begin];
									double q = matchingl.quality + oppositel.quality + matchingl.cost + oppositel.cost;
									if (q > maxq)
									{
										maxq = q;
										matching = &matchingl;
										opposite = &oppositel;
									}
								}
							}
						}
					}
				}
				if (matching == nullptr)
				{
					mag.tree->up = nullptr;
					mag.tree->down = nullptr;
				}
				else
				{
					*mag.tree = freeMatchingTree(matching, opposite);
				}
			}
			runmutex.lock();
			semaphore->release(loadeds + 1);
			break;
		}
		default:break;
		}
	}
}