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

	void agregarUnidad(Unit* u);

	Unit* getUltimoBunkerCreado(); // retorna el ultimo bunker que se agrego al grupo, es decir el mas recientemente creado
	Unit* getPrimerBunkerCreado(); // retorna el primer bunker que se agrego al grupo
	int getCantBunkers();

	void estrategia1(Unit *u);
	
	TilePosition* posicionNuevoBunker();
	TilePosition* posicionNuevaTorreta();

	void onFrame(); // metodo a ejecutar en cada frame

private:
	std::list<Unit*> bunkers;
	std::list<Unit*> misileTurrets;

	bool perteneceBunker(Unit *u);

	Region *reg; // region en la cual estara ubicado el grupo de bunkers
	Chokepoint *choke; // chokepoint que debe defender el grupo de bunkers
	AnalizadorTerreno *analizador; // puntero al analizador del terreno

	static const int frameLatency = 400; // cantidad de frames que espera para ejecutar nuevamente el control de bunkers destruidos
	void controlDestruidos(); // controla si algun bunker del grupo fue destruido y lo elimina de la lista de bunkers
	
};
