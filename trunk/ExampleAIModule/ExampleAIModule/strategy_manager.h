#pragma once
#include <BWAPI.h>
#include "Utilidades.h"

using namespace BWAPI;

class strategy_manager
{
public:
	strategy_manager(void);
	void checkGoals(void);
	int* getGoals(void);
	int* getResearchs(void);
	~strategy_manager(void);


private:
	int GoalUnidades[34];
	int cantUnidades[34];
	int GoalResearch[Utilidades::maxResearch];
	bool ResearchDone[Utilidades::maxResearch];

};
