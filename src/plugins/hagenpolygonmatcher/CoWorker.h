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
	CoWorker(QSemaphore *semaphore, volatile bool *aborted);
	~CoWorker() override;
signals:
	void initlookupinv(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup, double skiparea);
	void matchinv(int basei, int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup);
private slots:
void initlookupinvslot(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup, double skiparea);
void matchinvslot(int basei, int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup);
private:
	QThread thread;
	QSemaphore *semaphore;
	volatile bool *aborted;
};
