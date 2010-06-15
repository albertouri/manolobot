#pragma once

#include <BWAPI.h>
#include <BWTA.h>
#include "Utilidades.h"

using namespace BWAPI;
using namespace BWTA;

class GrupoBunkers
{
public:
	GrupoBunkers(void);
	~GrupoBunkers(void);
	void agregarBunker(Unit* u);
	Unit* getUltimoBunkerCreado();
	int getCantBunkers();

	void estrategia1(Unit *u);

private:
	std::list<Unit*> bunkers;
	bool perteneceBunker(Unit *u);
};
