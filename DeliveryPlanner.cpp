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
	const StreetMap* m_map;
	bool generateCommand(double& distance, GeoCoord start, GeoCoord end, DeliveryRequest d, vector<DeliveryCommand>& commands)const;
	string getdir(StreetSegment seg)const;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
{
	m_map = sm;
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
	totalDistanceTravelled = 0;
	DeliveryOptimizer optimizer(m_map);
	vector<DeliveryRequest> temp = deliveries;
	double oldcrow;
	double newcrow;
	vector<StreetSegment> test;
	//test for bad coordinates
	if (!m_map->getSegmentsThatStartWith(depot, test))
		return BAD_COORD;
	for (int i = 0; i < temp.size(); i++) {
		if (!m_map->getSegmentsThatStartWith(deliveries[i].location, test))
			return BAD_COORD;
	}
	//use optimizer to generate optimized delivery order sequence
	optimizer.optimizeDeliveryOrder(depot, temp, oldcrow, newcrow);
	double distance1 = 0;
	//generate commands and calculate distance traveled, if cannot generate, then return no route
	if (!generateCommand(distance1, depot, temp[0].location, temp[0], commands))
		return NO_ROUTE;
	totalDistanceTravelled += distance1;
	for (int i = 0; i < temp.size() - 1; i++) {
		double distance = 0;
		if (!generateCommand(distance, temp[i].location, temp[i + 1].location, temp[i + 1], commands))
			return NO_ROUTE;
		totalDistanceTravelled += distance;
	}
	//if all commands can be generates, delivery success
	return DELIVERY_SUCCESS;  // Delete this line and implement this function correctly
}
string DeliveryPlannerImpl::getdir(StreetSegment seg)const {
	double angle = angleOfLine(seg);//get angle of line
	//return direction strings according to angles of line
	if (0 <= angle && angle < 22.5)
		return "east";
	if (22.5 <= angle && angle < 67.5)
		return "northeast";
	if (67.5 <= angle && angle < 112.5)
		return "north";
	if (112.5 <= angle && angle < 157.5)
		return "northwest";
	if (157.5 <= angle && angle < 202.5)
		return"west";
	if (202.5 <= angle && angle < 247.5)
		return "southwest";
	if (247.5 <= angle && angle < 292.5)
		return "south";
	if (292.5 <= angle && angle < 337.5)
		return "southeast";
	if (angle >= 337.5)
		return "east";
	else
		return "thiswillnothappen";
}
bool DeliveryPlannerImpl::generateCommand(double& distance, GeoCoord start, GeoCoord end, DeliveryRequest d, vector<DeliveryCommand>& commands)const {
	double dis = 0;
	list<StreetSegment> segs;
	PointToPointRouter router(m_map);
	//if no route, return false immediately
	if (router.generatePointToPointRoute(start, end, segs, dis) == NO_ROUTE)
		return false;
	//if start and end are not the same point
	if (dis != 0) {
		list<StreetSegment>::iterator iter = segs.begin();
		while (iter != segs.end()) {
			//if segments have the same name, get the direction of the first one and add up total distance traveled
			list<StreetSegment>::iterator iterer = iter;
			iterer++;
			string dir = getdir(*iter);
			double dis = distanceEarthMiles(iter->start, iter->end);
			while ( (iterer != segs.end()) && (iter->name == iterer->name)) {
				dis += distanceEarthMiles(iterer->start, iterer->end);
				iter++;
				iterer++;
			}
			//use the direction and distance from above to generate a proceed command and add to commands
			DeliveryCommand proceed;
			proceed.initAsProceedCommand(dir, iter->name, dis);
			commands.push_back(proceed);
			//if not at the end of route, then we must be on another street
			if (iterer != segs.end()) {
				double angle = angleBetween2Lines(*iter, *iterer);
				string turn;
				//if we encounter a turn, generate corresponding turn command and add to commands
				if (angle >= 1 && angle < 180)
					turn = "left";
				if (angle >= 180 && angle <= 359)
					turn = "right";
				if (!(angle < 1 || angle>359)) {
					DeliveryCommand turncommand;
					turncommand.initAsTurnCommand(turn, iterer->name);
					commands.push_back(turncommand);
				}
			}
			//move on to loop again
			iter ++;
		}
	}
	//after reaching the end location, generate the delivery command and add to commands. Then return true.
	DeliveryCommand dc;
	dc.initAsDeliverCommand(d.item);
	commands.push_back(dc);
	distance = dis;
	return true;
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
