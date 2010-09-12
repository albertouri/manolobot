#include "ManoloBot.h"
#include "unit_Manager.h"
#include "strategy_manager.h"
//#include "AnalizadorTerreno.h"
#include "Graficos.h"


unit_Manager *unitManager = NULL; // puntero al manager de unidades
strategy_manager *strategyManager = NULL;
AnalizadorTerreno *analizador = NULL;

int latency = 50;
int goals[34];


ManoloBot::ManoloBot(void)
{
	strategyManager = new strategy_manager();
	analizador = new AnalizadorTerreno();
	unitManager = new unit_Manager(analizador);
}

void ManoloBot::checkGoals(void){

	//Graficos::dibujarCuadro(new TilePosition(0,0), 1, 1);

	if ((strategyManager != NULL) && (unitManager != NULL)){
		if(latency >=50){
			strategyManager->setResearchsDone(unitManager->getResearchsDone());
			strategyManager->checkGoals();
			
			unitManager->setGoals(strategyManager->getGoals());
			unitManager->setResearchs(strategyManager->getResearchs());
			unitManager->setEstadoActual(strategyManager->getEstadoActual());

			latency=0;
		}
		else{
			latency++;
		}
	}
	
	if (unitManager != NULL)
		unitManager->executeActions();
	

	if ((analizador != NULL) && (analizador->analisisListo())){
		//if (/*(analizador->getCuadrante(BWTA::getStartLocation(Broodwar->self())->getPosition()) == 2) || */(analizador->getCuadrante(BWTA::getStartLocation(Broodwar->self())->getPosition()) != 2)/* || (analizador->getCuadrante(BWTA::getStartLocation(Broodwar->self())->getPosition()) == 1)*/)
		//	Broodwar->restartGame();
		analizador->dibujarResultados();
	}
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

void ManoloBot::onNukeDetect(Position p){
	unitManager->onNukeDetect(p);
}

void ManoloBot::onUnitShow(Unit* u){
	unitManager->onUnitShow(u);
}
