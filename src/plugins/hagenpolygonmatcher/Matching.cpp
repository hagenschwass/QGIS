#include "matching.h"

#include "CoWorker.h"
#include "SpecialWorker.h"
#include "MircoWorker.h"

#include <cfloat>

inline MatchingResult initMatchingResult()
{
	return{ nullptr, nullptr, 0.0, -DBL_MAX/*, nullptr*/ };
}

inline void deleteMatchingResult(MatchingResult &result)
{
	//delete[] result.constraint;
}

/*O(n^6)*/
inline LookupT computeInvMatching(SRing2 &base, SRing2 &match, double skiparea, MatchingResult &result, SpecialWorker *specialworker, QSemaphore *specialsemaphore, int nworkers, CoWorker** workers, QSemaphore *semaphore, volatile bool &aborted)
{
	LookupT lookup = new LookupArg[base.ring.n];
	for (int i = 0; i < base.ring.n; i++)
	{
		LookupArg &base1 = lookup[i];
		base1 = nullptr;
	}
	if (aborted)
	{
		return lookup;
	}

	int currentworker = 0;

	for (int basei = 0; basei < base.ring.n; basei++)
	{
		workers[currentworker]->initlookupinvforbase(basei, &base, &match, lookup, skiparea);
		currentworker = (currentworker + 1) % nworkers;
	}
	semaphore->acquire(base.ring.n);

	if (aborted)
	{
		return lookup;
	}

	for (int basei = 0; basei < base.ring.n; basei++)
	{
		workers[currentworker]->initshortcutsforbase(basei, &base, &match, lookup);
		currentworker = (currentworker + 1) % nworkers;
	}

	if (aborted)
	{
		semaphore->acquire(base.ring.n);
		return lookup;
	}

	for (int matchi = 0; matchi < match.ring.n; matchi++)
	{
		workers[currentworker]->initshortcutsformatch(matchi, &base, &match, lookup);
		currentworker = (currentworker + 1) % nworkers;
	}
	semaphore->acquire(base.ring.n * 2);

	if (aborted)
	{
		return lookup;
	}

	for (int basecut = 2; basecut < base.ring.n; basecut++)
	{


		for (int basei = 0; basei < base.ring.n; basei++)
		{

			workers[currentworker]->computeshortcutsinv(basei, basecut, &base, &match, lookup);
			currentworker = (currentworker + 1) % nworkers;


			if (aborted)
			{
				semaphore->release(base.ring.n - basei - 1);
				break;
			}
		}//basei

		for (int i = 0; i < base.ring.n; i++)
		{
			semaphore->acquire();
		}

		if (aborted)
		{
			break;
		}

	}//basecut

	if (aborted)
	{
		return lookup;
	}

	int specialworkerstartedcount = 0;
	for (int basecut = 2; basecut < base.ring.n; basecut++)
	{


		for (int basei = 0; basei < base.ring.n; basei++)
		{

			workers[currentworker]->matchinv(basei, basecut, &base, &match, lookup);
			currentworker = (currentworker + 1) % nworkers;


			if (aborted)
			{
				semaphore->release(base.ring.n - basei - 1);
				break;
			}
		}//basei

		for (int i = 0; i < base.ring.n; i++)
		{
			semaphore->acquire();
		}

		if (aborted)
		{
			break;
		}

		if (2 * basecut >= base.ring.n)
		{
			specialworker->searchbestmatch(basecut, &base, &match, lookup, &result);
			specialworkerstartedcount++;
		}

	}//basecut

	specialsemaphore->acquire(specialworkerstartedcount);

	return lookup;
}

inline void deleteMatching(SRing2 &base, SRing2 &match, LookupT lookup)
{
	for (int i = 0; i < base.ring.n; i++)
	{
		LookupArg base1 = lookup[i];
		if (base1 != nullptr)
		{
			for (int j = 0; j < base.ring.n; j++)
			{
				Lookup *base2 = base1[j];
				if (base2 != nullptr)
				{
					for (int k = 0; k < match.ring.n; k++)
					{
						Lookup &match1 = base2[k];
						delete[] match1.matching;
					}
					delete[] base2;
				}
			}
			delete[] base1;
		}
	}
	delete[] lookup;
}

/*
inline LookupT computeInvMatching(SRing2 &base, SRing2 &match, double skiparea, double &quality, Matching *&matching, SpecialWorker *specialworker, QSemaphore *specialsemaphore, int nworkers, CoWorker** workers, QSemaphore *semaphore, volatile bool &aborted)
{
	LookupT lookup = new LookupArg[base.ring.n];
	for (int i = 0; i < base.ring.n; i++)
	{
		LookupArg &base1 = lookup[i];
		int baseend = base.ring.n - i - 1;
		base1 = new Lookup*[baseend];
		for (int j = 0; j < baseend; j++)
		{
			Lookup *& match1 = base1[j];
			match1 = new Lookup[match.ring.n];
			for (int k = 0; k < match.ring.n; k++)
			{
				match1[k].matching = nullptr;
			}
		}
	}
	if (aborted)
	{
		return lookup;
	}

	int currentworker = 0;

	for (int basei = 0; basei < base.ring.n - 1; basei++)
	{
		workers[currentworker]->initlookupinvforbase(basei, &base, &match, lookup, skiparea);
		currentworker = (currentworker + 1) % nworkers;
	}
	semaphore->acquire(base.ring.n - 1);

	if (aborted)
	{
		return lookup;
	}

	for (int basei = 0; basei < base.ring.n - 1; basei++)
	{
		workers[currentworker]->updateexitcosts(basei, &base, &match, lookup);
		currentworker = (currentworker + 1) % nworkers;
	}
	semaphore->acquire(base.ring.n - 1);

	if (aborted)
	{
		return lookup;
	}

	int specialworkerstarts = 0;
	for (int basecut = 2; basecut < base.ring.n; basecut++)
	{

		int workerstarts = 0;
		for (int basei = 0; basei + basecut < base.ring.n; basei++)
		{

			workers[currentworker]->matchinv(basei, basecut, &base, &match, lookup);
			currentworker = (currentworker + 1) % nworkers;
			workerstarts++;

			if (aborted)
			{
				break;
			}
		}//basei

		for (int i = 0; i < workerstarts; i++)
		{
			semaphore->acquire();
		}

		if (aborted)
		{
			break;
		}

		specialworker->searchbestmatch(basecut, &base, &match, lookup, &quality, &matching);
		specialworkerstarts++;

	}//basecut

	specialsemaphore->acquire(specialworkerstarts);

	return lookup;
}

inline void deleteMatching(SRing2 &base, SRing2 &match, LookupT lookup)
{
	for (int i = 0; i < base.ring.n; i++)
	{
		LookupArg base1 = lookup[i];
		int jend = base.ring.n - i - 1;
		for (int j = 0; j < jend; j++)
		{
			Lookup *base2 = base1[j];
			for (int k = 0; k < match.ring.n; k++)
			{
				Lookup &match1 = base2[k];
				delete[] match1.matching;
			}
			delete[] base2;
		}
		delete[] base1;
	}
	delete[] lookup;
}
*/

inline void createConstraintRec(Matching *m, Constraint constraint)
{
	constraint[m->base1] = m->match1;
	if (m->rightback != nullptr)
	{
		createConstraintRec(m->rightback, constraint);
		createConstraintRec(m->leftback, constraint);
	}
}

inline Constraint createConstraint(SRing2 &base, MatchingResult &result)
{
	Constraint constraint = new int[base.ring.n];
	memset(constraint, -1, sizeof(int) * base.ring.n);
	createConstraintRec(result.matching, constraint);
	createConstraintRec(result.opposite, constraint);
	return constraint;
}

inline void deleteConstraint(Constraint constraint)
{
	delete[] constraint;
}

inline int privateCountMatchingTree(Matching *matching)
{
	if (matching->rightback == nullptr) return 1;
	return privateCountMatchingTree(matching->rightback) + privateCountMatchingTree(matching->leftback) + 1;
}

inline FreeMatching *privateBuildFreeMatchingTree(Matching *matching, FreeMatching *freematching, int &index)
{
	int i = index++;
	freematching[i].base = matching->base1;
	freematching[i].match = matching->match1;
	freematching[i].quality = matching->quality;
	if (matching->rightback == nullptr)
	{
		freematching[i].left = nullptr;
		freematching[i].right = nullptr;
	}
	else
	{
		//int &leftcount = freematching[i].leftcount;
		//leftcount = index;
		freematching[i].left = privateBuildFreeMatchingTree(matching->leftback, freematching, index);
		//leftcount = index - leftcount;
		//int &rightcount = freematching[i].rightcount;
		//rightcount = index;
		freematching[i].right = privateBuildFreeMatchingTree(matching->rightback, freematching, index);
		//rightcount = index - rightcount;
	}
	return &freematching[i];
}

inline FreeMatchingTree freeMatchingTree(Matching *up, Matching *down)
{
	FreeMatchingTree result;
	int &upcount = result.upcount;
	upcount = privateCountMatchingTree(up);
	result.up = new FreeMatching[upcount];
	upcount = 0;
	result.up = privateBuildFreeMatchingTree(up, result.up, upcount);
	int &downcount = result.downcount;
	downcount = privateCountMatchingTree(down);
	result.down = new FreeMatching[downcount];
	downcount = 0;
	result.down = privateBuildFreeMatchingTree(down, result.down, downcount);
	return result;
}

inline void adjustFreeMatching(SRing2 &base, SRing2 &match, FreeMatching *fm, int basej, int matchj)
{
	if (fm->right == nullptr) return;
	Transform tbaseleft;
	Transform tbaseright;
	Transform tmatchleft;
	Transform tmatchright;
	{
		Point &pbasei = base.ring.ring[fm->base], &pbasej = base.ring.ring[basej];
		double basedx = pbasej.x - pbasei.x, basedy = pbasej.y - pbasei.y;
		double baselength = sqrt(basedx * basedx + basedy * basedy);
		double basedxn = basedx / baselength, basedyn = basedy / baselength;

		Point &pbasepeek = base.ring.ring[fm->right->base];
		double basep11yp21yleft = pbasei.y - pbasepeek.y, basep21xp11xleft = pbasepeek.x - pbasei.x;
		double baseleft = (basep21xp11xleft * basedxn - basep11yp21yleft * basedyn);
		double basep11yp21yright = pbasej.y - pbasepeek.y, basep21xp11xright = pbasepeek.x - pbasej.x;
		double baseright = (basep21xp11xright * basedxn - basep11yp21yright * basedyn);
		double baseh = -(basep11yp21yleft * basedxn + basep21xp11xleft * basedyn);

		Point &pmatchi = match.ring.ring[fm->match], &pmatchj = match.ring.ring[matchj];
		double matchdx = pmatchj.x - pmatchi.x, matchdy = pmatchj.y - pmatchi.y;
		double matchlength = sqrt(matchdx * matchdx + matchdy * matchdy);
		double matchdxn = matchdx / matchlength, matchdyn = matchdy / matchlength;

		Point &pmatchpeek = match.ring.ring[fm->right->match];
		double matchp11yp21yleft = pmatchi.y - pmatchpeek.y, matchp21xp11xleft = pmatchpeek.x - pmatchi.x;
		double matchleft = (matchp21xp11xleft * matchdxn - matchp11yp21yleft * matchdyn);
		double matchp11yp21yright = pmatchj.y - pmatchpeek.y, matchp21xp11xright = pmatchpeek.x - pmatchj.x;
		double matchright = (matchp21xp11xright * matchdxn - matchp11yp21yright * matchdyn);
		double matchh = (matchp11yp21yleft * matchdxn + matchp21xp11xleft * matchdyn);

		double leftnew = .5 * (matchleft + baseleft);
		double rightnew = .5 * (matchright + baseright);
		Point pbaseleft = pbasei + (leftnew * Point{ basedxn, basedyn });
		Point pbaseright = pbasej + (rightnew * Point{ basedxn, basedyn });
		Point pmatchleft = pmatchi + (leftnew * Point{ matchdxn, matchdyn });
		Point pmatchright = pmatchj + (rightnew * Point{ matchdxn, matchdyn });

		double hnew = .5 * (baseh + matchh);

		Point pbasenew = (.5 * (pbaseleft + pbaseright)) + (hnew * Point{ -basedyn, basedxn });
		Point pmatchnew = (.5 * (pmatchleft + pmatchright)) + (hnew * Point{ matchdyn, -matchdxn });

		tbaseleft = transform(pbasepeek, pbasenew, pbasei);
		tbaseright = transform(pbasepeek, pbasenew, pbasej);
		tmatchleft = transform(pmatchpeek, pmatchnew, pmatchi);
		tmatchright = transform(pmatchpeek, pmatchnew, pmatchj);

		pbasepeek = pbasenew;
		pmatchpeek = pmatchnew;
	}

	int begin = fm->base + 1, end = fm->right->base;
	if (end < begin) end += base.ring.n;
	for (int i = begin; i < end; i++)
	{
		base.ring.ring[i % base.ring.n] = tbaseleft * base.ring.ring[i % base.ring.n];
	}
	begin = fm->match + 1;
	end = fm->right->match;
	if (end < begin) end += match.ring.n;
	for (int i = begin; i < end; i++)
	{
		match.ring.ring[i % match.ring.n] = tmatchleft * match.ring.ring[i % match.ring.n];
	}
	begin = fm->right->base + 1;
	end = basej;
	if (end < begin) end += base.ring.n;
	for (int i = begin; i < end; i++)
	{
		base.ring.ring[i % base.ring.n] = tbaseright * base.ring.ring[i % base.ring.n];
	}
	begin = fm->right->match + 1;
	end = matchj;
	if (end < begin) end += match.ring.n;
	for (int i = begin; i < end; i++)
	{
		match.ring.ring[i % match.ring.n] = tmatchright * match.ring.ring[i % match.ring.n];
	}

	adjustFreeMatching(base, match, fm->left, fm->right->base, fm->right->match);
	adjustFreeMatching(base, match, fm->right, basej, matchj);
}

inline void adjustFreeMatchingTree(SRing2 &base, SRing2 &match, FreeMatchingTree &tree)
{
	FreeMatching *up = tree.up, *down = tree.down;
	for (int i = 0; i < 1; i++)
	{
		adjustFreeMatching(base, match, up, down->base, down->match);
		adjustFreeMatching(base, match, down, up->base, up->match);
		//swapSRing2sInv(base, match);
		meanSRing2sInv(base, match);
	}
}

inline void deleteFreeMatchingTree(FreeMatchingTree &tree)
{
	delete[] tree.up;
	delete[] tree.down;
	//deleteSRing2(tree.base);
	//deleteSRing2(tree.match);
}

inline FreeMatchingTrees createFreeMatchingTrees(SRing2 &base, LookupT lookup, Constraint constraint, int nworker, MicroWorker **microworker, QSemaphore *semaphore, volatile bool &aborted)
{
	FreeMatchingTrees trees = new FreeMatchingTree[base.ring.n];

	for (int iw = 0; iw < nworker; iw++)
	{
		microworker[iw]->setupFreeMatchingTrees(base, lookup, constraint, (base.ring.n / nworker) + 1, semaphore);
	}

	if (aborted == false)
	{
		int currentworker = 0;
		for (int i = 0; i < base.ring.n; i++)
		{
			microworker[currentworker]->loadFreeMatchingTrees(&trees[i], i);
			currentworker = (currentworker + 1) % nworker;
		}
		for (int worker = 0; worker < nworker; worker++)
		{
			microworker[worker]->runFreeMatchingTrees();
		}
		semaphore->acquire(base.ring.n + nworker);
	}

	for (int worker = 0; worker < nworker; worker++)
	{
		microworker[worker]->clearFreeMatchingTrees();
	}

	return trees;
}

inline void adjustFreeMatchingTrees(SRing2 &base, SRing2 &match, FreeMatchingTrees &trees)
{
	for (int i = 0; i < base.ring.n; i++)
	{
		FreeMatchingTree &tree = trees[i];
		if (tree.up == nullptr)
		{
			//tree.base.ring.ring = nullptr;
			//tree.match.ring.ring = nullptr;
		}
		else
		{
			//tree.base = cloneSRing2(base);
			//tree.match = cloneSRing2(match);
			//adjustFreeMatchingTree(tree.base, tree.match, tree);
		}
	}
}

inline void deleteFreeMatchingTrees(SRing2 &base, FreeMatchingTrees trees)
{
	for (int i = 0; i < base.ring.n; i++)
	{
		deleteFreeMatchingTree(trees[i]);
	}
	delete[] trees;
}