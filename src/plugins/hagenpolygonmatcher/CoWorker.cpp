#include "CoWorker.h"

CoWorker::CoWorker()
{
	moveToThread(&thread);
	thread.start();
}

CoWorker::~CoWorker()
{
	thread.exit();
	thread.wait();
}
