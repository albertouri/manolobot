#include "strategy_manager.h"
#include <BWAPI.h>

using namespace BWAPI;

int IdUnidades[34] = {112,123,111,12,125,106,107,115,117,11,122,113,32,1,3,120,0,32,124,14,108,118,110,7,116,9,30,5,114,109,58,2,13,8};
int cantUnidades[34];
int estadoActual = 0;
int GoalUnidades[34];

strategy_manager::strategy_manager(void)
{
}

strategy_manager::~strategy_manager(void)
{
}


void strategy_manager::checkGoals(void){
	UnitType * tipo;
	for (int i=0; i<34; i++){
		tipo = new UnitType(IdUnidades[i]);
		cantUnidades[i] = Broodwar->self()->allUnitCount(*tipo);
	}
	
	//no tengo una refineria
	if(cantUnidades[22]==0){
		estadoActual = 0;
		GoalUnidades[22]=1;
		GoalUnidades[23]=10;
		GoalUnidades[29]=3;
	} // no hay barraca
	else if (cantUnidades[2]==0){
		estadoActual = 1;
		GoalUnidades[2]=1;
	}
	else {
		GoalUnidades[4]=2;
		GoalUnidades[16]=10;
	}



}

int* strategy_manager::getGoals(){
	return GoalUnidades;
}