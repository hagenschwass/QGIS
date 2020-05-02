#pragma once

#include "qmath.h"

#include <QObject>
#include <QThread>
#include <QSemaphore>

#include "CoWorker.h"
#include "Matching.h"

#include "qgsgeometry.h"

class MainWorker : public QObject
{
	Q_OBJECT
public:
	MainWorker();
	~MainWorker() override;
signals:
	void scan(QVector<QgsPolygonXY> *polygons, volatile bool *aborted, QSemaphore *finishedsemphore);
private slots:
	void scanslot(QVector<QgsPolygonXY> *polygons, volatile bool *aborted, QSemaphore *finishedsemphore);
private:
	QThread thread;

};