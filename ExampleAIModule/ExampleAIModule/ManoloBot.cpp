#include "ManoloBot.h"
#include "unit_Manager.h"
#include "strategy_manager.h"
#include <BWAPI.h>

using namespace BWAPI;

unit_Manager *unitManager; // puntero al manager de unidades
strategy_manager *strategyManager;
int latency = 50;
int goals[34];


ManoloBot::ManoloBot(void)
{	
	unitManager = new unit_Manager();
	strategyManager = new strategy_manager();
}

void ManoloBot::checkGoals(void){		
	if(latency >=50){
		strategyManager->checkGoals();
		unitManager->setGoals(strategyManager->getGoals());
		latency=0;
	}
	else{
		latency++;
	}
	unitManager->executeActions();
}

void ManoloBot::edificioConstruido(int Id){
	
	if(Id == 109) unitManager->newSupplyDepot();
	else if(Id == 111) unitManager->newBarrack();
	unitManager->resetBuildingSemaphore();

}

void ManoloBot::unidadConstruida(Unit* U){
	
	unitManager->asignarUnidadACompania(U);
}

ManoloBot::~ManoloBot(void)
{
}
