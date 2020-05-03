#include "Polygon.h"

inline Polygon createPolygon(QgsPolygonXY &qgspolygon)
{
	Polygon result;
	result.n = qgspolygon.size();
	result.rings = new SRing[result.n];
	for (int i = 0; i < result.n; i++)
	{
		result.rings[i] = createSRing(qgspolygon[i]);
	}
	return result;
}

inline void deletePolygon(Polygon &polygon)
{
	for (int i = 0; i < polygon.n; i++)
	{
		deleteSRing(polygon.rings[i]);
	}
	delete[] polygon.rings;
}

extern inline MultiPolygon createMultiPolygon(QgsMultiPolygonXY &qgspolygon)
{
	MultiPolygon result;
	result.n = qgspolygon.size();
	result.polygons = new Polygon[result.n];
	for (int i = 0; i < result.n; i++)
	{
		result.polygons[i] = createPolygon(qgspolygon[i]);
	}
	return result;
}

extern inline void deleteMultiPolygon(MultiPolygon &polygon)
{
	for (int i = 0; i < polygon.n; i++)
	{
		deletePolygon(polygon.polygons[i]);
	}
	delete[] polygon.polygons;
}
