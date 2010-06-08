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
	unit_Manager(void);
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
	TilePosition* getTilePositionAviable(UnitType* U);
	TilePosition* getTilePositionAviable(UnitType* U, TilePosition* t);
	int goalResearch[Utilidades::maxResearch]; // arreglo que mantiene las investigaciones que deben realizarce
	bool researchDone[Utilidades::maxResearch]; // arreglo que mantiene informacion sobre si una investigacion se realizo o no
	Unit *reparador; // puntero a un SCV que se encargara de reparar los bunkers que sean atacados

	void resaltarUnidad(Unit *u);
	TilePosition* ubicarBunker(Region *r, Chokepoint *c);
	void dibujarCuadro(TilePosition* p, int tilesAncho, int tilesAlto);

};
