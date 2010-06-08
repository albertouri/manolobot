#include "strategy_manager.h"
#include <BWAPI.h>

using namespace BWAPI;

int IdUnidades[34] = {112,123,111,12,125,106,107,115,117,11,122,113,32,1,3,120,0,32,124,14,108,118,110,7,116,9,30,5,114,109,58,2,13,8};
int cantUnidades[34];
int estadoActual = 0;


strategy_manager::strategy_manager(void)
{
	for (int x = 0; x < Utilidades::maxResearch; x++){
		ResearchDone[x] = false;
	}
}

strategy_manager::~strategy_manager(void)
{
}


void strategy_manager::checkGoals(void){
	UnitType * tipo;

	// quizas se puede reemplazar manteniendo la cantidad de unidades de cada tipo en el arreglo y en el evento
	// onUnitDestroy actualizar esa cantidad (para evitar el for)
	for (int i=0; i<34; i++){
		tipo = new UnitType(IdUnidades[i]);
		cantUnidades[i] = Broodwar->self()->allUnitCount(*tipo);

		delete tipo;
	}
	
	//no tengo una refineria
	if(cantUnidades[Utilidades::INDEX_GOAL_REFINERY] == 0){
		estadoActual = 0;
		GoalUnidades[Utilidades::INDEX_GOAL_REFINERY] = 1;
		GoalUnidades[Utilidades::INDEX_GOAL_SCV] = 10;
		GoalUnidades[Utilidades::INDEX_GOAL_DEPOT] = 3;
	} // no hay barraca
	else if (cantUnidades[Utilidades::INDEX_GOAL_BARRACK] == 0){
		estadoActual = 1;
		GoalUnidades[Utilidades::INDEX_GOAL_BARRACK] = 1;
	}
	else if (cantUnidades[Utilidades::INDEX_GOAL_BUNKER] < 3){
		GoalUnidades[Utilidades::INDEX_GOAL_BUNKER] = 3;
		GoalUnidades[Utilidades::INDEX_GOAL_MARINE] = 12;

		// Construyo un nuevo scv que se encargara de reparar los bunkers cuando sean atacados...
		//GoalUnidades[Utilidades::INDEX_GOAL_SCV]++;
	}
	else if (cantUnidades[Utilidades::INDEX_GOAL_ACADEMY] == 0){
		GoalUnidades[Utilidades::INDEX_GOAL_ACADEMY] = 1;
	}
	/*else if (!ResearchDone[Utilidades::INDEX_GOAL_STIMPACK]){
		GoalResearch[Utilidades::INDEX_GOAL_STIMPACK] = 1;

		// Setea la investigacion como completada, si el edificio que realiza la investigacion es destruido antes
		// de completarse la misma, se debera setear esta flag en false desde el evento onUnitDestroy, a menos
		// que exista mas de un edificio de este tipo
		ResearchDone[Utilidades::INDEX_GOAL_STIMPACK] = true;
	}*/
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