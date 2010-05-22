#include "unit_Manager.h"
#include <BWAPI.h>


using namespace BWAPI;

int cantBarracas, cantRefinerias = 0; // lleva la cuenta de la cantidad de barracas construidas
int cantSupplyDepot= 0;
int goalLimiteGeiser = 1;
int goalLimiteSCV = 8;
int goalLimiteBarracas = 1;
int cantSCV = 4;
int cantMarine = 0;
int SCVgatheringMinerals= 0, SCVgatheringGas = 0;
int frameLatency;
int buildingSemaphore =0;
int goalCantUnidades[34] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 

TilePosition *centroComando;	// mantiene la posicion del centro de comando
Unit *centroDeComando; //puntero a la posicion del centro;
UnitType *barraca; // puntero a la unidad que actualmente esta construyendo algo


unit_Manager::unit_Manager(void)
{
	//centroComando = new Position(100,100);
	cantBarracas=0;
	barraca = new UnitType(111);
	cantSCV = Broodwar->self()->completedUnitCount(*(new UnitType(7)));
	frameLatency = 0;

	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		if ((*i)->getType().isResourceDepot()){
			centroComando = new TilePosition((*i)->getTilePosition());
			centroDeComando = (*i);
			break;
		}
	}


}

void unit_Manager::executeActions(){
	
	//construye 'goalLimiteSCV' de SCV, este valor deberia ser seteado por una llamada a setGoal

	if((Broodwar->self()->allUnitCount(*(new UnitType(7))) < goalCantUnidades[23]) && (Broodwar->self()->minerals()>100) ) {
		trainWorker();
	}
	

	//metodo a corregir, solamente a fin de entrenar marines.
	if((Broodwar->self()->allUnitCount(*barraca))&&(Broodwar->self()->allUnitCount(*(new UnitType(0))) < goalCantUnidades[16]) && (Broodwar->self()->minerals()>100) ) {
		trainMarine();
	}



	if (frameLatency >= 200){
		int SCVgatheringCristal= 0, SCVgatheringGas = 0;
		Unit* trabajador;
		frameLatency=0;
		buildingSemaphore=0;
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
			if ((*i)->getType().isWorker()){
				trabajador = (*i);
				if(trabajador->isGatheringMinerals()) SCVgatheringCristal++;
				else if (trabajador->isGatheringGas())SCVgatheringGas++;
				else if (trabajador->isIdle()){ sendGatherCristal(trabajador); }
			}
	
		}

	}
	else{
		frameLatency++;
	}

	if((Broodwar->self()->minerals()>200) && ((Broodwar->self()->allUnitCount(*(new UnitType(110)))) <goalCantUnidades[22])&&(buildingSemaphore == 0)){
		TilePosition* pos = NULL;
		makeRefinery(pos);
		
	}

	if((Broodwar->self()->minerals()>200) && ((Broodwar->self()->allUnitCount(*(new UnitType(111)))) <goalCantUnidades[2])&&(buildingSemaphore == 0)){
		UnitType* building = new UnitType(111);
		TilePosition* posB = getTilePositionAviable(building);
		makeBarrack(posB);

	}

	if((Broodwar->self()->minerals()>200)&& ((Broodwar->self()->allUnitCount(*(new UnitType(109))))<goalCantUnidades[29])&&(buildingSemaphore == 0)){
		UnitType* building = new UnitType(109);
		TilePosition* posB = getTilePositionAviable(building);
		makeSupplyDepot(posB);
	}
		
	
}


unit_Manager::~unit_Manager(void)
{
}


void unit_Manager::makeBarrack(TilePosition *pos){
	Unit* trabajador;
	if (Broodwar->self()->minerals()>150){
		trabajador = getWorker();

		if ( Broodwar->canBuildHere(trabajador, *(new Position(*pos)), *(new UnitType(111)) )){
			if (trabajador!=NULL) {
				buildingSemaphore++;
				trabajador->build((*pos), *(new UnitType(111)));
				
			}
		}
	}

}

void unit_Manager::makeSupplyDepot(TilePosition *pos){
	Unit* trabajador;
	
	if (Broodwar->self()->minerals()>150){	
		trabajador = getWorker();
		
		if (trabajador!=NULL) {
			if ( Broodwar->canBuildHere(trabajador, *(new Position(*pos)), *(new UnitType(109)) )){
				buildingSemaphore++;
				trabajador->build((*pos), *(new UnitType(109)));

			}
		}

	}
}


void unit_Manager::makeRefinery(TilePosition *pos){
// el parámetro POS puede ser nulo, lo que indica que tiene que contruir la refineria en el geiser mas cercano
// si el parámetro tiene algun valor, se intenta construir sobre esa ubicación.
	Unit *trabajador;
	TilePosition *geyserPos;

	if ((cantRefinerias < goalLimiteGeiser)&&(Broodwar->self()->minerals()>100)){
		trabajador = getWorker();
		Unit* closestGeiser=NULL;
		if (pos == NULL){
			
			for(std::set<Unit*>::iterator i=Broodwar->getGeysers().begin();i!=Broodwar->getGeysers().end();i++)
			{
					if (closestGeiser==NULL) closestGeiser=*i;
					if (trabajador->getDistance(*i)<trabajador->getDistance(closestGeiser)) closestGeiser=*i;

			}
			geyserPos = new TilePosition(closestGeiser->getTilePosition());
		}
		else{
			*geyserPos = (*pos);
		}
	
		if ((trabajador!=NULL)&&(Broodwar->self()->minerals()>150)) {
			trabajador->build((*geyserPos), *(new UnitType(110)));
			cantRefinerias++;
		}

	}
}

Unit* unit_Manager::getWorker(){
	Unit* trabajador;
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
			if ((*i)->getType().isWorker()){
				if (!(*i)->isConstructing()){
					trabajador = *i;
					break;
				}
			}
		}
	return trabajador;

}

void unit_Manager::trainWorker(){
	if(Broodwar->self()->minerals()>=150){	
		centroDeComando->train(*Broodwar->self()->getRace().getWorker());
		cantSCV++;		
	}

}

void unit_Manager::trainMarine(){
	Unit* firstBarrack;
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		if ((*i)->getType().getID()==111){
			firstBarrack = (*i);
		}
	}

	if(Broodwar->self()->minerals()>50){	
		firstBarrack->train(*(new UnitType(0)));
		cantMarine++;		
	}

}

void unit_Manager::sendGatherCristal(Unit* worker){
	
	if (worker != NULL){
	Unit* closestMineral=NULL;
	//busca el mineral más cercano.
	if (closestMineral == NULL) {
		for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++)
		{
			if (closestMineral==NULL || centroDeComando->getDistance(*m)<centroDeComando->getDistance(closestMineral))
				closestMineral=*m;
		}
	}

	if (closestMineral!=NULL) 
	{
		worker->rightClick(closestMineral);
		
	}
	} else Broodwar->printf("el worker es null");

}

void unit_Manager::resetBuildingSemaphore(){
	buildingSemaphore=0;
}

TilePosition* unit_Manager::getTilePositionAviable(UnitType* U){
	TilePosition* pos;
	Unit* worker = getWorker();
	int x = centroComando->x();
	int y = centroComando->y();
	int i = 6;
	int j, k;
	int encontre=0;

	while (encontre==0){
		j = -i;
		if (x+j>=0){
			k = i;
			while((k>=-i) && (encontre==0)){
				if ((y+k>=0)&& (!((x+j>x-1) && (x+j<x+5) && (y+k>y-1) && (y+k<y+4)))){
					pos = new TilePosition(x + j, y + k);
					if(Broodwar->isExplored(*pos)){
						if (Broodwar->canBuildHere(worker, *pos, *U)) {encontre = 1;	Broodwar->printf("cord(%d , %d) quiero (%d , %d)", x, y, pos->x(), pos->y());}
					}
				}
				k = k-1;
			}
		}
		k = -i;
		if (y+k>=0){
			j = i;
			while((j>=-i) && (encontre==0)){
				if ((x+j>=0) && (!((x+j>x-1) && (x+j<x+5) && (y+k>y-1) && (y+k<y+4)))){
					pos = new TilePosition(x + j, y + k);
					if(Broodwar->isExplored(*pos)){
						if (Broodwar->canBuildHere(worker, *pos, *U)) {encontre = 1;	Broodwar->printf("cord(%d , %d) quiero (%d , %d)", x, y, pos->x(), pos->y());}
					}
				}
				j = j-1;
			}
		}
		
		j = i;
		k = i;
		while((k>=-i) && (encontre==0)){
			if ((y+k>=0)&& (!((x+j>x-1) && (x+j<x+5) && (y+k>y-1) && (y+k<y+4)))){
				pos = new TilePosition(x + j, y + k);
				if(Broodwar->isExplored(*pos)){
					if (Broodwar->canBuildHere(worker, *pos, *U)) {encontre = 1;	Broodwar->printf("cord(%d , %d) quiero (%d , %d)", x, y, pos->x(), pos->y());}
				}
			}
			k = k-1;
		}

		k=i;
		j = i;
		while((j>=-i) && (encontre==0)){
			if ((x+j>=0) && (!((x+j>x-1) && (x+j<x+5) && (y+k>y-1) && (y+k<y+4)))){
				pos = new TilePosition(x + j, y + k);
				if(Broodwar->isExplored(*pos)){
					if (Broodwar->canBuildHere(worker, *pos, *U)) {encontre = 1;	Broodwar->printf("cord(%d , %d) quiero (%d , %d)", x, y, pos->x(), pos->y());}
				}
			}
			j = j-1;
		}

		i++;
	}

	return pos;

}

void unit_Manager::newSupplyDepot(){
	cantSupplyDepot++;
}

void unit_Manager::newBarrack(){
	cantBarracas++;
}

void unit_Manager::setGoals(int goals[34]){
	for (int i=0; i<34; i++){
		goalCantUnidades[i] = goals[i];
	}
}