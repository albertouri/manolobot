#pragma once

#include <BWAPI.h>
#include <BWTA.h>
#include "Utilidades.h"
#include "AnalizadorTerreno.h"
#include "Graficos.h"

using namespace BWAPI;
using namespace BWTA;

class GrupoBunkers
{
public:
	GrupoBunkers(AnalizadorTerreno *a);
	~GrupoBunkers(void);

	void agregarUnidad(Unit* u);

	Unit* getUltimoBunkerCreado(); // retorna el ultimo bunker que se agrego al grupo, es decir el mas recientemente creado
	Unit* getPrimerBunkerCreado(); // retorna el primer bunker que se agrego al grupo
	
	int getCantBunkers(); // retorna la cantidad de bunkers que hay en el grupo de bunkers (no cuenta los destruidos)
	int getCantMisileTurrets(); // retorna la cantidad de misile turrets que hay en el grupo de bunkers (no cuenta los destruidos)
	int getCantMarines(); // retorna la cantidad de marines que hay en el grupo de bunkers (no cuenta los destruidos)

	void estrategia1(Unit *u);
	
	TilePosition* posicionNuevoBunker(); // retorna un tilePosition donde se deberia ubicar el nuevo bunker
	TilePosition* posicionNuevaTorreta(); // retorna un tilePosition donde se deberia ubicar la nueva misile turret bunker

	void onFrame(); // metodo a ejecutar en cada frame
	bool faltanMarines(); // retorna true si hay bunkers que no tienen suficientes marines dentro

private:
	// listas de unidades pertenecientes al grupo de bunkers
	std::list<Unit*> listBunkers;
	std::list<Unit*> listMisileTurrets;
	std::list<Unit*> listMarines;
	std::list<Unit*> listTanks;

	std::set<int> posicionesLibresBunkers; // a esta lista se agrega el numero de construccion de un bunker si el mismo fue destruido, para construir de nuevo en esa posicion
	std::set<int> posicionesLibresMisileTurrets; // a esta lista se agrega el numero de construccion de un misile turret si el mismo fue destruido, para construir de nuevo en esa posicion

	bool perteneceBunker(Unit *u);

	Region *reg; // region en la cual estara ubicado el grupo de bunkers
	Chokepoint *choke; // chokepoint que debe defender el grupo de bunkers
	AnalizadorTerreno *analizador; // puntero al analizador del terreno

	static const int frameLatency = 250; // cantidad de frames que espera para ejecutar nuevamente el control de bunkers destruidos
	void controlDestruidos(); // controla si algun bunker del grupo fue destruido y lo elimina de la lista de bunkers

	void ponerACubierto(); // ordena a los soldados que esten fuera de bunkers que ingresen en ellos
	void moverSoldadosPosEncuentro(); // mueve los soldados que estan fuera de los bunkers a un punto de encuentro, asi, en caso de que los soldados estaban en un bunker que fue destruido, liberan la zona para reconstruir el bunker
	Position *posEncuentro; // posicion de encuentro de los soldados del grupo de bunkers

	void resaltarUnidades();
	
};
