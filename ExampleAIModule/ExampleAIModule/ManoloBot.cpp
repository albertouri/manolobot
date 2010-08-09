#include "ManoloBot.h"
#include "unit_Manager.h"
#include "strategy_manager.h"
#include "AnalizadorTerreno.h"
#include "Graficos.h"


unit_Manager *unitManager = NULL; // puntero al manager de unidades
strategy_manager *strategyManager = NULL;
AnalizadorTerreno *analizador = NULL;

int latency = 50;
int goals[34];


ManoloBot::ManoloBot(void)
{	
	//TilePosition *cc;
	/*for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		if ((*i)->getType().getID() == Utilidades::ID_COMMANDCENTER){
			
			cc = new TilePosition((*i)->getTilePosition());

			if ((cc->x() != 116) || (cc->y() != 40)) {
				Broodwar->restartGame();
				break;
			}

			Broodwar->printf("centro de comando x: %d, y: %d", cc->x(), cc->y());
		}
	}

	if ((cc->x() == 116) || (cc->y() == 40)) {*/
		unitManager = new unit_Manager();
		strategyManager = new strategy_manager();
		analizador = new AnalizadorTerreno();
	//}

}

void ManoloBot::checkGoals(void){

	Graficos::dibujarCuadro(new TilePosition(0,0), 1, 1);

	if ((strategyManager != NULL) && (unitManager != NULL)){
		if(latency >=50){
			strategyManager->setResearchsDone(unitManager->getResearchsDone());
			strategyManager->checkGoals();
			
			unitManager->setGoals(strategyManager->getGoals());
			unitManager->setResearchs(strategyManager->getResearchs());

			latency=0;
		}
		else{
			latency++;
		}
	}

	if (unitManager != NULL)
		unitManager->executeActions(analizador);
	
	if ((analizador != NULL) && (analizador->analisisListo()))
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

void ManoloBot::onNukeDetect(Position p){
	unitManager->onNukeDetect(p);
}