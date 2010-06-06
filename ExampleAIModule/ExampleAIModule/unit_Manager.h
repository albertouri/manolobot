#pragma once
#include <BWAPI.h>
#include "AnalizadorTerreno.h"
#include "Utilidades.h"
#include <stdlib.h>

using namespace BWAPI;


class unit_Manager
{
public:
	unit_Manager(void);
	void executeActions(AnalizadorTerreno *analizador);
	void resetBuildingSemaphore();
	
	void newSupplyDepot();
	void newBarrack();
	void newAcademy();

	void setGoals(int goals[34]);
	void setResearchs(int researchs[10]); // agregado por mi
	void asignarUnidadACompania(Unit* unit);
	~unit_Manager(void);

private:
	void buildUnit(TilePosition *pos, int id);
	void makeRefinery(TilePosition *pos);
	Unit* getWorker();
	Unit* getUnit(int IDTipo); // agregado por mi
	void sendGatherCristal(Unit* worker);
	void sendGatherGas(Unit* worker);
	void trainWorker();
	void trainMarine();
	TilePosition* getTilePositionAviable(UnitType* U);
	TilePosition* getTilePositionAviable(UnitType* U, TilePosition* t);

};
