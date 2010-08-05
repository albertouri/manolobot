#pragma once
#include <BWAPI.h>
#include "AnalizadorTerreno.h"
#include "Utilidades.h"
#include "compania.h"
#include "CompaniaDefensiva.h"
#include "Scout.h"
#include "Graficos.h"
#include <math.h>
#include "GrupoBunkers.h"
#include <stdlib.h>

using namespace BWAPI;
using namespace BWTA;


class unit_Manager
{
public:
	unit_Manager();
	void executeActions(AnalizadorTerreno *analizador);
	void resetBuildingSemaphore();
	
	void setGoals(int goals[34]);
	void setResearchs(int researchs[Utilidades::maxResearch]);
	void asignarUnidadACompania(Unit* unit);
	
	void verificarBunkers(); // verifica si un bunker esta siendo atacado
	void repararUnidad(Unit *u); // repara la unidad pasada como parametro

	void onUnitCreate(Unit *u);
	void onUnitDestroy(Unit *u);
	void onNukeDetect(Position p);

	bool* getResearchsDone();

	~unit_Manager(void);

private:
	//-- VARIABLES GLOBALES

	// arreglo que mantiene la cantidad de unidades de cada tipo que hay actualmente
	int cantUnidades[34];
	// arreglo que mantiene la cantidad de unidades de cada tipo que deberia haber
	int goalCantUnidades[34];
	// arreglo que mantiene las investigaciones que deben realizarce
	int goalResearch[Utilidades::maxResearch];
	// arreglo que mantiene informacion sobre si una investigacion se realizo o no
	bool researchDone[Utilidades::maxResearch];
	// puntero a un SCV que se encargara de reparar los bunkers que sean atacados
	Unit *reparador1, *reparador2;

	// Grupos de bunkers
	GrupoBunkers *grupoB1;

	// mantiene la posicion del centro de comando
	TilePosition *centroComando;
	//puntero al centro de comando;
	Unit *centroDeComando;

	// Compañias de unidades
	compania* Easy;
	// compañia fox (no es por megan), defiende la base
	CompaniaDefensiva* Fox;

	Scout* magallanes;
	Player* enemigo;

	int SCVgatheringMinerals, SCVgatheringGas;

	// Lista de unidades que estan en construccion actualmente
	std::list<Unit*> unidadesEnConstruccion;

	// puntero a la ultima unidad finalizada, se calcula en cada frame con una llamada a controlarFinalizacion()
	Unit *ultimaFinalizada; 

	int frameLatency;
	int buildingSemaphore;




	//-- METODOS

	// construye una unidad en la ubicacion y del tipo pasados como parametro
	void buildUnit(TilePosition *pos, int id);

	// construye un add-on cuyo tipo se corresponde con el pasado como parametro
	void buildUnitAddOn(int id);

	// construye una refineria en la posicion pasada como parametro. Si la posicion es NULL construye en el geiser mas cercano
	void makeRefinery(TilePosition *pos);

	// retorna un puntero a un SCV
	Unit* getWorker();

	// retorna un puntero a una unidad del tipo pasado como parametro
	Unit* getUnit(int IDTipo);

	// manda a la unidad pasada como parametro a recolectar minerales
	void sendGatherCristal(Unit* worker);

	// manda a la unidad pasada como parametro a recolectar gas
	void sendGatherGas(Unit* worker);

	// manda a entrenar un SCV
	void trainWorker();

	// manda a entrenar un marine
	void trainMarine();

	// manda a entrenar un medico
	void trainMedic();

	// manda a entrenar un tanque de asedio
	void trainTankSiege();

	// calcula una TilePosition disponible para construir una unidad del tipo pasado como parametro
	// despues de usar este metodo ejecutar delete sobre el resultado para liberar memoria
	TilePosition* getTilePositionAviable(UnitType* U);

	// despues de usar este metodo ejecutar delete sobre el resultado para liberar memoria
	TilePosition* getTilePositionAviable(UnitType* U, TilePosition* t);


	TilePosition* getPosicionDistinta(TilePosition actual);

	// retorna true si una unidad del tipo pasado como parametro esta en construccion, false en caso contrario
	bool construyendo(int ID);

	// si hay unidades en el TilePosition pasado como parametro los mueve a otro build tile
	void moverUnidades(TilePosition *t); 
	
	// este metodo se invoca cuando se genera el evento onUnitCreate, y agrega la unidad en construccion a una lista
	void nuevaUnidadConstruccion(Unit *u);
	
	// retorna la primera unidad que haya sido terminada de construir que encuentre en la lista, NULL si no encuentra ninguna
	Unit* controlarFinalizacion();

	// manda un SCV a finalizar la construccion de la unidad pasada como parametro, ya que no fue finalizada y actualmente no esta siendo construida por ninguna unidad
	void finalizarConstruccion(Unit *u);
};
