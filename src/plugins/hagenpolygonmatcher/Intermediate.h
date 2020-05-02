#pragma once

#include "qmath.h"

#include <QObject>
#include <QThread>
#include <QSemaphore>

#include "MainWorker.h"

#include "qgsgeometry.h"

class Intermediate : public QObject
{
	Q_OBJECT
public:
	Intermediate();
	~Intermediate() override;
signals:
	void abort();
	void scan(QVector<QgsPolygonXY> *polygons);
private slots:
	void abortslot();
	void scanslot(QVector<QgsPolygonXY> *polygons);
private:
	QThread thread;
	volatile bool aborted;
	QSemaphore semaphore;
	MainWorker worker;
};