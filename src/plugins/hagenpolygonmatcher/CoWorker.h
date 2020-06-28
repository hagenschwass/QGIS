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
	void computesqrts(SRing2 *base, FuncsArg *sqrts);
	void computeatans(SRing2 *base, FuncsArg *atans);
	void initlookupinvforbase(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup, double skiparea);
	void initshortcutsforbase(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup);
	void initshortcutsformatch(int matchi, SRing2 *base, SRing2 *match, LookupArg *lookup);
	void computeshortcutsinv(int basei, int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup);
	void updateexitcosts(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup);
	void matchinv(int basei, int basecut, SRing2 *base, SRing2 *match, FuncsArg *sqrts, FuncsArg *atans, LookupArg *lookup);
	void findbestgate(int basei, int matchi, SRing2 *base, SRing2 *match, ConstraintArg *constraint, LookupArg *lookup, Matching **out);
private slots:
void computesqrtsslot(SRing2 *base, FuncsArg *sqrts);
void computeatansslot(SRing2 *base, FuncsArg *atans);
void initlookupinvforbaseslot(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup, double skiparea);
void initshortcutsforbaseslot(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup);
void initshortcutsformatchslot(int matchi, SRing2 *base, SRing2 *match, LookupArg *lookup);
void computeshortcutsinvslot(int basei, int basecut, SRing2 *base, SRing2 *match, LookupArg *lookup);
void updateexitcostsslot(int basei, SRing2 *base, SRing2 *match, LookupArg *lookup);
void matchinvslot(int basei, int basecut, SRing2 *base, SRing2 *match, FuncsArg *sqrts, FuncsArg *atans, LookupArg *lookup);
void findbestgateslot(int basei, int matchi, SRing2 *base, SRing2 *match, ConstraintArg *constraint, LookupArg *lookup, Matching **out);
private:
	QThread thread;
	QSemaphore *semaphore;
	volatile bool *aborted;
};
