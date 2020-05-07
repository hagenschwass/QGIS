#include "MainWorker.h"

#include "Ring.h"

MainWorker::MainWorker(volatile bool *aborted) :
	nworkers(QThread::idealThreadCount()),
	workers(new CoWorker*[nworkers])
{
	for (int i = 0; i < nworkers; i++)
	{
		workers[i] = new CoWorker(&workersemaphore, aborted);
	}
	moveToThread(&thread);
	connect(this, SIGNAL(scan(std::vector<MultiPolygon> *, volatile bool *, QSemaphore *)), this, SLOT(scanslot(std::vector<MultiPolygon> *, volatile bool *, QSemaphore *)));
	thread.start();
}

MainWorker::~MainWorker()
{
	for (int i = 0; i < nworkers; i++)
	{
		delete workers[i];
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
				SRing2 ring2 = createSRing2(*ring);
				if (ring2.area > 0.0)
				{
					SRing2 inv2 = invertedSRing2(ring2);
					LookupT lookup = computeInvMatching(ring2, inv2, ring2.area * .33, nworkers, workers, &workersemaphore, *aborted);
					deleteMatching(ring2, inv2, lookup);
					deleteSRing2( inv2);
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