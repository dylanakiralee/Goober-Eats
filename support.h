#include "provided.h"
#include "ExpandableHashMap.h"

// Comparator for the priority_queue utilized in a_star
struct CompareCoord {
	CompareCoord(ExpandableHashMap<GeoCoord, double>* m) {
		f_N = m;
	}
	// Prioritizes smaller fScore values
	bool operator() (const GeoCoord& g1, const GeoCoord& g2) {
		return *f_N->find(g1) > * f_N->find(g2);
	}

	ExpandableHashMap<GeoCoord, double>* f_N; // Map of fScore values
};