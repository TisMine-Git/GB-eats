#include "provided.h"
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
#include"ExpandableHashMap.h"
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
	return std::hash<string>()(g.latitudeText + g.longitudeText);
}
//unsigned int hasher(const string& s)
//{
//	return std::hash<string>()(s);
//}
class StreetMapImpl
{
public:
	StreetMapImpl();
	~StreetMapImpl();
	bool load(string mapFile);
	bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
	ExpandableHashMap<GeoCoord, vector<StreetSegment>> m_pairs;
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
}

bool StreetMapImpl::load(string mapFile)
{
	ifstream in(mapFile);
	//return false immediately if not content or cannot read
	if (!in)
		return false;
	while (in) {
		//first, get the street name
		string name;
		getline(in, name);
		//then, get the number of segments of the street
		int i;
		in >> i;
		in.ignore(10000, '\n');
		if (!in)
			break;
		//read number of segement lines and get the corresponding segments and coordinates and store them in the hash table
		for (int j = 0; j < i; j++) {
			string startlat;
			string startlon;
			string endlat;
			string endlon;
			string all;
			getline(in, all);
			//get the four coordinates and create coordinate objects according to these data
			//two method bellow with the first one being commented, just in case the latitude become non-negative in other mapdata files
			/*startlat = all.substr(0, 10);
			startlon = all.substr(11, 12);
			endlat = all.substr(24, 10);
			endlon = all.substr(35, 12);*/
			//second method:
			int blank = 0;
			for (int pos = 0;pos<all.length(); pos++) {
				if (all[pos] == ' ')
					blank++;
				else {
					switch (blank) {
					case 0: startlat += all[pos]; break;
					case 1: startlon += all[pos]; break;
					case 2: endlat += all[pos]; break;
					case 3: endlon += all[pos]; break;
					}
				}
			}
			GeoCoord start(startlat, startlon);
			GeoCoord end(endlat, endlon);
			StreetSegment s(start, end, name);
			StreetSegment r(end, start, name);
			//decide whether to create a new vector or just simply find the created one and add in
			if (m_pairs.find(start) == nullptr) {
				vector<StreetSegment> v;
				m_pairs.associate(start, v);
				v.push_back(s);
			}
			if (m_pairs.find(start) != nullptr) {
				vector<StreetSegment>* temp = m_pairs.find(start);
				temp->push_back(s);
			}
			if (m_pairs.find(end) == nullptr) {
				vector<StreetSegment> v;
				m_pairs.associate(end, v);
				v.push_back(r);
			}
			if (m_pairs.find(end) != nullptr) {
				vector<StreetSegment>* temp = m_pairs.find(end);
				temp->push_back(r);
			}
		}
	}
	return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
	//if cannot find the coordinate passed in, return false
	if (m_pairs.find(gc) == nullptr)
		return false;
	//else get the vector containing those segments and let segs equal to it
	else {
		const vector<StreetSegment>* temp = m_pairs.find(gc);
		segs = *temp;
		return true;
	}
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
	m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
	delete m_impl;
}

bool StreetMap::load(string mapFile)
{
	return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
	return m_impl->getSegmentsThatStartWith(gc, segs);
}
