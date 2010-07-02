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
}

strategy_manager::~strategy_manager(void)
{
}


void strategy_manager::checkGoals(void){
	//UnitType * tipo;

	// quizas se puede reemplazar manteniendo la cantidad de unidades de cada tipo en el arreglo y en el evento
	// onUnitDestroy actualizar esa cantidad (para evitar el for)
	/*for (int i=0; i<34; i++){
		tipo = new UnitType(IdUnidades[i]);
		cantUnidades[i] = Broodwar->self()->allUnitCount(*tipo);

		delete tipo;
	}*/
	
	/*if (Broodwar->self()->supplyUsed() == Broodwar->self()->supplyTotal()){
		GoalUnidades[Utilidades::INDEX_GOAL_DEPOT]++;
		Broodwar->printf("Usado: %d - Total: %d", Broodwar->self()->supplyUsed(), Broodwar->self()->supplyTotal());
	}*/

	//no tengo una refineria
	if (cantUnidades[Utilidades::INDEX_GOAL_BARRACK] == 0){
		estadoActual = 0;
		GoalUnidades[Utilidades::INDEX_GOAL_BARRACK] = 1;
		GoalUnidades[Utilidades::INDEX_GOAL_SCV] = 10;
		GoalUnidades[Utilidades::INDEX_GOAL_DEPOT] = 3;
	} // no hay barraca
	else if (cantUnidades[Utilidades::INDEX_GOAL_BUNKER] < 3) {
		estadoActual = 1;
		GoalUnidades[Utilidades::INDEX_GOAL_BUNKER] = 3;
		GoalUnidades[Utilidades::INDEX_GOAL_MARINE] = 12;
	}
	else if(cantUnidades[Utilidades::INDEX_GOAL_REFINERY] == 0){

		GoalUnidades[Utilidades::INDEX_GOAL_REFINERY] = 1;
	}
	else if (cantUnidades[Utilidades::INDEX_GOAL_ACADEMY] == 0){
		GoalUnidades[Utilidades::INDEX_GOAL_ACADEMY] = 1;
		//GoalUnidades[Utilidades::INDEX_GOAL_BARRACK] = 2;
		GoalUnidades[Utilidades::INDEX_GOAL_FACTORY] = 1;
	}
	else if(cantUnidades[Utilidades::INDEX_GOAL_FACTORY] == 1){
		GoalUnidades[Utilidades::INDEX_GOAL_MACHINESHOP] = 1;
		GoalUnidades[Utilidades::INDEX_GOAL_TANKSIEGE] = 3;
	}
	else if (!ResearchDone[Utilidades::INDEX_GOAL_STIMPACK]){
		GoalUnidades[Utilidades::INDEX_GOAL_DEPOT] = 3;
		GoalUnidades[Utilidades::INDEX_GOAL_MARINE] = 12;
		//GoalUnidades[Utilidades::INDEX_GOAL_FIREBAT] = 5;
		//GoalUnidades[Utilidades::INDEX_GOAL_MEDIC] = 5;


		GoalResearch[Utilidades::INDEX_GOAL_STIMPACK] = 1;

		// Setea la investigacion como completada, si el edificio que realiza la investigacion es destruido antes
		// de completarse la misma, se debera setear esta flag en false desde el evento onUnitDestroy, a menos
		// que exista mas de un edificio de este tipo
		ResearchDone[Utilidades::INDEX_GOAL_STIMPACK] = true;

	}
	else if (!ResearchDone[Utilidades::INDEX_GOAL_U238]) {
		GoalResearch[Utilidades::INDEX_GOAL_U238] = 1;
		ResearchDone[Utilidades::INDEX_GOAL_U238] = true;
	}
}

int* strategy_manager::getGoals(){
	return GoalUnidades;
}

int* strategy_manager::getResearchs(){
	return GoalResearch;
}

void strategy_manager::onUnitCreate(Unit* u){
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
	}

	//Broodwar->printf("Se construyo un %d", u->getType().getID());
}

void strategy_manager::onUnitDestroy(Unit *u){
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
	}
}


bool strategy_manager::investigado(int ID_tech){
	// este metodo deberia retornar true si ya se termino de investigar esa mejora, por ahora retorna true si se empezo a investigar la mejora
	if ((ID_tech >= 0) && (ID_tech < Utilidades::maxResearch))
		return this->ResearchDone[ID_tech];
	else{
		Broodwar->printf("---------- Error en el metodo ""investigado"" de la clase strategy_manager ----------");
		return false;
	}
}