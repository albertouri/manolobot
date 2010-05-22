#pragma once
#include <BWAPI.h>

using namespace BWAPI;

class strategy_manager
{
public:
	strategy_manager(void);
	void checkGoals(void);
	int* getGoals(void);
	~strategy_manager(void);
};
