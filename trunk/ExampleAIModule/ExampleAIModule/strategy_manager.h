#pragma once
#include <BWAPI.h>

using namespace BWAPI;

class strategy_manager
{
public:
	strategy_manager(void);
	void checkGoals(void);
	int* getGoals(void);
	int* getResearchs(void); // agregado por mi
	bool *strategy_manager::getResearchsDone(void); //agregado por mi
	~strategy_manager(void);
};
