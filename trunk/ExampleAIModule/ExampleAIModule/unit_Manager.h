#pragma once
#include <BWAPI.h>

using namespace BWAPI;


class unit_Manager
{
public:
	unit_Manager(void);
	void executeActions();
	void resetBuildingSemaphore();
	void newSupplyDepot();
	void newBarrack();
	void setGoals(int goals[34]);
	void asignarUnidadACompania(Unit* unit);
	~unit_Manager(void);
private:
	void buildUnit(TilePosition *pos, int id);
	void makeRefinery(TilePosition *pos);
	Unit* getWorker();
	void sendGatherCristal(Unit* worker);
	void sendGatherGas(Unit* worker);
	void trainWorker();
	void trainMarine();
	TilePosition* getTilePositionAviable(UnitType* U);



};
