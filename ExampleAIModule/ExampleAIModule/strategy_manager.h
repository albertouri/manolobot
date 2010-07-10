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

	void onUnitCreate(Unit *u);
	void onUnitDestroy(Unit *u);

	void setResearchsDone(bool *rd);
	
private:
	int GoalUnidades[34];
	int cantUnidades[34]; // mantiene la cantidad de unidades de cada tipo que hay actualmente
	int GoalResearch[Utilidades::maxResearch];
	bool ResearchDone[Utilidades::maxResearch];

};
