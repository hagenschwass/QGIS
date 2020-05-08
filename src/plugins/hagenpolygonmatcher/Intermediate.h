#pragma once

#include "qmath.h"

#include <QObject>
#include <QThread>
#include <QSemaphore>

#include "MainWorker.h"

#include "qgsgeometry.h"

#include <vector>

class Intermediate : public QObject
{
	Q_OBJECT
public:
	Intermediate();
	~Intermediate() override;
signals:
	void abort();
	void scan(std::vector<MultiPolygon> *polygons);
private slots:
	void abortslot();
	void scanslot(std::vector<MultiPolygon> *polygons);
private:
	QThread thread;
	volatile bool aborted;
	QSemaphore semaphore;
	MainWorker worker;
	friend class HPolygonMatcher;
};