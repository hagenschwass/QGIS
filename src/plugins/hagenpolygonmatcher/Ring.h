#pragma once

#include "qmath.h"

#include "qgsgeometry.h"

struct Point
{
	double x, y;
};

typedef Point* Ring;

inline void computeArea(Ring ring, int n, double &area);
inline void comupteCenterAndArea(Ring ring, Point &center, double &area);

struct Line
{
	Point p1, p2;
};

struct Triangle
{
	Point p1, p2, p3;
};

struct SRing
{
	Ring ring;
	int n;
};

extern inline SRing createSRing(QgsPolylineXY &qgsring);
extern inline void deleteSRing(SRing &ring);

struct SRing2
{
	SRing ring;
	double area;
};

extern inline SRing2 createSRing2(SRing &ring);
extern inline void deleteSRing2(SRing2 &ring);

extern inline SRing2 invertedSRing2(SRing2 &ring2);