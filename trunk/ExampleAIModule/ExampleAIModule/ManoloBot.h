#pragma once
#include <BWAPI.h>

using namespace BWAPI;

class ManoloBot
{
public:
	ManoloBot(void);
	void checkGoals(void);
	//void edificioConstruido(int Id);
	void edificioConstruido(Unit *u);
	void unidadConstruida(Unit* unit);
	~ManoloBot(void);
};
