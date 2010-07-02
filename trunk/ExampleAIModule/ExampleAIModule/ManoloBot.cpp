#include "ManoloBot.h"
#include "unit_Manager.h"
#include "strategy_manager.h"
#include "AnalizadorTerreno.h"
#include "Graficos.h"


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
	analizador->dibujarResultados();

}

void ManoloBot::onUnitCreate(Unit* u){
	strategyManager->onUnitCreate(u);
	unitManager->onUnitCreate(u);
}

ManoloBot::~ManoloBot(void)
{
}

void ManoloBot::onUnitDestroy(Unit *u){
	strategyManager->onUnitDestroy(u);
	unitManager->onUnitDestroy(u);
}