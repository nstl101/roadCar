//检测当前路口是否已经找到最短路径
bool crossChecked(int i, vector<int> &checkedCross){
	for(int j = 0; j < checkedCross.size(); ++j){
		if(i == checkedCross[j]){
			return true;
		}
	}
	return false;
}
//找寻最短路径的函数
vector<int> findWay(int st, int ed, int speed, map<int, Cross *> &crossMap, map<int, Road *> &roadMap){
	vector<vector<int>> searchMap(crossMap.size(), vector<int>(crossMap.size(), INT_MAX));
	//创建一个二维地图并初始化
	for(auto &e : crossMap){
		auto cross_id = e.first;
		auto cross = e.second;
		auto &RoadId = cross->RoadId;
		searchMap[cross_id][cross_id] = 0;
		for(int i = 0; i < RoadId.size(); ++i){
			auto road = roadMap[RoadId[i]];
			if(road.st == cross_id){
				int trueSpeed = min(road.maxSpeed, speed);
				searchMap[cross_id][road.ed] = road.len / trueSpeed;
			}else if(road.f){
				int trueSpeed = min(road.maxSpeed, speed);
				searchMap[cross_id][road.st] = road.len / trueSpeed;
			}
		}
	}
	vector<int> ans, checkedCross;//checkedCross用于存放已经找到最短路径的路口
	vector<int> jumpList(crossMap.size(), st);
	checkedCross.push_back(st);
	while(checkedCross.back() != ed){
		for(int i = 0; i < crossMap.size(); ++i){
			if(crossChecked(i, checkedCross)){
				continue;
			}
			for(int j = 0; j < checkedCross.size(); ++j){
				if(searchMap[st][checkedCross[j]] + searchMap[checkedCross[j]][i] < searchMap[st][i]){
					searchMap[st][i] = searchMap[st][checkedCross[j]] + searchMap[checkedCross[j]][i];
					jumpList[i] = checkedCross[j];
				}
			}
		}
		int shortestId;
		int curMinDistance = INT_MAX;
		for(int i = 0; i < crossMap.size(); ++i){
			if(crossChecked(i, checkedCross)){
				continue;
			}
			if(searchMap[st][i] < curMinDistance){
				shortestId = i;
				curMinDistance = searchMap[st][i];
			}
		}
		crossChecked.push_back(shortestId);
	}
	int curCrossId = ed;
	while(curCrossId != st){
		ans.insert(ans.begin(), curCrossId);
		curCrossId = jumpList[curCrossId];
	}
	return ans;
}