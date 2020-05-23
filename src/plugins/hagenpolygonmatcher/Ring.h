#pragma once

#include "qmath.h"

#include "qgsgeometry.h"

struct Point
{
	double x, y;
};

extern inline Point operator*(const double &d, const Point &p);
extern inline Point operator+(const Point &p1, const Point &p2);

typedef Point* Ring;

extern inline void computeArea(Ring ring, int n, double &area);
extern inline void comupteCenterAndArea(Ring ring, Point &center, double &area);

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
extern inline SRing cloneSRing(SRing &ring);
extern inline void swapSRings(SRing &ring1, SRing &ring2);
extern inline void deleteSRing(SRing &ring);

struct SRing2
{
	SRing ring;
	double area;
};

extern inline SRing2 createSRing2(SRing &ring);
extern inline void deleteSRing2(SRing2 &ring);

extern inline SRing2 invertedSRing2(SRing2 &ring2);