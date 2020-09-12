#include "provided.h"
#include "ExpandableHashMap.h"
#include <string>
#include <vector>
#include <functional>
#include <fstream>
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
	return std::hash<string>()(g.latitudeText + g.longitudeText);
}

unsigned int hasher(const string& s) {
	return std::hash<string>()(s);
}

class StreetMapImpl
{
public:
	StreetMapImpl();
	~StreetMapImpl();
	bool load(string mapFile);
	bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
	ExpandableHashMap<GeoCoord, vector<StreetSegment>> m_streetMap; // Stores all mapdata
	StreetSegment reverse(const StreetSegment& segment); // Reverse the GeoCoords on a StreetSegment
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
}

bool StreetMapImpl::load(string mapFile)
{
	// Attach the file to extract data
	ifstream inFile;
	inFile.open(mapFile);
	if (!inFile) {
		cerr << "Unable to open file " << mapFile << endl;
		return false;
	}
	// Retrieves the street name
	for (string street; getline(inFile, street);) {
		// Retrieves the number of GeoCoords the street contains
		string num;
		getline(inFile, num);
		// Converts num to an integer
		int numCoords = stoi(num);

		StreetSegment reversedSeg;
		// For each GeoCoord
		for (int i = 0; i < numCoords; i++) {
			int j = 0; // Index for the vector
			vector<StreetSegment> v(2);
			// Adds the previous segment reversed, if the current
			// GeoCoord isn't the first or last in the StreetSegment
			if (i != 0) {
				v[j] = reversedSeg;
				j++;
			}

			// Retrieves the first GeoCoord
			string startLat;
			inFile >> startLat;
			string startLong;
			inFile >> startLong;
			GeoCoord start(startLat, startLong);
			
			// Retrieves the second GeoCoord
			string endLat;
			inFile >> endLat;
			string endLong;
			inFile >> endLong;
			GeoCoord end(endLat, endLong);

			// Creates the StreetSegment
			StreetSegment seg(start, end, street);
			v[j] = seg;

			// Resizes the array if its the first StreetSegment of the street
			if (i == 0)
				v.resize(1);
			reversedSeg = reverse(seg); // Reverses the current StreetSegment and stores it

			// Check if the key-value pair already exists
			vector<StreetSegment>* s = m_streetMap.find(start);
			// If it does, add to the end of the existing vector
			if (s != nullptr)
				for (int i = 0; i < v.size(); i++)
					s->push_back(v[i]);
			// If not, add a new key-value pair
			else
				m_streetMap.associate(start, v);
		}

		// Adds the last StreetSegment of the road, if it's longer than 1
		if (numCoords > 1) {
			vector<StreetSegment> v(1);
			v[0] = (reversedSeg);
			
			vector<StreetSegment>* s = m_streetMap.find(reversedSeg.start);
			if (s != nullptr)
				for (int i = 0; i < v.size(); i++)
					s->push_back(v[i]);
			else
				m_streetMap.associate(reversedSeg.start, v);
		}

		// Discards the newline character
		getline(inFile, street);
	}

	inFile.close();
	return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
	// Finds the key-value pair for gc
	const vector<StreetSegment>* segVector = m_streetMap.find(gc);

	// If it doesn't exist, return false
	// If it does, retreive the value and return true
	if (segVector == nullptr)
		return false;
	segs = *segVector;
	return true;
}

StreetSegment StreetMapImpl::reverse(const StreetSegment& segment)
{
	// Reverse segment
	StreetSegment reversed(segment.end, segment.start, segment.name);
	return reversed;
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
