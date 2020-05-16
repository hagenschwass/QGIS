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
	void initlookupinvforbase(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup, double skiparea);
	void initshortcutsforbase(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup);
	void initshortcutsformatch(int matchi, SRing2 *base, SRing2 *match, LookupArg *lookup);
	void computeshortcutsinv(int basei, int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup);
	void updateexitcosts(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup);
	void matchinv(int basei, int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup);
private slots:
void initlookupinvforbaseslot(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup, double skiparea);
void initshortcutsforbaseslot(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup);
void initshortcutsformatchslot(int matchi, SRing2 *base, SRing2 *match, LookupArg *lookup);
void computeshortcutsinvslot(int basei, int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup);
void updateexitcostsslot(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup);
void matchinvslot(int basei, int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup);
private:
	QThread thread;
	QSemaphore *semaphore;
	volatile bool *aborted;
};
