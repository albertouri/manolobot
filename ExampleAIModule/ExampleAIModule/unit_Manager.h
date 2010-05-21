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

	~unit_Manager(void);
private:
	void makeBarrack(TilePosition *pos);
	void makeSupplyDepot(TilePosition *pos);
	void makeRefinery(TilePosition *pos);
	Unit* getWorker();
	void sendGatherCristal(Unit* worker);
	void trainWorker();
	void trainMarine();
	TilePosition* getTilePositionAviable(UnitType* U);

};
