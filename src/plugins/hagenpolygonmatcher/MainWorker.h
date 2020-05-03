#pragma once

#include "qmath.h"

#include <QObject>
#include <QThread>
#include <QSemaphore>

#include "CoWorker.h"
#include "Matching.h"
#include "Polygon.h"

#include "qgsgeometry.h"

#include <vector>

class MainWorker : public QObject
{
	Q_OBJECT
public:
	MainWorker();
	~MainWorker() override;
signals:
	void scan(std::vector<MultiPolygon> *polygons, volatile bool *aborted, QSemaphore *finishedsemphore);
private slots:
	void scanslot(std::vector<MultiPolygon> *polygons, volatile bool *aborted, QSemaphore *finishedsemphore);
private:
	QThread thread;
	int nworkers;
	CoWorker **workers;
	QSemaphore workersemaphore;
};

extern inline void deleteMultiPolygonVector(std::vector<MultiPolygon> *polygons);