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
	
	// retorna la cantidad de bunkers que hay en el grupo de bunkers (no cuenta los destruidos)
	int getCantBunkers();

	// retorna la cantidad de misile turrets que hay en el grupo de bunkers (no cuenta los destruidos)
	int getCantMisileTurrets();

	// retorna la cantidad de marines que hay en el grupo de bunkers (no cuenta los destruidos)
	int getCantMarines();
	
	// retorna la cantidad de tanques que hay en el grupo de bunkers (no cuenta los destruidos)
	int getCantTanks();

	void estrategia1(Unit *u);
	
	// retorna un tilePosition donde se deberia ubicar el nuevo bunker
	TilePosition* posicionNuevoBunker(); 

	// retorna un tilePosition donde se deberia ubicar la nueva misile turret bunker
	TilePosition* posicionNuevaTorreta(); 


	TilePosition* posicionNuevoTanque();

	void onFrame(); // metodo a ejecutar en cada frame

	bool faltanMarines(); // retorna true si hay bunkers que no tienen suficientes marines dentro
	bool faltanTanques();

	void onUnitDestroy(Unit *u);

private:
	//-- CONSTANTES
	static const int frameLatency = 150; // cantidad de frames que espera para ejecutar nuevamente el control de bunkers destruidos

	//-- VARIABLES GLOBALES
	TilePosition *bunkerCentral; // TilePosition donde esta ubicado el bunker central del grupo
	int anguloGrupo;

	// listas de unidades pertenecientes al grupo de bunkers
	std::list<Unit*> listBunkers;
	std::list<Unit*> listMisileTurrets;
	std::list<Unit*> listMarines;
	//std::list<Unit*> listTanks;
	std::list<std::pair<Position*, Unit*>> listTanks;

	//std::set<int> posicionesLibresTanques;

	//-- METODOS

	bool perteneceBunker(Unit *u);

	Region *reg; // region en la cual estara ubicado el grupo de bunkers
	Chokepoint *choke; // chokepoint que debe defender el grupo de bunkers
	AnalizadorTerreno *analizador; // puntero al analizador del terreno

	void controlDestruidos(); // controla si algun bunker del grupo fue destruido y lo elimina de la lista de bunkers

	void ponerACubierto(); // ordena a los soldados que esten fuera de bunkers que ingresen en ellos
	void moverSoldadosPosEncuentro(); // mueve los soldados que estan fuera de los bunkers a un punto de encuentro, asi, en caso de que los soldados estaban en un bunker que fue destruido, liberan la zona para reconstruir el bunker
	Position *posEncuentro; // posicion de encuentro de los soldados del grupo de bunkers

	void resaltarUnidades();
	void ubicarModoSiege();
	
	bool puedoConstruir(TilePosition t, UnitType tipo); // verifica si los build tiles necesarios para construir tienen terreno apropiado para construir, y tambien verifica si ese terreno no esta ocupado por un bunker, en ese caso retorna false
	TilePosition* encontrarPosicion(int cuadrante, Position p, int angulo); // busca una posicion donde ubicar el grupo de bunkers, este metodo solo se usa una vez en el constructor de la clase GrupoBunkers
	
	bool ocupado(TilePosition t, int IDTipo); // retorna true si el tile position esta ocupado con una unidad de tipo IDTipo
	TilePosition* posicionPrimerBunker(Region* r, Chokepoint* c); // retorna la posicion donde deberia ubicarse un bunker para defender el chokepoint pasado como parametro, retorna NULL si no pudo encontrar una posicion posible

	bool rellenarPosicionTanque(Unit *u); // recorre la lista de tanques y si encuentra un tanque que ya no existe lo reemplaza por el nuevo tanque. Retorna true si encontro una posicion vacia, false en caso contrario
};
