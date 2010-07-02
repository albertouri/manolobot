#pragma once

#include <BWAPI.h>
#include <BWTA.h>
#include "Utilidades.h"
#include "AnalizadorTerreno.h"

using namespace BWAPI;
using namespace BWTA;

class GrupoBunkers
{
public:
	//GrupoBunkers(void);
	GrupoBunkers(AnalizadorTerreno *a);
	~GrupoBunkers(void);
	void agregarBunker(Unit* u);
	Unit* getUltimoBunkerCreado();
	int getCantBunkers();

	void estrategia1(Unit *u);
	TilePosition* posicionNuevoBunker();

private:
	std::list<Unit*> bunkers;
	bool perteneceBunker(Unit *u);

	Region *reg; // region en la cual estara ubicado el grupo de bunkers
	Chokepoint *choke; // chokepoint que debe defender el grupo de bunkers
	AnalizadorTerreno *analizador; // puntero al analizador del terreno
};
