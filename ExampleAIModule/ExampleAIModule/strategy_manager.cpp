#include "strategy_manager.h"
#include <BWAPI.h>

using namespace BWAPI;

int IdUnidades[34] = {112,123,111,12,125,106,107,115,117,11,122,113,32,1,3,120,0,34,124,14,108,118,110,7,116,9,30,5,114,109,58,2,13,8};
int estadoActual = 0;


strategy_manager::strategy_manager(void)
{
	for (int x = 0; x < 34; x++){
		this->cantUnidades[x] = 0;
		this->GoalUnidades[x] = 0;
	}

	cantUnidades[Utilidades::INDEX_GOAL_SCV] = 4;
	cantUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER] = 1;

	for (int x = 0; x < Utilidades::maxResearch; x++){
		this->ResearchDone[x] = false;
		this->GoalResearch[x] = 0;
	}

	estadoActual = 0;
}

strategy_manager::~strategy_manager(void)
{
}


void strategy_manager::checkGoals(void){

	// Realiza un control para ver si es necesario construir un nuevo supply depot
	if (Broodwar->getFrameCount() % 20 == 0){
		if (Broodwar->self()->supplyUsed()+4 >= Broodwar->self()->supplyTotal()){
			GoalUnidades[Utilidades::INDEX_GOAL_DEPOT]++;
		}
	}

	if (estadoActual == 0){
		
		if (cantUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER] == 0){
			GoalUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER] = 1;
		}
		else if (cantUnidades[Utilidades::INDEX_GOAL_BARRACK] == 0){
			estadoActual = 0;
			GoalUnidades[Utilidades::INDEX_GOAL_BARRACK] = 1;
			GoalUnidades[Utilidades::INDEX_GOAL_SCV] = 9;
			GoalUnidades[Utilidades::INDEX_GOAL_MARINE] = 10;
		}
		else if ((cantUnidades[Utilidades::INDEX_GOAL_BARRACK] == 1)&&(cantUnidades[Utilidades::INDEX_GOAL_MARINE] < 4)){
			GoalUnidades[Utilidades::INDEX_GOAL_MARINE] = 22;
			GoalUnidades[Utilidades::INDEX_GOAL_BUNKER] = 3;
		}
		else if ((cantUnidades[Utilidades::INDEX_GOAL_BARRACK] == 1) && (cantUnidades[Utilidades::INDEX_GOAL_BUNKER] >= 1)&& (cantUnidades[Utilidades::INDEX_GOAL_MARINE] > 10)){
			GoalUnidades[Utilidades::INDEX_GOAL_BARRACK] = 2;
		}
		else if(cantUnidades[Utilidades::INDEX_GOAL_BARRACK] == 2){
			Broodwar->printf("Pase al estado 1 en el strategy manager");
			estadoActual = 1;
		}
	}
	else if (estadoActual == 1){
		if(cantUnidades[Utilidades::INDEX_GOAL_REFINERY] == 0){
			GoalUnidades[Utilidades::INDEX_GOAL_REFINERY] = 1;
		}
		else if (cantUnidades[Utilidades::INDEX_GOAL_ACADEMY] == 0){
			GoalUnidades[Utilidades::INDEX_GOAL_ACADEMY] = 1;
			//GoalUnidades[Utilidades::INDEX_GOAL_SCV] = 15;
		}
		else if ((cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY] == 0) && (cantUnidades[Utilidades::INDEX_GOAL_MARINE] > 10)){
			GoalUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY] = 1;
		}
		
		if ((Broodwar->self()->minerals() > 50) && (Broodwar->self()->gas() > 50) && (cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY] > 0)){
			Broodwar->printf("Pase al estado 2 en el strategy manager");
			estadoActual = 2;
		}
	}
	else if (estadoActual == 2){
		if ((cantUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER] > 0) && (cantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION] == 0)){
			GoalUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION] = 1;
			GoalUnidades[Utilidades::INDEX_GOAL_FACTORY] = 1;
		}


		if (cantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION] == 1){
			GoalUnidades[Utilidades::INDEX_GOAL_SCV] = 15;
			Broodwar->printf("Pase al estado 3 en el strategy manager");
			estadoActual = 3;
		}
	}
	else if (estadoActual == 3){
		if (!ResearchDone[Utilidades::INDEX_GOAL_U238]) {
			GoalResearch[Utilidades::INDEX_GOAL_U238] = 1;
			//GoalUnidades[Utilidades::INDEX_GOAL_MISSILE_TURRET] = 2;
			GoalUnidades[Utilidades::INDEX_GOAL_MARINE] = 27;
		}
		else if ((cantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION] > 0) && (!ResearchDone[Utilidades::INDEX_GOAL_STIMPACK])){
			GoalUnidades[Utilidades::INDEX_GOAL_MACHINESHOP] = 1;
			GoalUnidades[Utilidades::INDEX_GOAL_TANKSIEGE] = 3;
			
			//GoalUnidades[Utilidades::INDEX_GOAL_MEDIC] = 6;
			//GoalUnidades[Utilidades::INDEX_GOAL_MARINE] = 22;
			GoalResearch[Utilidades::INDEX_GOAL_STIMPACK] = 1;

			// Setea la investigacion como completada, si el edificio que realiza la investigacion es destruido antes
			// de completarse la misma, se debera setear esta flag en false desde el evento onUnitDestroy, a menos
			// que exista mas de un edificio de este tipo
			//ResearchDone[Utilidades::INDEX_GOAL_STIMPACK] = true;
		}
		else if ((cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY] > 0) && (cantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP] == 1) && (!ResearchDone[Utilidades::INDEX_GOAL_TANK_SIEGE_MODE])) {
			GoalResearch[Utilidades::INDEX_GOAL_TANK_SIEGE_MODE] = 1;
			GoalUnidades[Utilidades::INDEX_GOAL_MISSILE_TURRET] = 4;
		}
		else if (cantUnidades[Utilidades::INDEX_GOAL_ARMORY] == 0){
			GoalUnidades[Utilidades::INDEX_GOAL_ARMORY] = 1;
			//GoalUnidades[Utilidades::INDEX_GOAL_MISSILE_TURRET] = 6;
		}
		/*else if (cantUnidades[Utilidades::INDEX_GOAL_STARPORT] == 0){
			GoalUnidades[Utilidades::INDEX_GOAL_STARPORT] = 1;
		}*/
		else if ((cantUnidades[Utilidades::INDEX_GOAL_ARMORY] > 0) && (!ResearchDone[Utilidades::INDEX_GOAL_VEHICLE_WEAPONS_LVL1])){
			//Broodwar->printf("Deberia investigar mejora armas vehiculos cuando tenga mas de 100 100");
			GoalResearch[Utilidades::INDEX_GOAL_VEHICLE_WEAPONS_LVL1] = 1;
		}
		else if (!ResearchDone[Utilidades::INDEX_GOAL_INFANTRY_WEAPONS_LVL1]){
			GoalResearch[Utilidades::INDEX_GOAL_INFANTRY_WEAPONS_LVL1] = 1;
			GoalResearch[Utilidades::INDEX_GOAL_RESTORATION] = 1;
			GoalUnidades[Utilidades::INDEX_GOAL_MEDIC] = 6;
			GoalUnidades[Utilidades::INDEX_GOAL_GOLIATH] = 4;
			GoalUnidades[Utilidades::INDEX_GOAL_TANKSIEGE] = 7;
		}
		else if (!ResearchDone[Utilidades::INDEX_GOAL_INFANTRY_ARMOR_LVL1]){
			GoalResearch[Utilidades::INDEX_GOAL_INFANTRY_ARMOR_LVL1] = 1;
			//GoalUnidades[Utilidades::INDEX_GOAL_BUNKER] = 6;
		}
	}
}

int* strategy_manager::getGoals(){
	return GoalUnidades;
}

int* strategy_manager::getResearchs(){
	return GoalResearch;
}

void strategy_manager::setResearchsDone(bool *rd){
	for (int x = 0; x < Utilidades::maxResearch; x++)
		ResearchDone[x] = rd[x];
}

void strategy_manager::onUnitCreate(Unit* u){
	
	if(Broodwar->self()== u->getPlayer()){

		switch (u->getType().getID()){
			case Utilidades::ID_ACADEMY:
				cantUnidades[Utilidades::INDEX_GOAL_ACADEMY]++;
				break;
			case Utilidades::ID_BARRACK:
				cantUnidades[Utilidades::INDEX_GOAL_BARRACK]++;
				break;
			case Utilidades::ID_BUNKER:
				cantUnidades[Utilidades::INDEX_GOAL_BUNKER]++;
				break;
			case Utilidades::ID_DEPOT:
				cantUnidades[Utilidades::INDEX_GOAL_DEPOT]++;
				break;
			case Utilidades::ID_FIREBAT:
				cantUnidades[Utilidades::INDEX_GOAL_FIREBAT]++;
				break;
			case Utilidades::ID_MARINE:
				cantUnidades[Utilidades::INDEX_GOAL_MARINE]++;
				break;
			case Utilidades::ID_MEDIC:
				cantUnidades[Utilidades::INDEX_GOAL_MEDIC]++;
				break;
			case Utilidades::ID_REFINERY:
				cantUnidades[Utilidades::INDEX_GOAL_REFINERY]++;
				break;
			case Utilidades::ID_SCV:
				cantUnidades[Utilidades::INDEX_GOAL_SCV]++;
				break;
			case Utilidades::ID_FACTORY:
				cantUnidades[Utilidades::INDEX_GOAL_FACTORY]++;
				break;
			case Utilidades::ID_MACHINESHOP:
				cantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP]++;
				break;
			case Utilidades::ID_TANKSIEGE:
				cantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE]++;
				break;
			case Utilidades::ID_TANKSIEGE_SIEGEMODE:
				cantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE]++;
				break;
			case Utilidades::ID_ENGINEERING_BAY:
				cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY]++;
				break;
			case Utilidades::ID_ARMORY:
				cantUnidades[Utilidades::INDEX_GOAL_ARMORY]++;
				break;
			case Utilidades::ID_STARPORT:
				cantUnidades[Utilidades::INDEX_GOAL_STARPORT]++;
				break;
			case Utilidades::ID_COMSAT_STATION:
				cantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION]++;
				break;
			case Utilidades::ID_MISSILE_TURRET:
				cantUnidades[Utilidades::INDEX_GOAL_MISSILE_TURRET]++;
				break;
			case Utilidades::ID_GOLIATH:
				cantUnidades[Utilidades::INDEX_GOAL_GOLIATH]++;
				break;
		}
		
	}
}

void strategy_manager::onUnitDestroy(Unit *u){
	
	if(Broodwar->self()== u->getPlayer()){

		switch (u->getType().getID()){
			case Utilidades::ID_ACADEMY:
				cantUnidades[Utilidades::INDEX_GOAL_ACADEMY]--;
				break;
			case Utilidades::ID_BARRACK:
				cantUnidades[Utilidades::INDEX_GOAL_BARRACK]--;
				break;
			case Utilidades::ID_BUNKER:
				cantUnidades[Utilidades::INDEX_GOAL_BUNKER]--;
				break;
			case Utilidades::ID_DEPOT:
				cantUnidades[Utilidades::INDEX_GOAL_DEPOT]--;
				break;
			case Utilidades::ID_FIREBAT:
				cantUnidades[Utilidades::INDEX_GOAL_FIREBAT]--;
				break;
			case Utilidades::ID_MARINE:
				cantUnidades[Utilidades::INDEX_GOAL_MARINE]--;
				break;
			case Utilidades::ID_MEDIC:
				cantUnidades[Utilidades::INDEX_GOAL_MEDIC]--;
				break;
			case Utilidades::ID_REFINERY:
				cantUnidades[Utilidades::INDEX_GOAL_REFINERY]--;
				break;
			case Utilidades::ID_SCV:
				cantUnidades[Utilidades::INDEX_GOAL_SCV]--;
				break;
			case Utilidades::ID_FACTORY:
				cantUnidades[Utilidades::INDEX_GOAL_FACTORY]--;
				break;
			case Utilidades::ID_MACHINESHOP:
				cantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP]--;
				break;
			case Utilidades::ID_TANKSIEGE:
				cantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE]--;
				break;
			case Utilidades::ID_TANKSIEGE_SIEGEMODE:
				cantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE]--;
				break;
			case Utilidades::ID_ENGINEERING_BAY:
				cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY]--;
				break;
			case Utilidades::ID_ARMORY:
				cantUnidades[Utilidades::INDEX_GOAL_ARMORY]--;
				break;
			case Utilidades::ID_STARPORT:
				cantUnidades[Utilidades::INDEX_GOAL_STARPORT]--;
				break;
			case Utilidades::ID_COMSAT_STATION:
				cantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION]--;
				break;
			case Utilidades::ID_MISSILE_TURRET:
				cantUnidades[Utilidades::INDEX_GOAL_MISSILE_TURRET]--;
				break;
			case Utilidades::ID_GOLIATH:
				cantUnidades[Utilidades::INDEX_GOAL_GOLIATH]--;
				break;
		}
		
	}
}
