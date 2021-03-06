#include "provided.h"
#include "ExpandableHashMap.h"
#include "support.h"
#include <list>
#include <queue>
#include <map>
#include <set>
#include <unordered_set>
using namespace std;

class PointToPointRouterImpl
{
public:
	PointToPointRouterImpl(const StreetMap* sm);
	~PointToPointRouterImpl();
	DeliveryResult generatePointToPointRoute(
		const GeoCoord& start,
		const GeoCoord& end,
		list<StreetSegment>& route,
		double& totalDistanceTravelled) const;
private:
	const StreetMap* m_streetMap;

	bool a_star(GeoCoord start, GeoCoord end, list<StreetSegment>& routedPath) const;
	void getPath(const ExpandableHashMap<GeoCoord, GeoCoord>& closed, GeoCoord start, GeoCoord current, list<StreetSegment>& routedPath) const;
	double getDistance(const list <StreetSegment>& s) const;
	bool checkCoord(const GeoCoord& g1, const GeoCoord& g2) const;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
{
	m_streetMap = sm;
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
	const GeoCoord& start,
	const GeoCoord& end,
	list<StreetSegment>& route,
	double& totalDistanceTravelled) const
{
	// Clears route and totalDistanceTravelled
	route.clear();
	totalDistanceTravelled = 0;
	// If can't find the start and end StreetSegments, then return BAD_COORD
	vector<StreetSegment> segs;
	if (!(m_streetMap->getSegmentsThatStartWith(start, segs) && m_streetMap->getSegmentsThatStartWith(end, segs)))
		return BAD_COORD;
	if (start == end) {
		return DELIVERY_SUCCESS;
	}
	// If a_star can find a route, return DELIVERY_SUCCESS
	if (a_star(start, end, route)) {
		totalDistanceTravelled = getDistance(route);
		return DELIVERY_SUCCESS;
	}
	// Else, return NO_ROUTE
	return NO_ROUTE;
}

bool PointToPointRouterImpl::a_star(GeoCoord start, GeoCoord end, list<StreetSegment>& routedPath) const {
	ExpandableHashMap<GeoCoord, double> gScore; // Stores gScores (cost up to a particular GeoCoord)
	ExpandableHashMap<GeoCoord, double> fScore; // Stores fScores (cost up to a particular GeoCoord plus a heuristic (Euclidian distance))
	gScore.associate(start, 0);
	fScore.associate(start, 0);
	priority_queue<GeoCoord, vector<GeoCoord>, CompareCoord> pq(&fScore); // Create a priority_queue with the CompareCoord comparator
	set<GeoCoord> open; // Stores what values are in the priority_queue (because a priority_queue cannot be searched)
	ExpandableHashMap<GeoCoord, GeoCoord> path; // Stores the path travelled

	// Add the starting GeoCoord to pq and open
	pq.push(start);
	open.insert(start);

	// While the pq is not empty
	GeoCoord previous;
	while (!pq.empty()) {
		// Retrieve and pop the first value off the pq
		GeoCoord cur = pq.top();
		pq.pop();
		open.erase(cur);

		// If the current GeoCoord equals the destination, generate a list of StreetSegments and return true
		if (cur == end) {
			getPath(path, start, cur, routedPath);
			return true;
		}

		// Set previous to the current GeoCoord
		previous.latitudeText = cur.latitudeText; previous.longitudeText = cur.longitudeText;
		previous.latitude = stoi(previous.latitudeText); previous.longitude = stoi(previous.longitudeText);

		// Get the neighboring GeoCoords of cur
		vector<StreetSegment> neighbors;
		m_streetMap->getSegmentsThatStartWith(cur, neighbors);
		// Loop through the neighbors
		for (vector<StreetSegment>::iterator itr = neighbors.begin(); itr != neighbors.end(); itr++) {
			GeoCoord neighbor = itr->end;
			// Calculate a potential gScore for the current neighbor
			double tentative_gScore = *gScore.find(cur) + distanceEarthMiles(cur, neighbor);

			// If a key-value pair doesn't exist yet for neighbor, or the potential gScore is less than the current one,
			if (gScore.find(neighbor) == nullptr || tentative_gScore < *gScore.find(neighbor)) {
				// Add neighbor-cur key-value pair to path
				path.associate(neighbor, cur);
				// Associate a gScore and fScore for neighbor
				gScore.associate(neighbor, tentative_gScore);
				fScore.associate(neighbor, *gScore.find(neighbor) + distanceEarthMiles(cur, end));

				// If neighbor is not in open (and therefore pq), add to both
				if (open.find(neighbor) == open.end()) {
					open.insert(neighbor);
					pq.push(neighbor);
				}
			}
		}
	}
	return false;
}

void PointToPointRouterImpl::getPath(const ExpandableHashMap<GeoCoord, GeoCoord>& closed, GeoCoord start, GeoCoord end, list<StreetSegment>& routedPath) const {
	GeoCoord cur = end;
	for (;;) {
		// Find the GeoCoord that preceeded cur
		const GeoCoord cameFrom = *closed.find(cur);
		// Get the GeoCoords that start with cameFrom
		vector<StreetSegment> currentSegs;
		m_streetMap->getSegmentsThatStartWith(cameFrom, currentSegs);
		for (vector<StreetSegment>::iterator itr = currentSegs.begin(); itr != currentSegs.end(); itr++) {
			// Find the StreetSegment connecting the two GeoCoords, and add it to routedPath
			if (itr->start == cameFrom && itr->end == cur) {
				routedPath.push_front(*itr);
			}
		}
		// If reach the end, break out of the loop
		if (cameFrom == start)
			break;
		cur = cameFrom; // Set the current GeoCoord to the last one
	}
}

double PointToPointRouterImpl::getDistance(const list<StreetSegment>& s) const {
	list<StreetSegment>::const_iterator itr = s.begin();

	// Gets the distance for a list of StreetSegments
	double distance = 0;
	while (itr != s.end()) {
		distance += distanceEarthMiles(itr->start, itr->end);
		itr++;
	}

	return distance;
}

bool PointToPointRouterImpl::checkCoord(const GeoCoord& g1, const GeoCoord& g2) const {
	vector<StreetSegment> segments;
	m_streetMap->getSegmentsThatStartWith(g1, segments);
	for (vector<StreetSegment>::iterator itr = segments.begin(); itr != segments.end(); itr++) {
		if (g2 == itr->end) {
			return true;
		}
	}
	return false;
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
	m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
	delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
	const GeoCoord& start,
	const GeoCoord& end,
	list<StreetSegment>& route,
	double& totalDistanceTravelled) const
{
	return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}