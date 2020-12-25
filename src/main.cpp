#include "ExpandableHashMap.h"
#include "provided.h"
#include <vector>
#include <list>
#include <string>
#include <initializer_list>
#include <functional>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <thread>
#include <future>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <unistd.h>

using namespace std;

bool isNear(double x, double y)
{
	return abs(x - y) < 1e-5;
}

char* prepfile(const string& lines)
{
	static int nfiles = 0;
	static char filename[][40] = {
		"zzdatasmXXXXXX", "zzdatasmXXXXXX", "zzdatasmXXXXXX",
		"zzdatasmXXXXXX", "zzdatasmXXXXXX", "zzdatasmXXXXXX",
		"zzdatasmXXXXXX", "zzdatasmXXXXXX", "zzdatasmXXXXXX",
		"zzdatasmXXXXXX"
	};
	if (nfiles == sizeof(filename) / sizeof(filename[0]))
	{
		cerr << "Too many files" << endl;
		exit(1);
	}
	int fd = mkstemp(filename[nfiles]);
	if (nfiles == 0)
		atexit([] { for (int k = 0; k < nfiles; k++) remove(filename[k]); });
	if (fd == -1)
	{
		cerr << "Cannot create " << filename[nfiles] << endl;
		exit(1);
	}
	close(fd);
	ofstream ofs(filename[nfiles]);
	if (!ofs)
	{
		cerr << "Cannot open " << filename[nfiles] << endl;
		exit(1);
	}
	char lastch = '\n';
	for (auto c : lines)
	{
		lastch = (c == '/' ? '\n' : c);
		ofs.put(lastch);
	}
	if (lastch != '\n')
		ofs.put('\n');
	return filename[nfiles++];
}

void load(StreetMap& sm, const string& lines)
{
	sm.load(prepfile(lines));
}

#define gc(x,y) GeoCoord(#x,#y)
#define ss(x1,y1,x2,y2,n) StreetSegment(GeoCoord(#x1,#y1), GeoCoord(#x2,#y2), n)

double actualdist(const GeoCoord& depot, const vector<DeliveryRequest>& req)
{
	if (req.empty())
		return 0;
	double dist = 0;
	dist += distanceEarthMiles(depot, req.front().location);
	for (int k = 0; k < req.size() - 1; k++)
		dist += distanceEarthMiles(req[k].location, req[k + 1].location);
	dist += distanceEarthMiles(req.back().location, depot);
	return dist;
}

bool lessdo(const DeliveryRequest& lhs, const DeliveryRequest& rhs)
{
	if (lhs.location < rhs.location) return true;
	if (rhs.location < lhs.location) return false;
	return lhs.item < rhs.item;
}

bool eqdo(const DeliveryRequest& lhs, const DeliveryRequest& rhs)
{
	return lhs.location == rhs.location && lhs.item == rhs.item;
}

bool doit(DeliveryOptimizer& dop, const GeoCoord& depot,
	vector<DeliveryRequest>& req, double& odist, double& ndist,
	double& oactdist, double& nactdist)
{
	oactdist = actualdist(depot, req);
	auto oldreq = req;
	sort(begin(oldreq), end(oldreq), lessdo);
	dop.optimizeDeliveryOrder(depot, req, odist, ndist);
	nactdist = actualdist(depot, req);
	auto newreq = req;
	sort(begin(newreq), end(newreq), lessdo);
	return oldreq.size() == newreq.size() &&
		equal(begin(oldreq), end(oldreq), begin(newreq), eqdo);
}

bool matches(const DeliveryCommand& cmd, char type, string name, string dir, double& cumdist)
{
	if (type != 'P')
	{
		cerr << "Wrong call to matches P!" << endl;
		return false;
	}
	cumdist += cmd.dist();
	return cmd.type() == type && cmd.name() == name && cmd.dir() == dir;
}

bool matches(const DeliveryCommand& cmd, char type, string name, string dir)
{
	if (type != 'T')
	{
		cerr << "Wrong call to matches T!" << endl;
		return false;
	}
	return cmd.type() == type && cmd.name() == name && cmd.dir() == dir;
}

bool matches(const DeliveryCommand& cmd, char type, string item)
{
	if (type != 'D')
	{
		cerr << "Wrong call to matches D!" << endl;
		return false;
	}
	return cmd.item() == item;
}

void xxxtestone(int n)
{
	StreetMap sm;
	vector<DeliveryRequest> req;
	vector<DeliveryCommand> cmds;
	double dist = 0;

	switch (n)
	{
	default: {
		cout << "Bad argument" << endl;
	} break; case  1: {
		load(sm, "Broadway/2/1.2 2.3 1.4 2.5/1.4 2.5 1.5 2.5/");
		DeliveryPlanner dp(&sm);
		dp.generateDeliveryPlan(gc(1.2, 2.3), req, cmds, dist);
		assert(cmds.empty() && dist == 0);
	} break; case  2: {
		load(sm, "Kasturba Gandhi Marg/2/1.0 2.0 2.0 3.0/2.0 3.0 1.0 3.0/");
		DeliveryPlanner dp(&sm);
		req.push_back({ "A", gc(1.0,3.0) });
		dist = -999;
		dp.generateDeliveryPlan(gc(1.0, 2.0), req, cmds, dist);
		assert(dist > 0);
	} break; case  3: {
		load(sm, "Elm/2/1.0 2.0 2.0 3.0/2.0 3.0 1.0 3.0/");
		DeliveryPlanner dp(&sm);
		req.push_back({ "rice", gc(1.0,3.0) });
		dp.generateDeliveryPlan(gc(1.0, 2.0), req, cmds, dist);
		assert(cmds.size() == 3);
		double cumdist = 0;
		assert(matches(cmds[0], 'P', "Elm", "northeast", cumdist));
		assert(matches(cmds[1], 'D', "rice"));
		assert(matches(cmds[2], 'P', "Elm", "north", cumdist));
		assert(isNear(dist, 333.577));
		assert(isNear(dist, cumdist));
	} break; case  4: {
		load(sm, "Elm/1/1.0 2.0 2.0 3.0/Oak/1/2.0 3.0 1.0 3.0/");
		DeliveryPlanner dp(&sm);
		req.push_back({ "rice", gc(1.0,3.0) });
		dp.generateDeliveryPlan(gc(1.0, 2.0), req, cmds, dist);
		assert(cmds.size() == 7);
		double cumdist = 0;
		assert(matches(cmds[0], 'P', "Elm", "northeast", cumdist));
		assert(matches(cmds[1], 'T', "Oak", "right"));
		assert(matches(cmds[2], 'P', "Oak", "south", cumdist));
		assert(matches(cmds[3], 'D', "rice"));
		assert(matches(cmds[4], 'P', "Oak", "north", cumdist));
		assert(matches(cmds[5], 'T', "Elm", "left"));
		assert(matches(cmds[6], 'P', "Elm", "southwest", cumdist));
		assert(isNear(dist, 333.577));
		assert(isNear(dist, cumdist));
	} break; case  5: {
		load(sm, "Elm/1/1.0 2.0 2.0 3.0/Oak/1/2.0 3.0 3.0 4.0/");
		DeliveryPlanner dp(&sm);
		req.push_back({ "rice", gc(3.0,4.0) });
		dp.generateDeliveryPlan(gc(1.0, 2.0), req, cmds, dist);
		assert(cmds.size() == 5);
		double cumdist = 0;
		assert(matches(cmds[0], 'P', "Elm", "northeast", cumdist));
		assert(matches(cmds[1], 'P', "Oak", "northeast", cumdist));
		assert(matches(cmds[2], 'D', "rice"));
		assert(matches(cmds[3], 'P', "Oak", "southwest", cumdist));
		assert(matches(cmds[4], 'P', "Elm", "southwest", cumdist));
		assert(isNear(dist, 390.722));
		assert(isNear(dist, cumdist));
	}
	}
}

int main()
{
	cout << "Enter test number: ";
	int n;
	cin >> n;
	if (n < 1 || n > 5)
	{
		cout << "Bad test number" << endl;
		return 1;
	}
	testone(n);
	cout << "Passed" << endl;
}
