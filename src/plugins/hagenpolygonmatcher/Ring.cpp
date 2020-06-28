#include "ring.h"

#include "qgspointxy.h"

double H_PI = acos(-1);
double H_2_PI = 2. * H_PI;

inline Funcs createFuncs(int n)
{
	Funcs funcs = new FuncsArg[n];
	for (int i = 0; i < n; i++)
	{
		funcs[i] = new double[n];
	}
	return funcs;
}

inline void deleteFuncs(Funcs funcs, int n)
{
	for (int i = 0; i < n; i++)
	{
		delete[] funcs[i];
	}
	delete[] funcs;
}

inline Point operator*(const double &d, const Point &p)
{
	return{ d * p.x, d * p.y };
}

inline Point operator+(const Point &p1, const Point &p2)
{
	return{ p1.x + p2.x, p1.y + p2.y };
}

inline Point operator-(const Point &p1, const Point &p2)
{
	return{ p1.x - p2.x, p1.y - p2.y };
}

inline Matrix operator >> (const Point &from, const Point &to)
{
	const double &x = from.x, &y = from.y, &_x = to.x, &_y = to.y;
	double xsq = x * x, ysq = y * y;
	double a11 = (_x * x + _y * y) / (xsq + ysq);
	double a21 = (_y * x - _x * y) / (xsq + ysq);
	if (abs(x) < abs(y))
		return Matrix{ { {a11, ((_x - a11 * x) / y)}, {a21, ((_y - a21 * x) / y)} } };
	return Matrix{ { {a11, ((a11 * y - _y) / x)}, {a21, ((_x + a21 * y) / x)} } };
}

inline Point operator*(const Matrix &m, const Point &p)
{
	return{ m.a[0][0] * p.x + m.a[0][1] * p.y, m.a[1][0] * p.x + m.a[1][1] * p.y };
}

inline Point operator*(const Transform &t, const Point &p)
{
	return (t.m * p) + t.t;
}

inline Transform transform(const Point &from, const Point &to, const Point &about)
{
	Point vfrom = from - about, vto = to - about;
	Matrix m = vfrom >> vto;
	return{ m, about - (m * about) };
}

inline void computeArea(Ring ring, int n, double &area)
{
	area = 0.0;
	Point &p1 = ring[0];
	for (int i = 1, i2 = 2; i2 < n; i = i2, i2++)
	{
		Point &p2 = ring[i], &p3 = ring[i2];
		double &p1x = p1.x, &p1y = p1.y, &p2x = p2.x, &p2y = p2.y, &p3x = p3.x, &p3y = p3.y;
		double p1yp2y = p1y - p2y, p1xp3x = p1x - p3x, p1yp3y = p1y - p3y, p2xp1x = p2x - p1x;
		double larea = .5 * (p1yp2y * p1xp3x + p1yp3y * p2xp1x);
		area += larea;
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

inline SRing cloneSRing(SRing &ring)
{
	SRing result{ new Point[ring.n], ring.n };
	memcpy_s(result.ring, sizeof(Point) * ring.n, ring.ring, sizeof(Point) * ring.n);
	return result;
}

inline void swapSRings(SRing &ring1, SRing &ring2)
{
	SRing save = ring2;
	ring2 = ring1;
	ring1 = save;
}

inline void swapSRingsInv(SRing &ring, SRing &inv)
{
	for (int i = 0; i < ring.n; i++)
	{
		Point heap = ring.ring[i];
		ring.ring[i] = inv.ring[inv.n - i - 1];
		inv.ring[inv.n - i - 1] = heap;
	}
}

inline void meanSRings(SRing &r1, SRing &r2)
{
	for (int i = 0; i < r1.n; i++)
	{
		Point mean = .5 * (r1.ring[i] + r2.ring[i]);
		r1.ring[i] = mean;
		r2.ring[i] = mean;
	}
}

inline void meanSRingsInv(SRing &ring, SRing &inv)
{
	for (int i = 0; i < ring.n; i++)
	{
		Point mean = .5 * (ring.ring[i] + inv.ring[inv.n - i - 1]);
		ring.ring[i] = mean;
		inv.ring[inv.n - i - 1] = mean;
	}
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

inline SRing2 cloneSRing2(SRing2 &ring)
{
	return{ cloneSRing(ring.ring), ring.area };
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
	result.area = -ring2.area;
	return result;
}

inline void swapSRing2sInv(SRing2 &ring, SRing2 &inv)
{
	swapSRingsInv(ring.ring, inv.ring);
}

inline void meanSRing2sInv(SRing2 &ring, SRing2 &inv)
{
	meanSRingsInv(ring.ring, inv.ring);
}
