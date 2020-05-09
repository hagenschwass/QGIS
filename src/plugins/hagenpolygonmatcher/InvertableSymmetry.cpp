#include "InvertableSymmetry.h"

InvertableSymmetry::InvertableSymmetry(SRing2 &base, SRing2 &match, Matching *matching, LookupT lookup) :
	Symmetry(base, match, matching, lookup)
{
}

void fillmatchlinesrec(int *stuff, Matching *matching)
{
	if (matching->leftback != nullptr)
	{
		stuff[matching->rightback->base1] = matching->rightback->match1;
		fillmatchlinesrec(stuff, matching->leftback);
		fillmatchlinesrec(stuff, matching->rightback);
	}
}

void InvertableSymmetry::fillmatchlines(std::vector<Line> *lines)
{
	int *stuff = new int[base.ring.n];
	memset(stuff, -1, sizeof(int) * base.ring.n);
	stuff[matching->base1] = matching->match1;
	stuff[matching->base2 % match.ring.n] = matching->match2 % match.ring.n;
	fillmatchlinesrec(stuff, matching);
	int gap = base.ring.n;
	int start;
	for (int i = 0; i < base.ring.n; i++)
	{
		int j = stuff[i];
		if (j > -1)
		{
			j = base.ring.n - j - 1;
			if ((i - j)*(i - j) < gap * gap)
			{
				gap = i - j;
				if (j > i) start = j;
				else start = i;
			}
		}
	}
	for (int i = start; i < start + base.ring.n; i++)
	{
		int imod = i % base.ring.n;
		int j = stuff[imod];
		if (j > -1)
		{
			j = base.ring.n - j - 1;
			stuff[j] = -1;
			lines->push_back({ base.ring.ring[imod], base.ring.ring[j] });
		}
	}
	delete[] stuff;
}

void filltrianglesrect(Matching *matching, SRing2 &base, std::vector<Triangle> *triangles)
{
	if (matching->rightback)
	{
		triangles->push_back({ base.ring.ring[matching->base1], base.ring.ring[matching->base2 % base.ring.n], base.ring.ring[matching->rightback->base1] });
		filltrianglesrect(matching->rightback, base, triangles);
		filltrianglesrect(matching->leftback, base, triangles);
	}
}

void InvertableSymmetry::filltriangles(std::vector<Triangle> *triangles)
{
	filltrianglesrect(matching, base, triangles);
}