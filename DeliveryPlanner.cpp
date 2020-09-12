#include "provided.h"
#include <vector>
using namespace std;

class DeliveryPlannerImpl
{
public:
	DeliveryPlannerImpl(const StreetMap* sm);
	~DeliveryPlannerImpl();
	DeliveryResult generateDeliveryPlan(
		const GeoCoord& depot,
		const vector<DeliveryRequest>& deliveries,
		vector<DeliveryCommand>& commands,
		double& totalDistanceTravelled) const;
private:
	const StreetMap* m_streetMap;

	string getDirection(double angle) const; // Gets the geographic direction in string form
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
{
	m_streetMap = sm;
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
	const GeoCoord& depot,
	const vector<DeliveryRequest>& deliveries,
	vector<DeliveryCommand>& commands,
	double& totalDistanceTravelled) const
{
	// Create a new vector to store optimized DeliveryRequests
	double oldCrowDistance, newCrowDistance;
	DeliveryOptimizer op(m_streetMap);
	vector<DeliveryRequest> newDeliveries;
	for (vector<DeliveryRequest>::const_iterator itr = deliveries.begin(); itr != deliveries.end(); itr++)
		newDeliveries.push_back(*itr);
	op.optimizeDeliveryOrder(depot, newDeliveries, oldCrowDistance, newCrowDistance);

	// Loop through the deliveries, generating routes between each
	PointToPointRouter router(m_streetMap);
	double distance = 0, routeDistance = 0;
	list<StreetSegment> route;
	for (int i = 0; i <= deliveries.size(); i++) {
		if (i == 0) // Depot to first delivery
			router.generatePointToPointRoute(depot, deliveries[0].location, route, routeDistance);
		else if (i == deliveries.size()) // Last delivery to depot
			router.generatePointToPointRoute(deliveries[i - 1].location, depot, route, routeDistance);
		else // (ith - 1) delivery to ith delivery
			router.generatePointToPointRoute(deliveries[i - 1].location, deliveries[i].location, route, routeDistance);
		distance += routeDistance; // Add to distance

		// Loop through the route
		list<StreetSegment>::iterator itr = route.begin();
		while (itr != route.end()) {
			double currentDistance = 0;
			string currentStreet = itr->name, currentDirection = getDirection(angleOfLine(*itr));
			StreetSegment previous; // Stores previous StreetSegment (for turns)
			// Loop through current street, adding to the current route distance
			while (itr != route.end() && currentStreet == itr->name) {
				currentDistance += distanceEarthMiles(itr->start, itr->end);
				previous = *itr;
				itr++;
			}
			DeliveryCommand proceed;
			proceed.initAsProceedCommand(currentDirection, currentStreet, currentDistance);
			commands.push_back(proceed);
			
			// If reach the end of the route, break out of the loop
			if (itr == route.end())
				break;

			// For a turn, get the angle between the lines, and then create a turn command
			double turnAngle = angleBetween2Lines(previous, *itr);
			if (turnAngle < 1 || turnAngle > 359)
				continue;
			DeliveryCommand turn;
			string turnDirection;
			if (turnAngle >= 1 && turnAngle < 180)
				turnDirection = "left";
			else
				turnDirection = "right";
			turn.initAsTurnCommand(turnDirection, itr->name);
			commands.push_back(turn);
		}

		// If haven't returned to the depot, create a delivery command
		if (i != deliveries.size()) {
			DeliveryCommand deliver;
			deliver.initAsDeliverCommand(deliveries[i].item);
			commands.push_back(deliver);
		}
	}

	totalDistanceTravelled = distance;
	return DELIVERY_SUCCESS;
}

string DeliveryPlannerImpl::getDirection(double angle) const {
	// Returns geographic direction based on angle
	string direction;
	while (angle < 0)
		angle += 360;
	if (angle >= 0 && angle < 22.5)
		direction = "east";
	else if (angle < 67.5)
		direction = "northeast";
	else if (angle < 112.5)
		direction = "north";
	else if (angle < 157.5)
		direction = "northwest";
	else if (angle < 202.5)
		direction = "west";
	else if (angle < 247.5)
		direction = "southwest";
	else if (angle < 292.5)
		direction = "south";
	else if (angle < 337.5)
		direction = "southeast";
	else
		direction = "east";

	return direction;
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
	m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
	delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
	const GeoCoord& depot,
	const vector<DeliveryRequest>& deliveries,
	vector<DeliveryCommand>& commands,
	double& totalDistanceTravelled) const
{
	return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
