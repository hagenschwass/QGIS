#include "matching.h"

#include "CoWorker.h"
#include "SpecialWorker.h"

#include <cfloat>

inline MatchingResult initMatchingResult()
{
	return{ nullptr, nullptr, 0.0, -DBL_MAX };
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

inline Matching* getmatchingmod(LookupT lookup, SRing2 &base, SRing2 &match, int base1, int match1, int base2mod, int match2mod)
{
	Lookup &lookupl = lookup[base1][base2mod][match1];
	if (match2mod < match1)
	{

	}
	else
	{
	}
	return nullptr;
}

/*
inline Matching* getoppositematching(LookupT lookup, SRing2 &base, SRing2 &match, Matching *matching)
{
	Lookup &lookupl = lookup[matching->base2 % base.ring.n][matching->base1][matching->match2 % match.ring.n];
	int match1 = matching->match1;
	if (lookupl.end >= match.ring.n)
	{
		if (lookupl.begin >= match.ring.n)
		{
			int match1up = match1 + match.ring.n;
			if (match1up < lookupl.begin || match1up > lookupl.end) return nullptr;
			return &lookupl.matching[match1up - lookupl.begin];
		}
		else
		{
			if (match1 >= lookupl.begin)
			{
				return &lookupl.matching[match1 - lookupl.begin];
			}
			else
			{
				int match1up = match1 + match.ring.n;
				if (match1up > lookupl.end) return nullptr;
				return &lookupl.matching[match1up - lookupl.begin];
			}
		}
	}
	else
	{
		if (match1 < lookupl.begin || match1 > lookupl.end) return nullptr;
		return &lookupl.matching[match1 - lookupl.begin];
	}
}*/