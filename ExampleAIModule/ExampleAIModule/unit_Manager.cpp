#include "unit_Manager.h"
#include <BWAPI.h>
#include "compania.h"
#include "Scout.h"

using namespace BWAPI;

int cantBarracas, cantRefinerias = 0, cantAcademias = 0; // lleva la cuenta de la cantidad de barracas construidas
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
int goalResearch[10] = {0,0,0,0,0,0,0,0,0,0}; // arreglo que mantiene las investigaciones que deben realizarce
bool researchDone[10] = {false, false, false, false, false, false, false, false, false, false};

compania* Easy;
Scout* magallanes;


int investigue = false;


TilePosition *centroComando;	// mantiene la posicion del centro de comando
Unit *centroDeComando; //puntero a la posicion del centro;
UnitType *barraca; // puntero a la unidad que actualmente esta construyendo algo


unit_Manager::unit_Manager(void)
{
	Easy = new compania();
	magallanes = new Scout(getWorker());
	cantBarracas = 0;

	barraca = new UnitType(Utilidades::ID_ACADEMY);

	cantSCV = Broodwar->self()->completedUnitCount(*(new UnitType(Utilidades::ID_SCV)));
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

void unit_Manager::executeActions(AnalizadorTerreno *analizador){
	
	//construye 'goalLimiteSCV' de SCV, este valor deberia ser seteado por una llamada a setGoal
	magallanes->explorar();
	if((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_SCV))) < goalCantUnidades[Utilidades::INDEX_GOAL_SCV]) && (Broodwar->self()->minerals()>100) ) {
		trainWorker();
	}
	

	//metodo a corregir, solamente a fin de entrenar marines.
	if((Broodwar->self()->allUnitCount(*barraca))&&(Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_MARINE))) < goalCantUnidades[Utilidades::INDEX_GOAL_MARINE]) && (Broodwar->self()->minerals()>100) ) {
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
				else if (trabajador->isIdle()){ sendGatherCristal(trabajador); SCVgatheringCristal++;}
			}	
		}

		if ((Broodwar->self()->completedUnitCount(*(new UnitType(110)))>0) && (SCVgatheringCristal+SCVgatheringGas>10)){
			if (SCVgatheringGas < 4){
				for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
				{
					if (SCVgatheringGas<4){
						if ((*i)->getType().isWorker()){
							trabajador = (*i);
							if(trabajador->isGatheringMinerals()) {
								SCVgatheringCristal--;
								SCVgatheringGas++;
								sendGatherGas(trabajador); }
						}	
					}
					else{
						break;
					}
				}
			}
		}

	}
	else{
		frameLatency++;
	}

	if((Broodwar->self()->minerals()>200) && ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_REFINERY)))) <goalCantUnidades[Utilidades::INDEX_GOAL_REFINERY])&&(buildingSemaphore == 0)){
		TilePosition* pos = NULL;
		makeRefinery(pos);
		
	}


	if((Broodwar->self()->minerals() > 200)&& ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_DEPOT))))<goalCantUnidades[Utilidades::INDEX_GOAL_DEPOT])&&(buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_DEPOT);
		TilePosition* posB = getTilePositionAviable(building);
		buildUnit(posB, Utilidades::ID_DEPOT);
	}


	if((Broodwar->self()->minerals() > 200) && ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_BARRACK)))) <goalCantUnidades[Utilidades::INDEX_GOAL_BARRACK])&&(buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_BARRACK);
		TilePosition* posB = getTilePositionAviable(building);
		buildUnit(posB, Utilidades::ID_BARRACK);
	}

	// construye bunker
	if((Broodwar->self()->minerals() > 150)&& ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_BUNKER))))<goalCantUnidades[Utilidades::INDEX_GOAL_BUNKER])&&(buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_BUNKER);

		// Obtiene la posicion del centro del chokepoint a defender y la convierte a una TilePosition (que se 
		// mide en build tiles)
		Position *p = analizador->obtenerCentroChokepoint();
		TilePosition *t = new TilePosition(p->x() / 32, p->y() / 32);
		TilePosition* posB = getTilePositionAviable(building, t);

		delete t;

		//TilePosition* posB = getTilePositionAviable(building);
		buildUnit(posB, Utilidades::ID_BUNKER);
	}

	// construye academia
	if((Broodwar->self()->minerals() > 200)&& ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_ACADEMY))))<goalCantUnidades[Utilidades::INDEX_GOAL_ACADEMY])&&(buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_ACADEMY);
		TilePosition* posB = getTilePositionAviable(building);
		buildUnit(posB, Utilidades::ID_ACADEMY);
	}

	// ----------------------------------------------------------------------------
	//								Investigaciones
	// ----------------------------------------------------------------------------


	// stim_pack (se investiga en academia terran)
	// TODO: que entre una sola vez al ciclo, sino entra un monton de veces al pedo
	if ((cantAcademias > 0) && (Broodwar->self()->minerals() > 100) && (Broodwar->self()->gas() > 100) && (goalResearch[Utilidades::INDEX_GOAL_STIMPACK] == 1)){
		Unit *u;

		u = getUnit(Utilidades::ID_ACADEMY);

		if (u != NULL){

			if (u->isCompleted()){
				// Construccion de la academia finalizada, se puede investigar mejoras

				//Broodwar->printf("INVESTIGANDO MEJORA...");
				TechType *t = new TechType(TechTypes::Stim_Packs);
				//std::string nombre = "Stim_Packs";
				//TechType t = TechTypes::getTechType(nombre);

				u->research(*t);
				//delete t;
			}

			investigue = true;
		}
	}

}


unit_Manager::~unit_Manager(void)
{
}


void unit_Manager::buildUnit(TilePosition *pos, int id){
	Unit* trabajador;
	UnitType *tipo = new UnitType(id);
	if ((Broodwar->self()->minerals()>tipo->mineralPrice())&&(Broodwar->self()->gas()>tipo->gasPrice())){
		trabajador = getWorker();

		if (trabajador!=NULL) {
			if ( Broodwar->canBuildHere(trabajador, *(new Position(*pos)), *tipo )){
				buildingSemaphore++;
				trabajador->build((*pos), *tipo);
				
			}
		}
	}

}


void unit_Manager::makeRefinery(TilePosition *pos){
// el par�metro POS puede ser nulo, lo que indica que tiene que contruir la refineria en el geiser mas cercano
// si el par�metro tiene algun valor, se intenta construir sobre esa ubicaci�n.
	Unit *trabajador;
	TilePosition *geyserPos;

	if ((cantRefinerias < goalLimiteGeiser)&&(Broodwar->self()->minerals()>100)){
		trabajador = getWorker();
		Unit* closestGeiser=NULL;
		if (trabajador != NULL){
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
		
			if (Broodwar->self()->minerals()>150) {
				trabajador->build((*geyserPos), *(new UnitType(Utilidades::ID_REFINERY)));
				cantRefinerias++;
			}
		}

	}
}

Unit* unit_Manager::getWorker(){
	Unit* trabajador = NULL;
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

// retorna un puntero a la primera unidad del tipo pasado como parametro que encuentra
Unit* unit_Manager::getUnit(int IDTipo){

	Unit* u = NULL;

	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		if ((*i)->getType().getID() == IDTipo ){
				u = (*i);
				break;
		}
	}

	return u;
}


void unit_Manager::trainWorker(){
	if(centroDeComando->exists()){
		if(Broodwar->self()->minerals()>=150){	
			centroDeComando->train(Broodwar->self()->getRace().getWorker());
			cantSCV++;		
		}
	}
}

void unit_Manager::trainMarine(){
	Unit* firstBarrack = NULL;
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		if ((*i)->getType().getID() == Utilidades::ID_BARRACK){
			firstBarrack = (*i);
		}
	}

	if((Broodwar->self()->minerals()>50) && (firstBarrack != NULL)){	
		firstBarrack->train(*(new UnitType(0)));
	}

}

void unit_Manager::sendGatherCristal(Unit* worker){
	
	if (worker != NULL){
	Unit* closestMineral=NULL;
	//busca el mineral m�s cercano.
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

void unit_Manager::sendGatherGas(Unit* worker){
	
	if (worker != NULL){
	Unit* closestGeyser=NULL;
	//busca el mineral m�s cercano.
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		if ((*i)->getType().isRefinery()){
				closestGeyser = (*i);
		}
	}
	
	if (closestGeyser!=NULL) 
	{
		worker->rightClick(closestGeyser);
	}
	} else Broodwar->printf("el worker es null");

}



void unit_Manager::resetBuildingSemaphore(){
	buildingSemaphore=0;
}

// Obtiene un TilePosition disponible en las cercanias del centro de comando
TilePosition* unit_Manager::getTilePositionAviable(UnitType* U){
	TilePosition* pos;
	Unit* worker = getWorker();
	int x = centroComando->x();
	int y = centroComando->y();
	int i = 6;
	int j, k;
	int encontre=0;
	if (worker != NULL) {
		while (encontre==0){
			j = -i;
			if (x+j>=0){
				k = i;
				while((k>=-i) && (encontre==0)){
					if ((y+k>=0)&& (!((x+j>x-1) && (x+j<x+5) && (y+k>y-1) && (y+k<y+4)))){
						pos = new TilePosition(x + j, y + k);
						if(Broodwar->isExplored(*pos)){
							if (Broodwar->canBuildHere(worker, *pos, *U)) {encontre = 1;	/*Broodwar->printf("cord(%d , %d) quiero (%d , %d)", x, y, pos->x(), pos->y());*/}
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
							if (Broodwar->canBuildHere(worker, *pos, *U)) {encontre = 1;	/*Broodwar->printf("cord(%d , %d) quiero (%d , %d)", x, y, pos->x(), pos->y());*/}
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
						if (Broodwar->canBuildHere(worker, *pos, *U)) {encontre = 1;	/*Broodwar->printf("cord(%d , %d) quiero (%d , %d)", x, y, pos->x(), pos->y());*/}
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
						if (Broodwar->canBuildHere(worker, *pos, *U)) {encontre = 1;	/*Broodwar->printf("cord(%d , %d) quiero (%d , %d)", x, y, pos->x(), pos->y());*/}
					}
				}
				j = j-1;
			}

			i++;
		}
	}
	return pos;

}


// Obtiene un TilePosition disponible en las cercanias de la posicion pasada como parametro
TilePosition* unit_Manager::getTilePositionAviable(UnitType* U, TilePosition* t){
	TilePosition* pos;
	Unit* worker = getWorker();
	//int x = centroComando->x();
	//int y = centroComando->y();

	int x = t->x();
	int y = t->y();

	int i = 6;
	int j, k;
	int encontre=0;
	if (worker != NULL) {
		while (encontre==0){
			j = -i;
			if (x+j>=0){
				k = i;
				while((k>=-i) && (encontre==0)){
					if ((y+k>=0)&& (!((x+j>x-1) && (x+j<x+5) && (y+k>y-1) && (y+k<y+4)))){
						pos = new TilePosition(x + j, y + k);
						if(Broodwar->isExplored(*pos)){
							if (Broodwar->canBuildHere(worker, *pos, *U)) {encontre = 1;	/*Broodwar->printf("cord(%d , %d) quiero (%d , %d)", x, y, pos->x(), pos->y());*/}
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
							if (Broodwar->canBuildHere(worker, *pos, *U)) {encontre = 1;	/*Broodwar->printf("cord(%d , %d) quiero (%d , %d)", x, y, pos->x(), pos->y());*/}
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
						if (Broodwar->canBuildHere(worker, *pos, *U)) {encontre = 1;	/*Broodwar->printf("cord(%d , %d) quiero (%d , %d)", x, y, pos->x(), pos->y());*/}
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
						if (Broodwar->canBuildHere(worker, *pos, *U)) {encontre = 1;	/*Broodwar->printf("cord(%d , %d) quiero (%d , %d)", x, y, pos->x(), pos->y());*/}
					}
				}
				j = j-1;
			}

			i++;
		}
	}
	return pos;

}


void unit_Manager::newSupplyDepot(){
	cantSupplyDepot++;
}

void unit_Manager::newBarrack(){
	cantBarracas++;
}

void unit_Manager::newAcademy(){
	cantAcademias++;
}

void unit_Manager::setGoals(int goals[34]){
	for (int i=0; i<34; i++){
		goalCantUnidades[i] = goals[i];
	}
}

void unit_Manager::setResearchs(int researchs[10]){
	for (int i=0; i<10; i++){
		goalResearch[i] = researchs[i];
	}
}

void unit_Manager::asignarUnidadACompania(Unit* unit){
	Easy->asignarUnidad(unit);
}