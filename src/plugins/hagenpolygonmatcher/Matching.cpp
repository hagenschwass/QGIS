#include "matching.h"

#include "CoWorker.h"

#include <cfloat>

/*O(n^6)*/
inline Lookup computeInvMatching(SRing2 &base, SRing2 &match, int nworkers, CoWorker** workers, QSemaphore *semaphore, volatile bool &aborted)
{
	Lookup lookup = new Matching***[base.ring.n];
	for (int i = 0; i < base.ring.n; i++)
	{
		Matching ***&base1 = lookup[i];
		base1 = new Matching **[base.ring.n];
		for (int j = 0; j < base.ring.n; j++)
		{
			Matching **&base2 = base1[j];
			base2 = new Matching*[match.ring.n];
			for (int k = 0; k < match.ring.n; k++)
			{
				Matching *&match1 = base2[k];
				match1 = new Matching[match.ring.n];
				for (int l = 0; l < match.ring.n; l++)
				{
					match1[l].quality = -DBL_MAX;
					match1[l].leftback = nullptr;
					match1[l].rightback = nullptr;
				}
			}
		}
	}
	if (aborted)
	{
		return lookup;
	}

	int currentworker = 0;

	for (int basei = 0; basei < base.ring.n; basei++)
	{
		workers[currentworker]->initbaseperimeter(basei, &base, &match, &lookup, &aborted);
		currentworker = (currentworker + 1) % nworkers;
	}
	semaphore->acquire(base.ring.n);
	if (aborted)
	{
		return lookup;
	}
	for (int matchi = 0; matchi < match.ring.n; matchi++)
	{
		workers[currentworker]->initmatchperimeter(matchi, &base, &match, &lookup, &aborted);
		currentworker = (currentworker + 1) % nworkers;
	}
	if (aborted == false)
	{
		for (int basei = 0; basei < base.ring.n; basei++)
		{
			for (int matchi = 0; matchi < match.ring.n; matchi++)
			{
				lookup[basei][(basei + 1) % base.ring.n][matchi][(matchi + 1) % match.ring.n].quality = 0.0;
			}
		}
	}
	semaphore->acquire(match.ring.n);

	if (aborted)
	{
		return lookup;
	}

	for (int basecut = 2; basecut < base.ring.n; basecut++)
	{


		for (int basei = 0; basei < base.ring.n; basei++)
		{

			workers[currentworker]->matchinv(/**/basei, basecut, &base, &match, &lookup, &aborted);
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


	return lookup;
}

inline void deleteMatching(SRing2 &base, SRing2 &match, Lookup lookup)
{
	for (int i = 0; i < base.ring.n; i++)
	{
		Matching ***base1 = lookup[i];
		for (int j = 0; j < base.ring.n; j++)
		{
			Matching **base2 = base1[j];
			for (int k = 0; k < match.ring.n; k++)
			{
				Matching *match1 = base2[k];
				delete[] match1;
			}
			delete[] base2;
		}
		delete[] base1;
	}
	delete[] lookup;
}