#pragma once

#include <QObject>
#include <QThread>
#include <QSemaphore>

class CoWorker : public QObject
{
	Q_OBJECT
public:
	CoWorker();
	~CoWorker() override;
private:
	QThread thread;
};
