#pragma once

#include "Ring.h"

struct Polygon
{
	SRing *rings;
	int n;
};

extern inline Polygon createPolygon(QgsPolygonXY &qgspolygon);
extern inline void deletePolygon(Polygon &polygon);

struct MultiPolygon
{
	Polygon *polygons;
	int n;
};

extern inline MultiPolygon createMultiPolygon(QgsMultiPolygonXY &qgspolygon);
extern inline void deleteMultiPolygon(MultiPolygon &polygon);
