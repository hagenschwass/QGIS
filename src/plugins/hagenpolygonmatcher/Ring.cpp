#include "ring.h"

#include "qgspointxy.h"

inline void computeArea(Ring ring, int n, double &area)
{
	area = 0.0;
	Point &p1 = ring[0];
	for (int i = 1, i2 = 2; i2 < n; i = i2, i2++)
	{
		Point &p2 = ring[i], &p3 = ring[i2];
		double &p1x = p1.x, &p1y = p1.y, &p2x = p2.x, &p2y = p2.y, &p3x = p3.x, &p3y = p3.y;
		double p1yp2y = p1y - p2y, p1xp3x = p1x - p3x, p1yp3y = p1y - p3y, p2xp1x = p2x - p1x;
		area += .5 * (p1yp2y * p1xp3x + p1yp3y * p2xp1x);
	}
}

inline void comupteCenterAndArea(Ring ring, Point &center, double &area)
{

}

inline SRing createSRing(QgsPolylineXY &qgsring)
{
	SRing result;
	result.n = qgsring.size() - 1;
	result.ring = new Point[result.n];
	for (int i = 0; i < result.n; i++)
	{
		result.ring[i] = { qgsring[i].x(), qgsring[i].y() };
	}
	return result;
}

inline void deleteSRing(SRing &ring)
{
	delete[] ring.ring;
}

inline SRing2 createSRing2(SRing &ring)
{
	SRing2  result{ ring };
	computeArea(ring.ring, ring.n, result.area);
	return result;
}

inline void deleteSRing2(SRing2 &ring)
{
	deleteSRing(ring.ring);
}

inline SRing2 invertedSRing2(SRing2 &ring2)
{
	SRing2 result;
	result.ring.n = ring2.ring.n;
	result.ring.ring = new Point[result.ring.n];
	for (int i = 0; i < result.ring.n; i++)
	{
		result.ring.ring[i] = ring2.ring.ring[result.ring.n - i - 1];
	}
	result.area = ring2.area;
	return result;
}