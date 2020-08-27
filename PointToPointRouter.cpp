#include "provided.h"
#include <list>
#include"ExpandableHashMap.h"
#include<queue>
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
	const StreetMap* m_map;
	ExpandableHashMap<GeoCoord, StreetSegment>* m_pre;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
{
	m_map = sm;
	m_pre = new ExpandableHashMap<GeoCoord, StreetSegment>;
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
	delete m_pre;
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
	const GeoCoord& start,
	const GeoCoord& end,
	list<StreetSegment>& route,
	double& totalDistanceTravelled) const
{
	vector<StreetSegment> s;
	//if start or end location not in our file, return bad coordinate
	if (!m_map->getSegmentsThatStartWith(start, s) || !m_map->getSegmentsThatStartWith(end, s))
		return BAD_COORD;
	list<StreetSegment>::iterator iter = route.begin();
	//clear the passed in route list and distance traveled
	while (route.size()!=0&&iter != route.end())
		iter = route.erase(iter);
	totalDistanceTravelled = 0;
	//if start location equals end location, then do nothing and return delivery success immeadiately
	if (start == end) {
		return DELIVERY_SUCCESS;
	}
	//use the queue method to explore routes (with optimization as explained below)
	DeliveryResult d = NO_ROUTE;
	queue<GeoCoord> q;
	q.push(start);
	while (q.empty() == false) {
		GeoCoord cur = q.front();
		q.pop();
		if (cur == end) {
			d = DELIVERY_SUCCESS;
			break;
		}
		//optimization:create a vector of streetsegments, store valid unexplored segments into it, sort it and push them in order to the queue
		vector<StreetSegment> segs;
		m_map->getSegmentsThatStartWith(cur, segs);
		vector<StreetSegment> ssm;;
		//get valid segments
		for (int i = 0; i < segs.size(); i++) {
			if (!(segs[i].end == start || m_pre->find(segs[i].end))) {
				ssm.push_back(segs[i]);
			}
		}
		//sorting:
		if (ssm.size() != 0) {
			for (int i = 0; i < ssm.size()-1; i++) {
				for (int j = i; j < ssm.size(); j++) {
					if (distanceEarthKM(ssm[j].end, end) < distanceEarthKM(ssm[i].end, end)) {
						StreetSegment temp = ssm[j];
						ssm[j] = ssm[i];
						ssm[i] = temp;
					}
				}
			}
			//for each explored segment, map its end point to its start point for path tracing
			for (int i = 0; i < ssm.size(); i++) {
				q.push(ssm[i].end);
				m_pre->associate(ssm[i].end, ssm[i]);
			}
		}
	}
	//if delivery success, use the hashtable to trace from the end point all the way back to the start point, generate the route and calculate the distance
	if (d == DELIVERY_SUCCESS) {
		StreetSegment tempseg = *(m_pre->find(end));
		route.push_front(tempseg);
		totalDistanceTravelled += distanceEarthKM(end, tempseg.start);
		while (tempseg.start != start) {
			tempseg = *(m_pre->find(tempseg.start));
			route.push_front(tempseg);
			totalDistanceTravelled += distanceEarthKM(tempseg.end, tempseg.start);
		}
	}
	return d;  // Delete this line and implement this function correctly
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
