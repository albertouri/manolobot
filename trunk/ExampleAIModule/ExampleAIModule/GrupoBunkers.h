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


	// agrega una unidad al grupo de bunkers, este metodo ubica la unidad en la posicion necesaria
	void agregarUnidad(Unit* u);
	
	// retorna la cantidad de bunkers que hay en el grupo de bunkers (no cuenta los destruidos)
	int getCantBunkers();

	// retorna la cantidad de misile turrets que hay en el grupo de bunkers (no cuenta los destruidos)
	int getCantMisileTurrets();

	// retorna la cantidad de marines que hay en el grupo de bunkers (no cuenta los destruidos)
	int getCantMarines();
	
	// retorna la cantidad de tanques que hay en el grupo de bunkers (no cuenta los destruidos)
	int getCantTanks();

	// retorna un tilePosition donde se deberia ubicar el nuevo bunker
	TilePosition* posicionNuevoBunker(); 

	// retorna un tilePosition donde se deberia ubicar la nueva misile turret
	TilePosition* posicionNuevaTorreta(); 

	// retorna un tilePosition donde se deberia ubicar el nuevo tanque
	TilePosition* posicionNuevoTanque();

	// metodo a ejecutar en cada frame
	void onFrame(); 

	// retorna true si hay bunkers que no tienen suficientes marines dentro
	bool faltanMarines(); 

	// retorna true si hay menos de 3 tanques cuidando los bunkers
	bool faltanTanques();

	// metodo invocado cuando una unidad fue destruida
	void onUnitDestroy(Unit *u);

private:
	//-- CONSTANTES

	// cantidad de frames que espera para ejecutar nuevamente el control de bunkers destruidos
	static const int frameLatency = 150; 

	//-- VARIABLES GLOBALES

	// TilePosition donde esta ubicado el bunker central del grupo
	TilePosition *bunkerCentral; 
	
	// angulo del grupo de bunkers (0: vertical - 90: horizontal)
	int anguloGrupo;

	// region en la cual estara ubicado el grupo de bunkers
	Region *reg; 

	// chokepoint que debe defender el grupo de bunkers
	Chokepoint *choke; 

	// puntero al analizador del terreno
	AnalizadorTerreno *analizador; 

	// listas de unidades pertenecientes al grupo de bunkers
	std::list<Unit*> listBunkers;
	std::list<Unit*> listMisileTurrets;
	std::list<Unit*> listMarines;
	std::list<std::pair<Position*, Unit*>> listTanks;


	//-- METODOS
	bool perteneceBunker(Unit *u);

	// controla si algun bunker, misile turret o soldado del grupo fue destruido y lo elimina de la lista de bunkers
	void controlDestruidos();

	// ordena a los soldados que esten fuera de bunkers que ingresen en ellos
	void ponerACubierto(); 

	// mueve los soldados que estan fuera de los bunkers a un punto de encuentro, asi, en caso de que los soldados estaban en un bunker que fue destruido, liberan la zona para reconstruir el bunker
	void moverSoldadosPosEncuentro(); 
	
	// posicion de encuentro de los soldados del grupo de bunkers
	Position *posEncuentro; 

	void resaltarUnidades();
	void ubicarModoSiege();
	
	// verifica si los build tiles necesarios para construir tienen terreno apropiado para construir, y tambien verifica si ese terreno no esta ocupado por un bunker, en ese caso retorna false
	bool puedoConstruir(TilePosition t, UnitType tipo); 
	
	// busca una posicion donde ubicar el grupo de bunkers, este metodo solo se usa una vez en el constructor de la clase GrupoBunkers
	TilePosition* encontrarPosicion(int cuadrante, Position p, int angulo); 
	
	// retorna true si el tile position esta ocupado con una unidad de tipo IDTipo
	bool ocupado(TilePosition t, int IDTipo); 

	// retorna la posicion donde deberia ubicarse un bunker para defender el chokepoint pasado como parametro, retorna NULL si no pudo encontrar una posicion posible
	TilePosition* posicionPrimerBunker(Region* r, Chokepoint* c); 

	// recorre la lista de tanques y si encuentra un tanque que ya no existe lo reemplaza por el nuevo tanque. Retorna true si encontro una posicion vacia, false en caso contrario
	bool rellenarPosicionTanque(Unit *u); 
};
