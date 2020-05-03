#pragma once

#include <QObject>
#include <QThread>
#include <QSemaphore>

#include "Ring.h"
#include "Matching.h"

class CoWorker : public QObject
{
	Q_OBJECT
public:
	CoWorker(QSemaphore *semaphore);
	~CoWorker() override;
signals:
	void initbaseperimeter(int basei, SRing2 *base, SRing2 *match, Lookup *lookup, volatile bool *aborted);
	void initmatchperimeter(int matchi, SRing2 *base, SRing2 *match, Lookup *lookup, volatile bool *aborted);
	void matchinv(int basei, int basecut, SRing2 *base, SRing2 *match, Lookup *lookup, volatile bool *aborted);
private slots:
void initbaseperimeterslot(int basei, SRing2 *base, SRing2 *match, Lookup *lookup, volatile bool *aborted);
void initmatchperimeterslot(int matchi, SRing2 *base, SRing2 *match, Lookup *lookup, volatile bool *aborted);
void matchinvslot(int basei, int basecut, SRing2 *base, SRing2 *match, Lookup *lookup, volatile bool *aborted);
private:
	QThread thread;
	QSemaphore *semaphore;
};
