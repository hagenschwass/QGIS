#include "matching.h"

#include "CoWorker.h"
#include "SpecialWorker.h"

#include <cfloat>

inline MatchingResult initMatchingResult()
{
	return{ nullptr, nullptr, 0.0, -DBL_MAX, nullptr };
}

inline void deleteMatchingResult(MatchingResult &result)
{
	delete[] result.constraint;
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
		freematching[i].left = privateBuildFreeMatchingTree(matching->leftback, freematching, index);
		freematching[i].right = privateBuildFreeMatchingTree(matching->rightback, freematching, index);
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

inline void deleteFreeMatchingTree(FreeMatchingTree &tree)
{
	delete[] tree.up;
	delete[] tree.down;
}