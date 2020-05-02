#include "MainWorker.h"

MainWorker::MainWorker()
{
	moveToThread(&thread);
	connect(this, SIGNAL(scan(QVector<QgsPolygonXY> *, volatile bool *, QSemaphore *)), this, SLOT(scanslot(QVector<QgsPolygonXY> *, volatile bool *, QSemaphore *)));
	thread.start();
}

MainWorker::~MainWorker()
{
	thread.exit();
	thread.wait();
}

void MainWorker::scanslot(QVector<QgsPolygonXY> *polygons, volatile bool *aborted, QSemaphore *finishedsemphore)
{
	for (QgsPolygonXY &polygon : *polygons)
	{
		for (QgsPolylineXY &ring : polygon)
		{

			Matching *matching = new Matching();
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
	delete polygons;
}