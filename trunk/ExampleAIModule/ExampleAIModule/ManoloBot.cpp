#include "ManoloBot.h"
#include "unit_Manager.h"
#include "strategy_manager.h"
#include "AnalizadorTerreno.h"
#include <BWAPI.h>

using namespace BWAPI;

unit_Manager *unitManager; // puntero al manager de unidades
strategy_manager *strategyManager;
int latency = 50;
int goals[34];

AnalizadorTerreno *analizador;


ManoloBot::ManoloBot(void)
{	
	unitManager = new unit_Manager();
	strategyManager = new strategy_manager();
	analizador = new AnalizadorTerreno();
}

void ManoloBot::checkGoals(void){

	if(latency >=50){
		strategyManager->checkGoals();
		
		unitManager->setGoals(strategyManager->getGoals());
		unitManager->setResearchs(strategyManager->getResearchs());
		
		latency=0;
	}
	else{
		latency++;
	}

	unitManager->executeActions(analizador);
	analizador->dibujar();

}



void ManoloBot::edificioConstruido(int Id){
	
	if(Id == 109) unitManager->newSupplyDepot();
	else if(Id == 111) unitManager->newBarrack();
	else if(Id == Utilidades::ID_ACADEMY) unitManager->newAcademy();

	unitManager->resetBuildingSemaphore();

}

void ManoloBot::unidadConstruida(Unit* U){
	
	unitManager->asignarUnidadACompania(U);
}

ManoloBot::~ManoloBot(void)
{
}

