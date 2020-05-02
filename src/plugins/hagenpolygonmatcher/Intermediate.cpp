#include "Intermediate.h"

Intermediate::Intermediate()
{
	moveToThread(&thread);
	connect(this, SIGNAL(abort()), this, SLOT(abortslot()));
	connect(this, SIGNAL(scan(QVector<QgsPolygonXY> *)), this, SLOT(scanslot(QVector<QgsPolygonXY> *)));
	thread.start();
	semaphore.release();
}

Intermediate::~Intermediate()
{
	aborted = true;
	thread.exit();
	thread.wait();
}

void Intermediate::abortslot()
{
	aborted = true;
	semaphore.acquire();
}

void Intermediate::scanslot(QVector<QgsPolygonXY> *polygons)
{
	aborted = false;
	worker.scan(polygons, &aborted, &semaphore);
}