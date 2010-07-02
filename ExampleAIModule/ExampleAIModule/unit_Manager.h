#pragma once
#include <BWAPI.h>
#include "AnalizadorTerreno.h"
#include "Utilidades.h"
#include <stdlib.h>

using namespace BWAPI;
using namespace BWTA;


class unit_Manager
{
public:
	unit_Manager();
	void executeActions(AnalizadorTerreno *analizador);
	void resetBuildingSemaphore();
	
	void newSupplyDepot();
	void newBarrack();
	void newAcademy();

	void setGoals(int goals[34]);
	void setResearchs(int researchs[10]);
	void asignarUnidadACompania(Unit* unit);
	
	void verificarBunkers(); // verifica si un bunker esta siendo atacado
	void repararUnidad(Unit *u); // repara la unidad pasada como parametro

	void onUnitCreate(Unit *u);
	void onUnitDestroy(Unit *u);

	~unit_Manager(void);

private:
	void buildUnit(TilePosition *pos, int id);
	void makeRefinery(TilePosition *pos);
	Unit* getWorker();
	Unit* getUnit(int IDTipo);
	void sendGatherCristal(Unit* worker);
	void sendGatherGas(Unit* worker);
	void trainWorker();
	void trainMarine();
	void trainMedic();
	void trainTankSiege();

	TilePosition* getTilePositionAviable(UnitType* U); // despues de usar este metodo ejecutar delete sobre el resultado para liberar memoria
	TilePosition* getTilePositionAviable(UnitType* U, TilePosition* t); // despues de usar este metodo ejecutar delete sobre el resultado para liberar memoria

	bool construyendo(int ID); // retorna true si una unidad del tipo pasado como parametro esta en construccion, false en caso contrario
	void moverUnidades(TilePosition *t); // si hay unidades en el TilePosition pasado como parametro los mueve a otro build tile


	int goalResearch[Utilidades::maxResearch]; // arreglo que mantiene las investigaciones que deben realizarce
	bool researchDone[Utilidades::maxResearch]; // arreglo que mantiene informacion sobre si una investigacion se realizo o no
	
	Unit *reparador1, *reparador2; // puntero a un SCV que se encargara de reparar los bunkers que sean atacados

	void nuevaUnidadConstruccion(Unit *u); // este metodo se invoca cuando se genera el evento onUnitCreate, y agrega la unidad en construccion a una lista
	Unit* controlarFinalizacion(); // retorna la primera unidad que haya sido terminada de construir que encuentre en la lista, NULL si no encuentra ninguna

	int cantUnidades[34];
};
