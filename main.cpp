//#include "bits/stdc++.h"
#include <set>
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

#include "findWay.cpp"

using namespace std;

enum
{
	END,
	WAIT
};

class RoadLine
{
  public:
	int len, st, ed;
	list<int> waitqueue;
	vector<int> car_id;
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
};

class Cross
{
  public:
	int id;
	vector<int> RoadId, Order;
	vector<pair<int, int>> waitqueue; //car_id, left_dist

	Cross() : RoadId(4), Order(4) {}
};

class AnsPath
{
  public:
	int car_id, st;
	vector<int> path;
	int pi = 0, pos = 0;
};

class Graph
{
  public:
};

int g_time = 0;

map<int, Car *> car_map;
map<int, Road *> road_map;
map<int, Cross *> cross_map;
map<int, AnsPath *> ans_map;
map<pair<int, int>, int> road_cross;

vector<Car> cars;
vector<Road> roads;
vector<Cross> cross;

void driveAllCarJustOnRoadToEndState()
{
	for (auto &road : roads)
	{
		int id = road.id, maxSpeed = road.maxSpeed;
		int len = road.len;
		int f = road.f;
		//车道
		for (auto &line : road.lines)
		{
			auto &cars = line.car_id; 
			bool wait = false;
			for (int i = cars.size() - 1; i >= 0; --i)
			{
				int car_id = cars[i];
				auto car = car_map[car_id];
				int car_speed = car->maxSpeed;
				int speed = min(maxSpeed, car_speed);
				int st = line.st, ed = line.ed;

				auto ans = ans_map[car_id];

				int maxlen = INT_MAX;

				if (car->pos + speed > len)
				{
					//进入路口
					int left_dist = min(road_map[ed]->maxSpeed, car->maxSpeed - (len - car->pos));

					if (ans->pi + 1 < ans->path.size()) //还有道路没走完
					{

						int road_next = ans->path[ans->pi + 1];

						int target_corssid = road_cross[{id, road_next}];

						//cross_map[target_corssid]->waitqueue.push_back({car_id, left_dist}); 
						car->left_dist = left_dist;
						line.waitqueue.push_back(car_id);
						//肖：我改成了用line存放，路口调度统一用这个
						maxlen = car->pos;
						wait = true;
						car->state = WAIT;
					}
					else //路走完了
					{
					}
				}
				else if (car->pos + speed >= maxlen)
				{
					//等待前车
					if (wait) //如果前车是等待状态
					{
						line.waitqueue.push_back(car_id);
						car->state = WAIT;
					}
					else // 终止状态
					{
						car->pos = maxlen - 1;
						car->state = END;
					}
					maxlen = car->pos;
				}
				else
				{
					car->pos += speed;
					maxlen = car->pos;
					car->state = END;
					/*肖添加*/
					wait = false;
				}
			}
		}
	}
}

int getOrder(Cross *cross, Car *car)
{
	int car_id = car->id;
	auto ans = ans_map[car_id];
	if (ans->pi + 1 == ans->path.size())
		return 0;
	auto road_id_old = ans->path[ans->pi];
	auto road_id_new = ans->path[ans->pi + 1];
	int old = -1, mew = -1;
	for (int i = 0; i < 4; ++i)
	{
		if (road_id_old == cross->RoadId[i])
			old = i;
		if (road_id_new == cross->RoadId[i])
			mew = i;
	}
	return getOrder(old, mew);
}

int getOrder(int old_road, int new_road)
{
	if ((new_road - old_road + 4) % 4 == 2)
		return 3; // 直行
	else if (new_road - old_road == 1)
		return 2; // 左转
	else
		return 1; //右转
}

//从原路口st到新的路口 roadid 能否放下去
int canPlace(int roadid, int st, int left_dist) //肖：增加left_dist用于判断是否需要等待前车，更改道路已满的返回值为-2
{
	auto road = road_map[roadid];
	for (int i = 0; i < road->lines.size(); ++i)
	{
		auto &roadline = road->lines[i];
		if (roadline.st == st)
		{
			if (roadline.car_id.empty())
			{
				return i;
			}
			else if (car_map[roadline.car_id[0]]->state == END) //寻找第一个终止状态的车道
			{
				auto car_id = roadline.car_id[0];
				auto ans = ans_map[car_id];
				if (ans->pos > 1)
				{
					return i;
				}
			}
			else //肖：如果第一个车道的状态不是终止且小于等于前行距离 继续等待
			{
				auto car_id = roadline.car_id[0];
				auto ans = ans_map[car_id];
				if(ans->pos > left_dist){
					return i;//肖：道路前车为等待状态，但当前车辆不会到达这么远的距离，可以进入
				}
				return -1;
			}
		}
	}
	return -2;
}
/*肖：添加函数getFirstWaitCar用于取得指定Road上的第一优先级等待车辆*/
int getFirstWaitCar(int roadId, int ed)
{
	auto road = road_map[roadId];
	int retId = -1;
	int maxPos = -1;
	for (int i = 0; i < road->lines.size(); ++i)
	{
		auto &roadline = road->lines[i];
		if (roadline.ed != ed)
		{
			continue;
		}
		auto carId = roadline.waitqueue.front();
		auto car = car_map[carId];
		if (car->pos > maxPos)
		{
			retId = carId;
			maxPos = car->pos;
		}
	}
	return retId;
}
/*肖：添加updateRoadLine用于在路口等待车辆进入终止状态后对其后roadLine上的车辆状态进行刷新*/
void updateRoadLine(RoadLine &roadline)
{
	auto waitqueue = roadline.waitqueue;
	int maxPos = INT_MAX;
	while (waitqueue.size() != 0)
	{
		auto carId = waitqueue.front();
		auto car = car_map[carId];
		int speed = min(road->maxSpeed, car->maxSpeed);
		if (car->pos + speed > road->len && car->pos + speed < maxPos)
		{
			return;
		}
		else if (car->pos + speed < maxPos)
		{
			car->pos = car->pos + speed;
			ans_map[carId]->pos = car->pos;
			maxPos = car->pos;
			waitqueue.pop_front();
		}
		else
		{
			car->pos = maxPos - 1;
			ans_map[carId]->pos = car->pos;
			maxPos = car->pos;
			waitqueue.pop_front();
		}
	}
}

void goCross(Car *car, int roadFromId, int roadToId, int roadlineid)
{
	int car_id = car->id;
	auto ans = ans_map[car_id];
	auto road_from = road_map[roadFromId];
	auto road_next = road_map[roadToId];
	ans->pi++;
	auto roadline_next = road_next->lines[roadlineid];
	auto car = car_map[car_id];
	ans->pos = min(road_next->maxSpeed, car->maxSpeed - (road_from->len - ans->pos));
	car->pos = ans->pos;
	//肖：增加对car->pos的刷新与ans->pos保持同步
	if (!roadline_next.car_id.empty())
	{
		ans->pos = min(ans->pos, ans_map[roadline_next.car_id[0]]->pos - 1);
		car->pos = ans->pos;
	}
	// hasCarDrive = true;
	// roadline.waitqueue.pop_front();
}
/*肖：添加hasConflict用于判断是否存在与其他道路上车辆冲突*/
bool hasConflict(int curDirect, int oi, Cross *cross, int cross_id){
	for (int i = 0; i < 4; ++i)
	{
		if (i == oi || cross->RoadId[i] == -1)
			continue;
		int roadid = cross->RoadId[i];
		auto car_id = getFirstWaitCar(roadid, cross_id);
		auto car = car_map[car_id];
		auto ans = ans_map[car_id];
		if (ans->pi == ans->path.size() - 1)
			continue;
		int d = getOrder(cross, car);
		if (d < curDirect)
		{
			return true;
		}
	}
	return false;
}
bool driveAllWaitCar()
{
	bool hasCarDrive = false;
	for (auto &e : cross_map) // for small to big by corss_ids
	{
		auto cross_id = e.first;
		auto cross = e.second;
		auto &RoadId = cross->RoadId;
		while (true)
		{

			for (auto oi : cross->Order)
			{
				if (oi == -1)
					continue;
				int roadid = cross->RoadId[oi];
				auto road_from = road_map[roadid];
				bool roadWait = false;
				while(!roadWait){
					if(getFirstWaitCar(roadid, cross_id) == -1){
						break;
					}
				for (auto &roadline : road_from->lines)
				{
					if (roadline.ed == cross_id && !roadline.waitqueue.empty())
					{
						auto car_id = getFirstWaitCar(roadid, cross_id);
						if(car_id != roadline.waitqueue.front()){
							continue;
						}
						//肖：增加判断car_id是否在当前road line上
						auto ans = ans_map[car_id];
						if (ans->pi == ans->path.size() - 1)
						{
							ans->pi = 0;
							ans->pos = 0;
							//这个车到达终点结束,将pi,pos置0.
							roadline.waitqueue.pop_front();
							roadline.car_id.erase(car_id);
							//肖：同时更新roadline上的car_id信息
							hasCarDrive = true;
							updateRoadLine(roadline);
							//肖：当车辆到达终点时更新road line后方车辆
							continue;
						}
						int road_next_id = ans->path[ans->pi + 1];
						int order_old = -1, order_new = -1;
						for (int i = 0; i < 4; ++i)
						{
							if (cross->RoadId[i] == road_from->id)
								order_old = i;
							if (cross->RoadId[i] == road_next_id)
								order_new = i;
						}
						int d = getOrder(order_old, order_new);
						if(hasConflict(d, oi, cross, cross_id)){
							roadWait = true;
							break;
						}
						int ret = canPlace(road_next_id, cross_id, car_map[car_id]->left_dist);
						if(ret == -2){
							car_map[car_id]->state = END;
							car_map[car_id]->pos = roadline.len;
							ans->pos = roadline.len;
							roadline.waitqueue.pop_front();
							updateRoadLine(roadline);
							break;
						}else if(ret != -1){
							auto roadline_next = road_next->lines[ret];
							auto car = car_map[car_id];
							car_map[car_id]->state = END;
							goCross(car, roadid, road_next_id, ret);
							//调用goCross更新道路车辆信息
							roadline.waitqueue.pop_front();
							updateRoadLine(roadline);
							hasCarDrive = true;
						}else{
							roadWait = true;
							break;
						}
					}
				}
			    }
			}
		}
	}
	return hasCarDrive;
}

int main(int argc, char *argv[])
{
	std::cout << "Begin" << std::endl;

	if (argc < 5)
	{
		std::cout << "please input args: carPath, roadPath, crossPath, answerPath" << std::endl;
		exit(1);
	}

	fstream carPath(argv[1]);
	fstream roadPath(argv[2]);
	fstream crossPath(argv[3]);
	fstream answerPath(argv[4]);

	string line;
	while (getline(carPath, line))
	{
		stringstream ss;
		ss << line;
		char c;
		ss >> c;
		Car car;
		ss >> car.id >> c >> car.st >> c >> car.dst >> c >> car.maxSpeed >> c >> car.time;
		cars.push_back(car);

		car_map[car.id] = &car;
	}

	while (getline(roadPath, line))
	{
		stringstream ss;
		ss << line;
		char c;
		Road road;
		int cnt;
		ss >> c >> road.id >> c >> road.len >> c >> road.maxSpeed >> c >> road.cnt >> c >> road.st >> c >> road.ed >> c >> road.f;
		road.lines = vector<RoadLine>(cnt);
		for (auto &e : road.lines)
		{
			e.len = road.len;
			e.st = road.st;
			e.ed = road.ed;
		}
		if (road.f)
		{
			road.lines.resize(cnt * 2);
			for (int i = cnt; i < cnt * 2; ++i)
			{
				road.lines[i].len = road.len;
				road.lines[i].st = road.ed;
				road.lines[i].ed = road.st;
			}
			cnt *= 2;
		}
		roads.push_back(road);
		road_map[road.id] = &road;
	}

	while (getline(crossPath, line))
	{
		stringstream ss;
		ss << line;
		char c;
		Cross cros;
		ss >> c >> cros.id;
		for (int i = 0; i < 4; ++i)
		{
			ss >> c;
			ss >> cros.RoadId[i];
			cros.Order[i] = i;
		}

		for (int i = 0; i < 4; ++i)
		{
			for (int j = i + 1; j < 4; ++j)
			{
				if (cros.RoadId[i] != -1 && cros.RoadId[j] != -1)
					road_cross[{cros.RoadId[i], cros.RoadId[j]}] = cros.id;
			}
		}

		cross.push_back(cros);
		cross_map[cros.id] = &cros;
		auto cmp = [&](int i, int j) {
			return cros.RoadId[i] < cros.RoadId[j];
		};
		sort(cros.Order.begin(), cros.Order.end(), cmp);
	}

	vector<AnsPath> ansPath;
	while (getline(answerPath, line))
	{
		stringstream ss;
		ss << line;
		char c;
		ss >> c;
		AnsPath ans;
		ss >> c;
		ss >> ans.car_id;
		ss >> c;
		ss >> ans.st;
		ss >> c;
		while (c != ')')
		{
			int id;
			ss >> id;
			ans.path.push_back(id);
		}
		ansPath.push_back(ans);
		ans_map[ans.car_id] = &ans;
	}
	while (true)
	{
		break;
	}

	//for (;;) {
	//	while (/* all car in road run into end state */) {
	//		foreach(roads) {
	//			/* 调整所有道路上在道路上的车辆，让道路上车辆前进，只要不出路口且可以到达终止状态的车辆
	//		   * 分别标记出来等待的车辆（要出路口的车辆，或者因为要出路口的车辆阻挡而不能前进的车辆）
	//		   * 和终止状态的车辆（在该车道内可以经过这一次调度可以行驶其最大可行驶距离的车辆）*/
	//			driveAllCarJustOnRoadToEndState(allChannle);/* 对所有车道进行调整 */

	//			/* driveAllCarJustOnRoadToEndState该处理内的算法与性能自行考虑 */
	//		}
	//	}

	//	while (/* all car in road run into end state */) {
	//		/* driveAllWaitCar() */
	//		foreach(crosses) {
	//			foreach(roads) {
	//				Direction dir = getDirection();
	//				Car car = getCarFromRoad(road, dir);
	//				if (conflict) {
	//					break;
	//				}

	//				channle = car.getChannel();
	//				car.moveToNextRoad();

	//				/* driveAllCarJustOnRoadToEndState该处理内的算法与性能自行考虑 */
	//				driveAllCarJustOnRoadToEndState(channel);
	//			}
	//		}
	//	}

	//	/* 车库中的车辆上路行驶 */
	//	driveCarInGarage();
	//}

	return 0;
}
