#pragma once

#include <QObject>
#include <QAction>

#include "../qgisplugin.h"

#include "Intermediate.h"

class QgsMapLayer;
class QgsVectorLayer;

class HPolygonMatcher : public QObject, public QgisPlugin
{
	Q_OBJECT
public:
	HPolygonMatcher(QgisInterface *interface);
	~HPolygonMatcher() override;

	void initGui() override;
	void unload() override;

private slots:

	void currentLayerChanged(QgsMapLayer *layer);
	void scan();

private:
	QgisInterface *interface;

	QAction *mScanaction, *mSep;

	QgsVectorLayer *mLayer;

	Intermediate *intermediate;

private slots:
	
	void lines(std::vector<Line> *lines);
	void triangles(std::vector<Triangle> *triangles);
	void ring(SRing *ring);

};

