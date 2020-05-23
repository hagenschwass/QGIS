#pragma once

#include <QObject>
#include <QThread>
#include <QSemaphore>

#include "Ring.h"
#include "Matching.h"

class SpecialWorker : public QObject
{
	Q_OBJECT
public:
	SpecialWorker(QSemaphore *semaphore, volatile bool *aborted);
	~SpecialWorker() override;
signals:
	void searchbestmatch(int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup, MatchingResult*);
private slots:
	void searchbestmatchslot(int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup, MatchingResult *result);
private:
	QThread thread;
	QSemaphore *semaphore;
	volatile bool *aborted;
};
