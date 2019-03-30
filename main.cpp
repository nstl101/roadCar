#include "common.h"

using namespace std;
map<pair<int, int>, vector<int>> way_map;
//检测当前路口是否已经找到最短路径
bool crossChecked(int i, vector<int>& checkedCross) {
	for (int j = 0; j < checkedCross.size(); ++j) {
		if (i == checkedCross[j]) {
			return true;
		}
	}
	return false;
}
//找寻最短路径的函数
vector<int> findWay(int st, int ed, int speed, map<int, Cross*>& crossMap, map<int, Road*>& roadMap) {
	//vector<vector<int>> searchMap(crossMap.size() + 1, vector<int>(crossMap.size() + 1, INT_MAX));
	if (way_map.count({ st, ed })) return way_map[{st, ed}];
	map<int, map<int, int>> searchMap;
	for (auto& e : crossMap)
	{
		auto cross_id = e.first;
		for (auto& q : crossMap)
		{
			auto cross_id_to = q.first;
			if (cross_id == cross_id_to)
			{
				searchMap[cross_id][cross_id_to] = 0;
			}
			else
			{
				searchMap[cross_id][cross_id_to] = INT_MAX;
			}
		}
	}
	//创建一个二维地图并初始化
	for (auto& e : crossMap) {
		auto cross_id = e.first;
		auto cross = e.second;
		auto& RoadId = cross->RoadId;
		searchMap[cross_id][cross_id] = 0;
		for (int i = 0; i < RoadId.size(); ++i) {
			if (RoadId[i] == -1) continue;
			auto road = roadMap[RoadId[i]];
			if (road->st == cross_id) {
				int trueSpeed = min(road->maxSpeed, speed);
				searchMap[cross_id][road->ed] = (road->len * road->len) / (trueSpeed * road->cnt);
			}
			else if (road->f) {
				int trueSpeed = min(road->maxSpeed, speed);
				searchMap[cross_id][road->st] = (road->len * road->len) / (trueSpeed * road->cnt);
			}
		}
	}
	vector<int> ans, checkedCross;//checkedCross用于存放已经找到最短路径的路口
	//vector<int> jumpList(crossMap.size() + 1, st);
	map<int, int> jumpList;
	for (auto& e : crossMap)
	{
		jumpList[e.first] = st;
	}
	checkedCross.push_back(st);
	while (checkedCross.back() != ed) {
		for (auto& i : crossMap) {
			if (crossChecked(i.first, checkedCross)) {
				continue;
			}
			for (int j = 0; j < checkedCross.size(); ++j) {
				if (searchMap[st][checkedCross[j]] < searchMap[st][i.first] - searchMap[checkedCross[j]][i.first]) {
					searchMap[st][i.first] = searchMap[st][checkedCross[j]] + searchMap[checkedCross[j]][i.first];
					jumpList[i.first] = checkedCross[j];
				}
			}
		}
		int shortestId;
		int curMinDistance = INT_MAX;
		for (auto& i : crossMap) {
			if (crossChecked(i.first, checkedCross)) {
				continue;
			}
			if (searchMap[st][i.first] < curMinDistance) {
				shortestId = i.first;
				curMinDistance = searchMap[st][i.first];
			}
		}
		checkedCross.push_back(shortestId);
	}
	int curCrossId = ed;
	while (curCrossId != st) {
		ans.insert(ans.begin(), curCrossId);
		curCrossId = jumpList[curCrossId];
	}
	ans.insert(ans.begin(), curCrossId);
	vector<int> ans_road;
	for (int i = 0; i < ans.size(); ++i)
	{
		int cross_id = ans[i];
		auto cross = crossMap[cross_id];
		if (i + 1 < ans.size())
		{
			int cross_next = ans[i + 1];
			int tar = -1;
			for (auto& roadid : cross->RoadId)
			{
				if (roadid != -1)
				{
					if (roadMap[roadid]->ed == cross_next || (roadMap[roadid]->st == cross_next && roadMap[roadid]->f == 1))
					{
						tar = roadid;
						break;
					}
				}
			}
			if (!way_map.count({ st, cross_id }))
			{
				way_map[{st, cross_id}] = ans_road;
			}
			ans_road.push_back(tar);
		}
	}
	return ans_road;
}
bool driveAllCarJustOnRoadToEndState()
{
	bool hasWaitCar = false;
	for (auto& e : road_map)
	{
		auto& road = *e.second;
		int id = road.id, maxSpeed = road.maxSpeed;
		int len = road.len;
		int f = road.f;
		//车道
		for (auto& line : road.lines)
		{
			auto& cars = line.car_id;
			bool wait = false;
			int maxlen = INT_MAX;
			for (auto car_id : cars)
			{
				// int car_id = cars[i];
				auto car = car_map[car_id];
				int car_speed = car->maxSpeed;
				int speed = min(maxSpeed, car_speed);
				int st = line.st, ed = line.ed;

				auto ans = ans_map[car_id];
				if (car->id == 10000)
				{
					////////cout << ans->pi<<endl;
				}
				if (car->pos + speed > len && car->pos + speed < maxlen)
				{
					//进入路口
					if (ans->pi + 1 < ans->path.size()) //还有道路没走完
					{
						int road_next = ans->path[ans->pi + 1];
						int left_dist = min(road_map[road_next]->maxSpeed, car->maxSpeed - (len - car->pos));

						//cross_map[target_corssid]->waitqueue.push_back({car_id, left_dist}); 
						car->left_dist = left_dist;
						line.waitqueue.push_back(car_id);
						//肖：我改成了用line存放，路口调度统一用这个
						maxlen = car->pos;
						wait = true;
						car->state = WAIT;
						hasWaitCar = true;
					}
					else //路走完了
					{
						line.waitqueue.push_back(car_id);
						//肖：我改成了用line存放，路口调度统一用这个
						maxlen = car->pos;
						wait = true;
						car->state = WAIT;
						hasWaitCar = true;
					}
				}
				else if (car->pos + speed >= maxlen)
				{
					//等待前车
					if (wait) //如果前车是等待状态
					{
						if (car->pos + speed > len)
						{
							if (ans->pi + 1 < ans->path.size())
							{
								int road_next = ans->path[ans->pi + 1];
								int left_dist = min(road_map[road_next]->maxSpeed, car->maxSpeed - (len - car->pos));
								car->left_dist = left_dist;
							}
						}
						line.waitqueue.push_back(car_id);
						car->state = WAIT;
						hasWaitCar = true;
					}
					else // 终止状态
					{
						////////cout << "car_id" << car->id << " move from " << road.id << " : " << car->pos;
						car->pos = maxlen - 1;
						ans->pos = car->pos;
						car->state = END;
						////////cout <<" to" << car->pos << endl;
					}
					maxlen = car->pos;
				}
				else
				{
					car->pos += speed;
					ans->pos = car->pos;
					maxlen = car->pos;
					car->state = END;
					/*肖添加*/
					wait = false;
					////////cout << "car_id" << car->id << " move from " << road.id << " : " << car->pos - speed << " to" << car->pos<<endl;
				}
			}
		}
	}
	return hasWaitCar;
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
int getOrder(Cross * cross, Car * car)
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
//从原路口st到新的路口 roadid 能否放下去
int canPlace(int roadid, int st, int left_dist) //肖：增加left_dist用于判断是否需要等待前车，更改道路已满的返回值为-2
{
	auto road = road_map[roadid];
	for (int i = 0; i < road->lines.size(); ++i)
	{
		auto& roadline = road->lines[i];
		if (roadline.st == st)
		{
			if (roadline.car_id.empty())
			{
				return i;
			}
			else if (car_map[roadline.car_id.back()]->state == END) //寻找第一个终止状态的车道
			{
				auto car_id = roadline.car_id.back();
				auto ans = ans_map[car_id];
				if (ans->pos > 1)
				{
					return i;
				}
			}
			else //肖：如果第一个车道的状态不是终止且小于等于前行距离 继续等待
			{
				auto car_id = roadline.car_id.back();
				auto ans = ans_map[car_id];
				if (ans->pos > left_dist) {
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
		auto& roadline = road->lines[i];
		if (roadline.ed != ed)
		{
			continue;
		}
		if (roadline.waitqueue.size() == 0) {
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
void updateRoadLine(RoadLine & roadline, int maxPos)
{
	auto road = road_map[roadline.fathRoad];
	auto& waitqueue = roadline.waitqueue;
	while (waitqueue.size() != 0)
	{
		auto carId = waitqueue.front();
		auto car = car_map[carId];
		auto ans = ans_map[car->id];

		int speed = min(road->maxSpeed, car->maxSpeed);
		if (car->id == 10000)
		{
			////////cout << ans->pi << endl;
		}
		if (car->pos + speed > road->len && car->pos + speed < maxPos)
		{
			return;
		}
		else if (car->pos + speed < maxPos)
		{
			car->pos = car->pos + speed;
			ans->pos = car->pos;
			car->state = END;
			maxPos = car->pos;
			waitqueue.pop_front();
			////////cout << "car_id" << car->id << " move from" << road->id << " : " << car->pos - speed << " to" << car->pos << endl;
		}
		else
		{
			////////cout << "car_id" << car->id << " move from" << road->id << " : " << car->pos << " to";
			car->pos = maxPos - 1;
			////////cout << car->pos << endl;
			ans->pos = car->pos;
			car->state = END;
			maxPos = car->pos;
			waitqueue.pop_front();
		}
	}
}
//用于调度从车库中出来的车辆
void goCross(Car * car, int roadToId, int roadlineid)
{
	int car_id = car->id;
	auto ans = ans_map[car_id];
	auto road_next = road_map[roadToId];
	ans->pi++;
	auto& roadline_next = road_next->lines[roadlineid];
	ans->pos = min(road_next->maxSpeed, car->maxSpeed);
	car->pos = ans->pos;
	//肖：增加对car->pos的刷新与ans->pos保持同步
	if (!roadline_next.car_id.empty())
	{
		ans->pos = min(ans->pos, ans_map[roadline_next.car_id.back()]->pos - 1);
		car->pos = ans->pos;
	}
	roadline_next.car_id.push_back(car_id);
	////////cout << "car_id" << car_id << " route " << car_map[car_id]->st;
	////////cout << " cur line " << roadlineid << " cur pos " << car->pos << endl;
	// hasCarDrive = true;
	// roadline.waitqueue.pop_front();
}

void goCross(Car * car, int roadFromId, int roadToId, int roadlineid)
{
	int car_id = car->id;
	auto ans = ans_map[car_id];
	auto& road_from = road_map[roadFromId];
	auto& road_next = road_map[roadToId];
	ans->pi++;
	auto& roadline_next = road_next->lines[roadlineid];
	ans->pos = min(road_next->maxSpeed, car->maxSpeed - (road_from->len - ans->pos));
	car->pos = ans->pos;
	//肖：增加对car->pos的刷新与ans->pos保持同步
	if (!roadline_next.car_id.empty())
	{
		ans->pos = min(ans->pos, ans_map[roadline_next.car_id.back()]->pos - 1);
		car->pos = ans->pos;
	}
	roadline_next.car_id.push_back(car_id);
	////////cout << "car_id"<< car_id<<" route"<< car_map[car_id]->st;

	////////cout << " cur line " << roadlineid << " cur pos " << car->pos << endl;
	// hasCarDrive = true;
	// roadline.waitqueue.pop_front();
}
/*肖：添加hasConflict用于判断是否存在与其他道路上车辆冲突*/
bool hasConflict(int curDirect, int oi, Cross * cross, int cross_id) {
	for (int i = 0; i < 4; ++i)
	{
		if (i == oi || cross->RoadId[i] == -1)
			continue;
		int roadid = cross->RoadId[i];
		auto car_id = getFirstWaitCar(roadid, cross_id);
		if (car_id == -1) {
			continue;
		}
		auto car = car_map[car_id];
		auto ans = ans_map[car_id];
		if (ans->pi == ans->path.size() - 1)
			continue;
		int d = getOrder(cross, car);
		if (d > curDirect)
		{
			return true;
		}
	}
	return false;
}
bool driveAllWaitCar()
{
	bool hasCarDrive = false;
	for (auto& e : cross_map) // for small to big by corss_ids
	{
		auto cross_id = e.first;
		auto cross = e.second;
		auto& RoadId = cross->RoadId;

		for (auto oi : cross->Order)
		{
			if (cross->RoadId[oi] == -1)
				continue;
			int roadid = cross->RoadId[oi];
			auto road_from = road_map[roadid];
			bool roadWait = false;
			while (!roadWait)
			{
				if (getFirstWaitCar(roadid, cross_id) == -1)
				{
					break;
				}
				for (auto& roadline : road_from->lines)
				{
					if (roadline.ed == cross_id && !roadline.waitqueue.empty())
					{
						auto car_id = getFirstWaitCar(roadid, cross_id);
						if (car_id != roadline.waitqueue.front()) {
							continue;
						}

						//肖：增加判断car_id是否在当前road line上
						auto ans = ans_map[car_id];
						if (ans->pi == ans->path.size() - 1)
						{
							ans->pi = -1;
							ans->pos = 0;
							car_map[car_id]->pos = 0;
							car_map[car_id]->state = END;
							car_map[car_id]->left_dist = 0;
							//这个车到达终点结束,将pi,pos置0.
							roadline.waitqueue.pop_front();
							roadline.car_id.pop_front();
							//肖：同时更新roadline上的car_id信息
							hasCarDrive = true;
							updateRoadLine(roadline, INT_MAX);
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
						if (hasConflict(d, oi, cross, cross_id)) {
							roadWait = true;
							break;
						}
						int ret = canPlace(road_next_id, cross_id, car_map[car_id]->left_dist);
						if (ret == -2) {
							car_map[car_id]->state = END;
							car_map[car_id]->pos = roadline.len;
							ans->pos = roadline.len;
							roadline.waitqueue.pop_front();
							updateRoadLine(roadline, ans->pos);
							hasCarDrive = true;
							break;
						}
						else if (ret != -1) {
							auto roadline_next = road_map[ans->path[ans->pi + 1]]->lines[ret];
							auto car = car_map[car_id];
							car_map[car_id]->state = END;
							goCross(car, roadid, road_next_id, ret);
							//调用goCross更新道路车辆信息
							roadline.waitqueue.pop_front();
							roadline.car_id.pop_front();
							updateRoadLine(roadline, INT_MAX);
							hasCarDrive = true;
						}
						else {
							roadWait = true;
							break;
						}
					}
				}
			}
		}
	}
	return hasCarDrive;
}

bool driveCarInGarage(int now)
{
	int queueLen = CarPQ.size();
	for (int i = 0; i < queueLen; ++i)
	{
		if (CarPQ.front()->time > now)
		{
			CarPQ.push_back(CarPQ.front());
			CarPQ.pop_front();
		}
		else
		{
			auto car = CarPQ.front();
			int car_id = car->id;
			auto ans = ans_map[car_id];
			CarPQ.pop_front();
			int road_id = ans->path[0];
			int road_line_id;
			if ((road_line_id = canPlace(road_id, car->st, min(road_map[road_id]->maxSpeed, car->maxSpeed))) != -2)
			{
				goCross(car, road_id, road_line_id);
			}
			else
			{
				//////cout << "Garage Full !!!" << endl;
				return false;
				/*++car-> time;
				++ans_map[car->id]->st;
				CarPQ.insert(CarPQ.begin() + queueLen - i - 1, car); //push_back(CarPQ.front());*/
			}
		}
	}
	return true;
}

void reset()
{
	for (auto& e : road_map)
	{
		auto& road = e.second;
		for (auto& roadline : road->lines)
		{
			roadline.waitqueue.clear();
			roadline.car_id.clear();
		}
	}

	for (auto& e : car_map)
	{
		auto car = e.second;
		car->pos = 0;
		car->state = END;
		car->left_dist = 0;
	}

	for (auto& e : ans_map)
	{
		auto cross = e.second;
		cross->pos = 0;
		cross->pi = -1;
	}
}
bool hasWaitCarRemain() {
	for (auto& e : cross_map) {
		auto cross_id = e.first;
		auto cross = e.second;
		auto& RoadId = cross->RoadId;
		for (auto oi : cross->Order) {
			if (cross->RoadId[oi] == -1)
				continue;
			int roadid = cross->RoadId[oi];
			auto road_from = road_map[roadid];
			if (getFirstWaitCar(roadid, cross_id) != -1) {
				return true;
			}
		}
	}
	return false;
}
bool hasCarRemain() {
	for (auto& e : cross_map) {
		auto cross_id = e.first;
		auto cross = e.second;
		auto& RoadId = cross->RoadId;
		for (auto oi : cross->Order) {
			if (cross->RoadId[oi] == -1)
				continue;
			int roadid = cross->RoadId[oi];
			auto road_from = road_map[roadid];
			for (auto& line : road_from->lines)
			{
				if (line.car_id.size() != 0)
				{
					return true;
				}
			}
		}
	}
	return false;
}
int judge(int strat_time)
{
	int now = strat_time;
	while (true)
	{
		now++;
		////////cout << "time" << now << endl;
		bool hasWaitCar = driveAllCarJustOnRoadToEndState();
		if (hasWaitCar) {
			bool canMove = true;
			while (canMove) {
				canMove = driveAllWaitCar();
				if (!canMove && hasWaitCarRemain()) {
					//////cout <<"dead lock at"<<now<<" "<< ans_map[testCar->id]->pi << " " << ans_map[testCar->id]->pos << endl;
					return -1;
				}
				else if (!hasWaitCarRemain()) {
					break;
				}
			}
		}
		if (CarPQ.empty() && !hasCarRemain())
		{
			break;
		}
		if (!driveCarInGarage(now))
		{
			return -2;
		}
	}
	return now - strat_time;
}

int main(int argc, char* argv[])
{
	//cout << "Begin" << std::endl;



#ifdef _WIN32
	string car_p = "D:/projects/crack/config/car.txt";
	string road_p = "D:/projects/crack/config/road.txt";
	string cross_p = "D:/projects/crack/config/cross.txt";
	string ans_p = "D:/projects/crack/config/ans.txt";
#else
	if (argc < 5)
	{
		//cout << "please input args: carPath, roadPath, crossPath, answerPath" << std::endl;
		//exit(1);
	}
	char* car_p = argv[1];
	char* road_p = argv[2];
	char* cross_p = argv[3];
	char* ans_p = argv[4];
#endif
	fstream carPath(car_p);
	fstream roadPath(road_p);
	fstream crossPath(cross_p);
	fstream answerPath(ans_p);


	string line;
	while (getline(carPath, line))
	{
		stringstream ss;
		ss << line;
		char c;
		ss >> c;
		if (c == '#') continue;
		Car * car = new Car();
		ss >> car->id >> c >> car->st >> c >> car->dst >> c >> car->maxSpeed >> c >> car->time;
		cars.push_back(*car);
		car_map[car->id] = car;
	}

	while (getline(roadPath, line))
	{
		stringstream ss;
		ss << line;
		char c;
		ss >> c;
		if (c == '#') continue;
		Road * road = new Road();
		ss >> road->id >> c >> road->len >> c >> road->maxSpeed >> c >> road->cnt >> c >> road->st >> c >> road->ed >> c >> road->f;
		road->lines = vector<RoadLine>(road->cnt);
		for (auto& e : road->lines)
		{
			e.len = road->len;
			e.st = road->st;
			e.ed = road->ed;
			e.fathRoad = road->id;
		}
		if (road->f)
		{
			road->lines.resize(road->cnt * 2);
			for (int i = road->cnt; i < road->cnt * 2; ++i)
			{
				road->lines[i].len = road->len;
				road->lines[i].st = road->ed;
				road->lines[i].ed = road->st;
				road->lines[i].fathRoad = road->id;
			}
			road->cnt *= 2;
		}
		roads.push_back(*road);
		road_map[road->id] = road;
	}

	while (getline(crossPath, line))
	{
		stringstream ss;
		ss << line;
		char c;
		ss >> c;
		if (c == '#') continue;
		Cross * cros = new Cross();
		ss >> cros->id;
		for (int i = 0; i < 4; ++i)
		{
			ss >> c;
			ss >> cros->RoadId[i];
			cros->Order[i] = i;
		}

		cross.push_back(*cros);
		cross_map[cros->id] = cros;
		auto cmp = [&](int i, int j) {
			return cros->RoadId[i] < cros->RoadId[j];
		};
		sort(cros->Order.begin(), cros->Order.end(), cmp);
	}


	for (auto& e : car_map)
	{
		AnsPath* ans = new AnsPath();
		int car_id = e.first;
		auto& car = car_map[car_id];
		ans->car_id = car_id;
		ans->st = car->time;
		ans->pi = -1;
		ans->pos = 0;
		if (way_map.count({ car->st, car->dst }))
		{
			ans->path = way_map[{car->st, car->dst}];
			cout << "find reuse :" << car->st << " to " << car->dst << endl;
		}
		else
		{
			ans->path = findWay(car->st, car->dst, INT_MAX, cross_map, road_map);
			//way_map[{car->st, car->dst}] = ans->path;
		}
		ans_map[car_id] = ans;
	}
	/*
	while (getline(answerPath, line))
	{
		stringstream ss;
		ss << line;
		char c;
		AnsPath* ans = new AnsPath();
		ss >> c;
		ss >> ans->car_id;
		ss >> c;
		ss >> ans->st;
		car_map[ans->car_id]->time = ans->st;//同步更新车辆启动时间
		ss >> c;
		while (c != ')')
		{
			int id;
			ss >> id;
			ss >> c;
			ans->path.push_back(id);
		}
		ans->pi = -1;
		ansPath.push_back(*ans);
		ans_map[ans->car_id] = ans;
	}
	*/
	vector<int> ans_time(ans_map.size());
	auto it = ans_map.begin();
	for (int i = 0; i < ans_map.size(); ++i, ++it)
	{
		ans_time[i] = it->second->car_id;
	}

	auto cmp = [&](int lhs, int rhs) {
		auto l = ans_map[lhs], r = ans_map[rhs];
		int car_id_l = l->car_id, car_id_r = r->car_id;
		return !(*car_map[car_id_l] < *car_map[car_id_r]);
	};
	sort(ans_time.begin(), ans_time.end(), cmp);

	int STEP = 0;
	for (auto& e : road_map)
	{
		auto road_id = e.first;
		auto road = e.second;
		STEP += road->cnt * road->len / road->maxSpeed;
	}
	//cout << STEP << endl;
	int total_cost = 0;
	map<int, bool> used_car_map;
	for (auto& e : car_map) {
		used_car_map[e.first] = false;
	}
	int count = 0;
	int total_num = car_map.size();
	while (total_num > 0)
	{
		int cost_time;
		int last_postive_time;
		int duration_limit = 0;
		int const GROUP_NUM = 50;
		int group_num = GROUP_NUM;
		int cur_pos = car_map.size();

		int cur = 0;

		for (auto& e : used_car_map)
		{
			if (e.second) continue;
			if (++cur == GROUP_NUM)
			{
				cur = 0;
				cost_time = judge(total_cost);
				while (cost_time == -2 || cost_time == -1)
				{
					reset();
					used_car_map[CarPQ.back()->id] = false;
					CarPQ.pop_back();
					--count;
					--duration_limit;
					++total_num;
					cost_time = judge(total_cost);
				}
				////cout << "drive car : " << e.first << endl;
				reset();
				if (duration_limit >= STEP)
				{
					cur = 0;
					break;
				}
			}
			else
			{
				CarPQ.push_back(car_map[e.first]);
				used_car_map[e.first] = true;
				--group_num;
				--total_num;
				++count;
				++duration_limit;
			}

			//--cur_pos;
			//if (!e.second && group_num > 0) 
			//{
			//	CarPQ.push_back(car_map[e.first]);
			//	used_car_map[e.first] = true;
			//	--group_num;
			//	--total_num;
			//	++count;
			//	++duration_limit;
			//}
			//if (cur_pos == 0 || group_num == 0 || total_num == 0)
			//{
			//	group_num = GROUP_NUM;
			//	////cout << "cur pq size : " << CarPQ.size() << endl;
			//	//CarPQ.push_back(car_map[e.first]);
			//	//used_car_map[e.first] = true;
			//	
			//	cost_time = judge(total_cost);
			//	while (cost_time == -2 || cost_time == -1)
			//	{
			//		reset();
			//		used_car_map[CarPQ.back()->id] = false;
			//		CarPQ.pop_back();
			//		--count;
			//		--duration_limit;
			//		++total_num;
			//		cost_time = judge(total_cost);
			//	}
			//	////cout << "drive car : " << e.first << endl;
			//	reset();
			//	if (duration_limit >= STEP)
			//	{
			//		break;
			//	}
		//}
		}
		if (cur != 0)
		{
			cost_time = judge(total_cost);
			while (cost_time == -2 || cost_time == -1)
			{
				reset();
				used_car_map[CarPQ.back()->id] = false;
				CarPQ.pop_back();
				--count;
				--duration_limit;
				++total_num;
				cost_time = judge(total_cost);
			}
			////cout << "drive car : " << e.first << endl;
			reset();
			if (duration_limit >= STEP)
			{
				break;
			}
		}
		for (auto& e : used_car_map)
		{
			if (!e.second)
			{
				ans_map[e.first]->st += cost_time;
				car_map[e.first]->time += cost_time;
			}
		}
		CarPQ.clear();
		total_cost += cost_time;
		cout << " now car : " << count << " duration time : " << cost_time << "total time" << total_cost << endl;
	}
	/*
	for (int i = 0; i < ans_time.size(); i += STEP) {
		int cost_time;
		for (int j = 0; j < STEP && j + i < ans_time.size(); ++j)
		{
			CarPQ.push_back(car_map[ans_time[j + i]]);
			//if (i < ans_time.size() - 1)continue;//测试用
			auto old = CarPQ;
			//////cout <<"CarPQ size :"<< CarPQ.size() << endl;
			cost_time = judge(total_cost);
			while (cost_time == -1) {
				////////cout << "find error!!!" << endl;
				reset();
				CarPQ = old;
				CarPQ.pop_back();
				++ans_map[ans_time[j + i]]->st;
				++car_map[ans_time[j + i]]->time;
				CarPQ.push_back(car_map[ans_time[j + i]]);
				//////cout << CarPQ.back()->time<<" : ";
				testCar = CarPQ.back();
				cost_time = judge(total_cost);
			}
			//////cout <<"finish time"<< cost_time << endl;
			//reset();
			CarPQ = old;
		}
		reset();
		CarPQ.clear();
		//////cout << "i = " << i << endl;
		total_cost += cost_time;
		for (int j = i + STEP; j < ans_time.size(); ++j)
		{
			ans_map[ans_time[j]]->st += cost_time;
			car_map[ans_time[j]]->time += cost_time;
		}
		//cout << "duration " << i << endl;
	}
	*/
	//cout <<"total time"<< total_cost << endl;
	fstream fout(ans_p, ios::out);

	for (auto& e : ans_map)
	{
		auto car_id = e.first;
		auto ans = e.second;
		stringstream ss;
		ss << "(";
		ss << car_id << "," << ans->st;
		for (auto road_id : ans->path)
		{
			ss << "," << road_id;
		}
		ss << ')';
		fout << ss.str() << endl;
	}
	fout.close();
	return 0;
}