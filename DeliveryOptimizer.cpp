#include "provided.h"
#include <vector>
using namespace std;

class DeliveryOptimizerImpl
{
public:
	DeliveryOptimizerImpl(const StreetMap* sm);
	~DeliveryOptimizerImpl();
	void optimizeDeliveryOrder(
		const GeoCoord& depot,
		vector<DeliveryRequest>& deliveries,
		double& oldCrowDistance,
		double& newCrowDistance) const;
private:
	const StreetMap* m_streetMap;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{
	m_streetMap = sm;
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
	const GeoCoord& depot,
	vector<DeliveryRequest>& deliveries,
	double& oldCrowDistance,
	double& newCrowDistance) const
{
	// Gets the old crow distance by summing the distances between the DeliveryRequests
	double totalDistance = 0;
	for (vector<DeliveryRequest>::iterator itr = deliveries.begin(); itr != deliveries.end() - 1; itr++)
		totalDistance += distanceEarthMiles(itr->location, (itr + 1)->location);
	oldCrowDistance = totalDistance;
	
	double minDistance = 100000000;
	double distance;
	int minIndex;
	for (int i = 0; i < deliveries.size(); i++) {
		distance = distanceEarthMiles(depot, deliveries[i].location);
		if (distance < minDistance) {
			minDistance = distance;
			minIndex = i;
		}
	}

	DeliveryRequest temp = deliveries[0];
	deliveries[0] = deliveries[minIndex];
	deliveries[minIndex] = temp;

	// Sorts the array using selection sort
	minDistance = 100000000;
	for (int i = 0; i < deliveries.size() - 1; i++) {
		for (int j = i + 1; j < deliveries.size(); j++) {
			distance = distanceEarthMiles(deliveries[i].location, deliveries[j].location);
			if (distance < minDistance) {
				minDistance = distance;
				minIndex = j;
			}
		}
		DeliveryRequest temp = deliveries[i];
		deliveries[i] = deliveries[minIndex];
		deliveries[minIndex] = temp;
		minDistance = 10000000;
	}
	
	// Gets the new crow distance by summing the distances between the new DeliveryRequests
	totalDistance = 0;
	for (vector<DeliveryRequest>::iterator itr = deliveries.begin(); itr != deliveries.end() - 1; itr++) {
		totalDistance += distanceEarthMiles(itr->location, (itr + 1)->location);
	}
	newCrowDistance = totalDistance;
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
	m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
	delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
	const GeoCoord& depot,
	vector<DeliveryRequest>& deliveries,
	double& oldCrowDistance,
	double& newCrowDistance) const
{
	return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
