#include "InvertableSymmetry.h"
#include "CoWorker.h"

#include <QSemaphore>


void fillmatchlinesrec(SymmetryMatches &matches, Matching *matching, int n)
{
	if (matching->leftback != nullptr)
	{
		int base = matching->rightback->base1, match = n - matching->rightback->match1 - 1;
		matches.match[base].base.base = base;
		matches.match[base].match = match;
		matches.constraint[base] = matching->rightback->match1;
		appendPointMatch(matches.base[base], matches.match[base]);
		fillmatchlinesrec(matches, matching->leftback, n);
		fillmatchlinesrec(matches, matching->rightback, n);
	}
}

inline SymmetryMatches computeSymmetryMatchesInv(SRing2 &base, SRing2 &match, Matching *matching, LookupT lookup)
{
	SymmetryBase sb = new BaseMatch[base.ring.n];
	SymmetryMatch sm = new PointMatch[base.ring.n];
	Constraint constraint = new int[base.ring.n];
	SymmetryMatches matches = { sb, sm, constraint, -1, -1 };

	for (int i = 0; i < base.ring.n; i++)
	{
		constraint[i] = -1;
		sm[i].base.base = -1;
		sm[i].backptr = nullptr;
		sm[i].quality = 0.0;
		sm[i].gate = nullptr;
		sb[i].base = i;
		sb[i].next = sb + i;
		sb[i].prev = sb + i;
	}

	sm[matching->base1].match = base.ring.n - matching->match1 - 1;
	sm[matching->base1].base.base = matching->base1;
	constraint[matching->base1] = matching->match1;
	appendPointMatch(sb[matching->base1], sm[matching->base1]);
	sm[matching->base2 % base.ring.n].match = base.ring.n - (matching->match2 % base.ring.n) - 1;
	sm[matching->base2 % base.ring.n].base.base = matching->base2 % base.ring.n;
	constraint[matching->base2 % base.ring.n] = matching->match2 % base.ring.n;
	appendPointMatch(sb[matching->base2 % base.ring.n], sm[matching->base2 % base.ring.n]);

	fillmatchlinesrec(matches, matching, base.ring.n);
	fillmatchlinesrec(matches, getoppositematching(lookup, base, match, matching), base.ring.n);

	return matches;
}

inline void findSymmetryMatchesGates(SymmetryMatches &matches, SRing2 &base, SRing2 &match, LookupT lookup, int nworkers, CoWorker** workers, QSemaphore *semaphore, volatile bool &aborted)
{
	int currentworker = 0;
	int numjobs = 0;
	for (int i = 0; i < base.ring.n; i++)
	{
		BaseMatch &basematch = matches.base[i];
		for (BaseMatch *bm = basematch.next; bm != &basematch; bm = bm->next)
		{
			PointMatch *pm = reinterpret_cast<PointMatch*>(bm);

			workers[currentworker]->findbestgate(i, base.ring.n - pm->match - 1, &base, &match, matches.constraint, lookup, &pm->gate);
			currentworker = (currentworker + 1) % nworkers;
			numjobs++;
		}
		if (aborted)
		{
			break;
		}
	}
	semaphore->acquire(numjobs);
}

inline bool computeSymmetryBeginEndInv(SRing2 &base, SymmetryMatches &matches)
{
	for (int ib = 0; ib < base.ring.n; ib++)
	{
		if (matches.base[ib].next != &matches.base[ib])
		{
			if (matches.match[ib].match <= ib)
			{
				bool shift = false;
				int lastmatch = ib;
				for (int ie = base.ring.n - 1; ie > ib; ie--)
				{
					if (matches.base[ie].next != &matches.base[ie])
					{
						int match = matches.match[ie].match;
						if (shift)
						{
							match += base.ring.n;
						}
						else
						{
							if (match < lastmatch)
							{
								shift = true;
								match += base.ring.n;
							}
							else
							{
								lastmatch = match;
							}
						}
						if (match >= ie)
						{
							matches.ie = ie;
							break;
						}
					}
				}
				matches.ib = ib;
				break;
			}
		}
	}
	return matches.ib > -1 && matches.ie > -1;
}

inline void orderSymmetryMatchesInv(SRing2 &base, SymmetryMatches &matches)
{
	for (int i = 0; i < matches.ib; i++)
	{
		PointMatch &match = matches.match[i];
		if (match.base.base > -1)
		{
			//leavePointMatch(match);
			BaseMatch &basematch = matches.base[match.match];
			appendPointMatch(basematch, match);
			match.base.base = match.match;
			match.match = i;
		}
	}
	for (int i = matches.ie + 1; i < base.ring.n; i++)
	{
		PointMatch &match = matches.match[i];
		if (match.base.base > -1)
		{
			//leavePointMatch(match);
			BaseMatch &basematch = matches.base[match.match];
			appendPointMatch(basematch, match);
			match.base.base = match.match;
			match.match = i;
		}
	}
}

#define MAXRELAX			7

inline PointMatch *computeBestSymmetryInv(SRing2 &base, SymmetryMatches &matches, LookupT lookup)
{
	double maxquality = -DBL_MAX;
	PointMatch *maxqualitymatch = nullptr;
	for (int i = matches.ib; i < matches.ie; i++)
	{
		int iinv = base.ring.n - i - 1;
		for (BaseMatch *bm = matches.base[i].next; bm != &matches.base[i]; bm = bm->next)
		{
			PointMatch *pm = reinterpret_cast<PointMatch*>(bm);
			if (pm->gate == nullptr) continue;
			int match = pm->match, matchinv = base.ring.n - match - 1;

			double quality = pm->quality + pm->gate->quality;

			int targetsrelaxed = 0;
			for (int target = i + 1; target <= matches.ie && targetsrelaxed <= MAXRELAX; target++)
			{
				for (BaseMatch *targetbm = matches.base[target].next; targetbm != &matches.base[target]; targetbm = targetbm->next)
				{
					PointMatch *targetpm = reinterpret_cast<PointMatch*>(targetbm);
					if (targetpm->gate == nullptr) continue;
					int targetmatch = targetpm->match >= target ? targetpm->match - base.ring.n : targetpm->match;
					if (targetmatch < match)
					{
						if (quality > targetpm->quality)
						{
							targetpm->quality = quality;
							targetpm->backptr = pm;
							double exitquality = quality + targetpm->gate->quality;
							if (exitquality > maxquality)
							{
								maxquality = exitquality;
								maxqualitymatch = targetpm;
							}
						}
						targetsrelaxed++;
					}
				}
			}
		}
	}
	return maxqualitymatch;
}

inline SRing invertableSymmetry2Ring(SRing2 &base, PointMatch *pointmatch)
{
	int count = 0;
	for (PointMatch *run = pointmatch; run != nullptr; run = run->backptr)
	{
		count += run->base.base == run->match ? 1 : 2;
	}
	SRing result = { new Point[count], count };
	int i = 0;
	for (PointMatch *run = pointmatch; run != nullptr; run = run->backptr)
	{
		result.ring[i++] = base.ring.ring[run->match];
	}
	i = 1;
	for (PointMatch *run = pointmatch; run != nullptr; run = run->backptr)
	{
		if (run->base.base != run->match)
		{
			result.ring[count - i] = base.ring.ring[run->base.base];
			i++;
		}
	}
	return result;
}

InvertableSymmetry::InvertableSymmetry(SRing2 &base, SRing2 &match, Matching *matching, LookupT lookup) :
	Symmetry(base, match, matching, lookup)
{
}

InvertableSymmetry::~InvertableSymmetry()
{
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
	filltrianglesrect(getoppositematching(lookup, base, match, matching), base, triangles);
}

void fillmatchtrianglesrect(Matching *matching, SRing2 &match, std::vector<Triangle> *triangles)
{
	if (matching->rightback)
	{
		triangles->push_back({ match.ring.ring[matching->match1], match.ring.ring[matching->match2 % match.ring.n], match.ring.ring[matching->rightback->match1] });
		fillmatchtrianglesrect(matching->rightback, match, triangles);
		fillmatchtrianglesrect(matching->leftback, match, triangles);
	}
}

void InvertableSymmetry::fillmatchtriangles(std::vector<Triangle> *triangles)
{
	fillmatchtrianglesrect(matching, match, triangles);
	fillmatchtrianglesrect(getoppositematching(lookup, base, match, matching), match, triangles);
}