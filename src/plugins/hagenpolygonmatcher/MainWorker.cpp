#include "MainWorker.h"

#include "Ring.h"

MainWorker::MainWorker(volatile bool *aborted) :
	specialworker(&specialsemaphore, aborted),
	nworkers(QThread::idealThreadCount()),
	workers(new CoWorker*[nworkers]),
	microworkers(new MicroWorker *[nworkers])
{
	for (int i = 0; i < nworkers; i++)
	{
		workers[i] = new CoWorker(&workersemaphore, aborted);
		microworkers[i] = new MicroWorker(aborted, &workersemaphore);
	}
	moveToThread(&thread);
	connect(this, SIGNAL(scan(std::vector<MultiPolygon> *, volatile bool *, QSemaphore *)), this, SLOT(scanslot(std::vector<MultiPolygon> *, volatile bool *, QSemaphore *)));
	thread.start();
	workersemaphore.acquire(nworkers);
}

MainWorker::~MainWorker()
{
	for (int i = 0; i < nworkers; i++)
	{
		delete workers[i];
		delete microworkers[i];
	}
	thread.exit();
	thread.wait();
}

void MainWorker::scanslot(std::vector<MultiPolygon> *polygons, volatile bool *aborted, QSemaphore *finishedsemphore)
{
	for (MultiPolygon &multipolygon : *polygons)
	{
		for (Polygon *polygon = multipolygon.polygons; polygon != multipolygon.polygons + multipolygon.n; polygon++)
		{
			for (SRing *ring = polygon->rings; ring != polygon->rings + polygon->n; ring++)
			{
				if (ring->n > 2)
				{
					SRing2 ring2 = createSRing2(*ring);
					if (ring2.area > 1e-7)
					{
						SRing2 inv2 = invertedSRing2(ring2);
						MatchingResult result = initMatchingResult();
						LookupT lookup = computeInvMatching(ring2, inv2, ring2.area * .33, result, &specialworker, &specialsemaphore, nworkers, workers, &workersemaphore, *aborted);
						if (*aborted == false && result.matching != nullptr)
						{
							SymmetryMatches sms = computeSymmetryMatchesInv(ring2, inv2, result, lookup);
							if (*aborted == false)
							{
								if (computeSymmetryBeginEndInv(ring2, sms))
								{
									if (*aborted == false)
									{
										orderSymmetryMatchesInv(ring2, sms);
										if (*aborted == false)
										{
											PointMatch *pointmatch = computeBestSymmetryInv(ring2, sms, result.quality);
											if (*aborted == false && pointmatch != nullptr)
											{


												//findSymmetryMatchesGates(sms, ring2, inv2, lookup, nworkers, workers, &workersemaphore, *aborted);
												if (*aborted == false)
												{
												}


												//adjustSymmetryInv(inv2, pointmatch, 1e-11, nworkers, microworkers, &workersemaphore, *aborted);
												if (*aborted == false)
												{
													//updateTurnedIndexesInv(sms);
													SRing symmetricalring = invertableSymmetry2Ring(inv2, pointmatch);
													emit this->ring(new SRing(symmetricalring));
												}
											}
										}
									}
								}
							}
							/*
							InvertableSymmetry sym(ring2, inv2, matching, lookup);
							std::vector<Triangle> *triangles = new std::vector<Triangle>();
							sym.filltriangles(triangles);
							sym.fillmatchtriangles(triangles);
							emit this->triangles(triangles);
*/
							deleteSymmetryMatches(sms);
							deleteMatching(ring2, inv2, lookup);
						}
						else
						{
							deleteMatching(ring2, inv2, lookup);
						}
						deleteSRing2( inv2);
					}
					if (*aborted)
					{
						break;
					}
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

	finishedsemphore->release();

	deleteMultiPolygonVector(polygons);
}

inline void deleteMultiPolygonVector(std::vector<MultiPolygon> *polygons)
{
	for (MultiPolygon &multipolygon : *polygons)
	{
		deleteMultiPolygon(multipolygon);
	}
	delete polygons;
}