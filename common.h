#ifndef __COMMON__H
#define __COMMON__H
#include <set>
#include <queue>
#include <assert.h>
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>
#include <list>
#include <queue>

using namespace std;

enum
{
	END,
	WAIT,
	FIN
};

class RoadLine
{
public:
	int len, st, ed;
	list<int> waitqueue;
	list<int> car_id;
	int fathRoad;
};

class Road
{
public:
	vector<RoadLine> lines;
	int id, len, maxSpeed, st, ed;
	int cnt;
	bool f;
};

class Car
{
public:
	int id, st, dst, maxSpeed, time;
	int left_dist;
	//增加left_dist用于路口调度判断
	int pos = 0;
	int state = 0;
	bool operator < (const Car& rhs) const
	{
		if (time > rhs.time) return true;
		return id > rhs.id;
	}
};

auto cmp = [](const Car * lhs, const Car * rhs) {
	return *lhs < *rhs;
};

// priority_queue<Car *, vector<Car *>, decltype(cmp)> CarPQ(cmp);
deque<Car*> CarPQ;

class Cross
{
public:
	int id;
	vector<int> RoadId, Order;

	Cross() : RoadId(4), Order(4) {}
};

class AnsPath
{
public:
	int car_id, st;	//start_time
	vector<int> path;
	int pi = 0, pos = 0;
};

int g_time = 0;

map<int, Car*> car_map;
map<int, Road*> road_map;
map<int, Cross*> cross_map;
map<int, AnsPath*> ans_map;
map<pair<int, int>, int> road_cross;

vector<Car> cars;
vector<Road> roads;
vector<Cross> cross;
#endif