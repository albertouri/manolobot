#include "ManoloBot.h"
#include "unit_Manager.h"
#include <BWAPI.h>

using namespace BWAPI;

unit_Manager *unitManager; // puntero al manager de unidades

ManoloBot::ManoloBot(void)
{	
	unitManager = new unit_Manager();
}

void ManoloBot::checkGoals(void){

	unitManager->executeActions();

}

void ManoloBot::edificioConstruido(int Id){
	
	if(Id == 109) unitManager->newSupplyDepot();
	else if(Id == 111) unitManager->newBarrack();
	unitManager->resetBuildingSemaphore();

}

ManoloBot::~ManoloBot(void)
{
}
