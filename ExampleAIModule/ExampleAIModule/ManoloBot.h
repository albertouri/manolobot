#pragma once
#include <BWAPI.h>

class ManoloBot
{
public:
	ManoloBot(void);
	void checkGoals(void);
	void edificioConstruido(int Id);
	void unidadConstruida(BWAPI::Unit* unit);
	~ManoloBot(void);
};
