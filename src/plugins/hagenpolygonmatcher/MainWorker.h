#pragma once

#include "qmath.h"

#include <QObject>
#include <QThread>
#include <QSemaphore>

#include "CoWorker.h"
#include "SpecialWorker.h"
#include "Matching.h"
#include "Polygon.h"
#include "InvertableSymmetry.h"

#include "qgsgeometry.h"

#include <vector>

class MainWorker : public QObject
{
	Q_OBJECT
public:
	MainWorker(volatile bool *aborted);
	~MainWorker() override;
signals:
	void scan(std::vector<MultiPolygon> *polygons, volatile bool *aborted, QSemaphore *finishedsemphore);
private slots:
	void scanslot(std::vector<MultiPolygon> *polygons, volatile bool *aborted, QSemaphore *finishedsemphore);
private:
	QThread thread;
	SpecialWorker specialworker;
	QSemaphore specialsemaphore;
	int nworkers;
	CoWorker **workers;
	QSemaphore workersemaphore;
signals:
	void lines(std::vector<Line> *lines);
	void triangles(std::vector<Triangle> *triangles);
};

extern inline void deleteMultiPolygonVector(std::vector<MultiPolygon> *polygons);