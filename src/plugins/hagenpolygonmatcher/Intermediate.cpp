#include "Intermediate.h"

Intermediate::Intermediate() :
	worker(&aborted)
{
	moveToThread(&thread);
	connect(this, SIGNAL(abort()), this, SLOT(abortslot()));
	connect(this, SIGNAL(scan(std::vector<MultiPolygon> *)), this, SLOT(scanslot(std::vector<MultiPolygon> *)));
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

void Intermediate::scanslot(std::vector<MultiPolygon> *polygons)
{
	aborted = false;
	worker.scan(polygons, &aborted, &semaphore);
}