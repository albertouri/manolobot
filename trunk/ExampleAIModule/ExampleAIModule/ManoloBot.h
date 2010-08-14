#pragma once
#include <BWAPI.h>

using namespace BWAPI;

class ManoloBot
{
public:
	ManoloBot(void);
	void checkGoals(void);
	void onUnitCreate(Unit* u);
	void onUnitDestroy(Unit* u);
	void onNukeDetect(Position p);
	void onUnitShow(Unit* u);
	~ManoloBot(void);
};
