#if defined(_MSC_VER)  &&  !defined(_DEBUG)
#include <iostream>
#include <windows.h>
#include <conio.h>

struct KeepWindowOpenUntilDismissed
{
	~KeepWindowOpenUntilDismissed()
	{
		DWORD pids[1];
		if (GetConsoleProcessList(pids, 1) == 1)
		{
			std::cout << "Press any key to continue . . . ";
			_getch();
		}
	}
} keepWindowOpenUntilDismissed;
#endif

#include "provided.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#include "ExpandableHashMap.h"

bool loadDeliveryRequests(string deliveriesFile, GeoCoord& depot, vector<DeliveryRequest>& v);
bool parseDelivery(string line, string& lat, string& lon, string& item);

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		cout << "Usage: " << argv[0] << " mapdata.txt deliveries.txt" << endl;
		return 1;
	}

	StreetMap sm;

	if (!sm.load(argv[1]))
	{
		cout << "Unable to load map data file " << argv[1] << endl;
		return 1;
	}

	GeoCoord depot;
	vector<DeliveryRequest> deliveries;
	if (!loadDeliveryRequests(argv[2], depot, deliveries))
	{
		cout << "Unable to load delivery request file " << argv[2] << endl;
		return 1;
	}

	cout << "Generating route...\n\n";

	DeliveryPlanner dp(&sm);
	vector<DeliveryCommand> dcs;
	double totalMiles;
	DeliveryResult result = dp.generateDeliveryPlan(depot, deliveries, dcs, totalMiles);
	if (result == BAD_COORD)
	{
		cout << "One or more depot or delivery coordinates are invalid." << endl;
		return 1;
	}
	if (result == NO_ROUTE)
	{
		cout << "No route can be found to deliver all items." << endl;
		return 1;
	}
	cout << "Starting at the depot...\n";
	for (const auto& dc : dcs)
		cout << dc.description() << endl;
	cout << "You are back at the depot and your deliveries are done!\n";
	cout.setf(ios::fixed);
	cout.precision(2);
	cout << totalMiles << " miles travelled for all deliveries." << endl;


	/*ExpandableHashMap<string, int> test;
	test.associate("Dan", 4);
	test.associate("Dan", 10);
	cout << test.size() << endl;
	test.associate("Henry", 2);
	test.associate("Fred", 10);
	int* n = test.find("Henry");
	if (n == nullptr)
		cout << "Not found" << endl;
	else
		cout << *n << endl;
	test.associate("Lenny", 4);
	test.associate("a", 5);
	test.associate("b", 6);
	test.associate("c", 7);
	test.associate("d", 8);
	test.associate("e", 10);
	int* num = test.find("Dan");
	if (num == nullptr)
		cout << "Not found" << endl;
	else
		cout << *num << endl;
	cout << "size" << test.size() << endl;

	GeoCoord depot("34.0625329", "-118.4470263");
	GeoCoord sproulLanding("34.0712323", "-118.4505969");
	GeoCoord strathmoreWestwood("34.0687443", "-118.4449195");
	GeoCoord gayleyStrathmore("34.0685657", "-118.4489289");

	DeliveryRequest sproulChicken("Chicken tenders", sproulLanding);
	DeliveryRequest strathmoreSalmon("B-Plate salmon", strathmoreWestwood);
	DeliveryRequest gayleyBeer("Pabst Blue Ribbon beer", gayleyStrathmore);

	StreetMap sm;
	sm.load("mapdata.txt");
	DeliveryPlanner dp(&sm);
	vector<DeliveryCommand> commands;
	double distanceTravelled;
	vector<DeliveryRequest> requests;
	requests.push_back(sproulChicken);
	requests.push_back(strathmoreSalmon);
	requests.push_back(gayleyBeer);
	dp.generateDeliveryPlan(depot, requests, commands, distanceTravelled);*/

	/*StreetMap sm;
	sm.load("mapdata.txt");
	GeoCoord depot("34.0625329", "-118.4470263");
	vector<StreetSegment> segments;
	sm.getSegmentsThatStartWith(depot, segments);
	for (vector<StreetSegment>::iterator itr = segments.begin(); itr != segments.end(); itr++) {
		cerr << "Start: " << itr->start.latitudeText << " " << itr->start.longitudeText << endl;
		cerr << "End: " << itr->end.latitudeText << " " << itr->end.longitudeText << endl;
	}*/
}

bool loadDeliveryRequests(string deliveriesFile, GeoCoord& depot, vector<DeliveryRequest>& v)
{
	ifstream inf(deliveriesFile);
	if (!inf)
		return false;
	string lat;
	string lon;
	inf >> lat >> lon;
	inf.ignore(10000, '\n');
	depot = GeoCoord(lat, lon);
	string line;
	while (getline(inf, line))
	{
		string item;
		if (parseDelivery(line, lat, lon, item))
			v.push_back(DeliveryRequest(item, GeoCoord(lat, lon)));
	}
	return true;
}

bool parseDelivery(string line, string& lat, string& lon, string& item)
{
	const size_t colon = line.find(':');
	if (colon == string::npos)
	{
		cout << "Missing colon in deliveries file line: " << line << endl;
		return false;
	}
	istringstream iss(line.substr(0, colon));
	if (!(iss >> lat >> lon))
	{
		cout << "Bad format in deliveries file line: " << line << endl;
		return false;
	}
	item = line.substr(colon + 1);
	if (item.empty())
	{
		cout << "Missing item in deliveries file line: " << line << endl;
		return false;
	}
	return true;
}
