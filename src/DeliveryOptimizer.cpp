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
	const StreetMap* m_map;
	void createRand(vector<DeliveryRequest>& delivers) const;
	double countCrowDis(const GeoCoord& depot, vector <DeliveryRequest>& deliveries)const;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{
	m_map = sm;
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
	oldCrowDistance = countCrowDis(depot, deliveries);//get the old distance
	//Annealing:
	double T = 5000;
	double q = 0.98;
	double T_min = (1e-8);
	while (T > T_min) {
		for (int i = 0; i < deliveries.size() * deliveries.size(); i++) {
			vector<DeliveryRequest> temp = deliveries;
			createRand(deliveries);
			double dis1 = countCrowDis(depot, deliveries);
			double dis2 = countCrowDis(depot, temp);
			if (dis1 > dis2) {
				double r = ((double)rand()) / (RAND_MAX);
				if (exp(-(dis1 - dis2) / T) <= r)
					deliveries = temp;
			}
		}
		T *= q;
	}//end of annealing
	newCrowDistance = countCrowDis(depot, deliveries);//get the new distance
}
void DeliveryOptimizerImpl::createRand(vector<DeliveryRequest>& deliveries) const {
	int i = (rand() % (deliveries.size() - 0)) + 0;
	int j = (rand() % (deliveries.size() - 0)) + 0;//generate random index
	DeliveryRequest temp = deliveries[i];
	deliveries[i] = deliveries[j];
	deliveries[j] = temp;//swap elements
}
double DeliveryOptimizerImpl::countCrowDis(const GeoCoord& depot, vector <DeliveryRequest>& deliveries) const {
	double dis = distanceEarthMiles(depot, deliveries[0].location);//distance from depot to first location
	for (int i = 0; i < deliveries.size() - 1; i++) {
		dis += distanceEarthMiles(deliveries[i].location, deliveries[i + 1].location);
	}//destance between delivery locations
	dis += distanceEarthMiles(depot, deliveries[deliveries.size() - 1].location);//distance between the last location and depot
	return dis;
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
