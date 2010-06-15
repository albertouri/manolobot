#include "unit_Manager.h"
#include "compania.h"
#include "Scout.h"
#include "Graficos.h"
#include <math.h>
#include "GrupoBunkers.h"

int cantBarracas, cantRefinerias = 0, cantAcademias = 0; // lleva la cuenta de la cantidad de barracas construidas
int cantSupplyDepot= 0;
int cantMarines = 0;
int cantMedics = 0;
int cantFirebats = 0;

int goalLimiteGeiser = 1;
int goalLimiteSCV = 8;
int goalLimiteBarracas = 1;
int cantSCV = 4;
int cantMarine = 0;
int SCVgatheringMinerals= 0, SCVgatheringGas = 0;
int frameLatency;
int buildingSemaphore =0;

int goalCantUnidades[34] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 

compania* Easy;
compania* Otra;

Scout* magallanes;
Player* enemigo;

TilePosition *centroComando;	// mantiene la posicion del centro de comando
Unit *centroDeComando; //puntero a la posicion del centro;
UnitType *barraca; // puntero a la unidad que actualmente esta construyendo algo

TilePosition *posUltimoBunker = NULL; // puntero a la posicion donde se construyo el ultimo bunker
GrupoBunkers *grupoB1;

std::list<Unit*> unidadesEnConstruccion; // Lista de unidades que estan en construccion actualmente
Unit *ultimaFinalizada = NULL; // puntero a la ultima unidad finalizada, se calcula en cada frame con una llamada a controlarFinalizacion()

unit_Manager::unit_Manager(void)
{
	Easy = new compania();
	Otra = new compania();

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

	for (int x = 0; x < Utilidades::maxResearch; x++){
		goalResearch[x] = 0;
		researchDone[x] = false;
	}

	reparador1 = NULL;
	reparador2 = NULL;


	// supongo que jugamos contra un solo enemigo
	// TODO: arreglar para que se puede jugar contra varios enemigos
	enemigo = Broodwar->enemy();

	// Crea un nuevo grupo de bunkers
	grupoB1 = new GrupoBunkers();

}

void unit_Manager::executeActions(AnalizadorTerreno *analizador){
	//Unit *fofo;

	// manda al scout a explorar el mapa
	magallanes->explorar();

	Graficos::resaltarUnidad(reparador1);
	Graficos::resaltarUnidad(reparador2);

	// verifica si se termino de construir alguna unidad en este frame
	//ultimaFinalizada = controlarFinalizacion();

	if (ultimaFinalizada != NULL){
		if (ultimaFinalizada->getType().getID() == Utilidades::ID_MARINE) cantMarines++;
		else if (ultimaFinalizada->getType().getID() == Utilidades::ID_MEDIC) cantMedics++;
		else if (ultimaFinalizada->getType().getID() == Utilidades::ID_FIREBAT) cantFirebats++;
	}

	// verifica daños en los bunkers
	verificarBunkers();

	/*fofo = grupoB1->getUltimoBunkerCreado();
	if (fofo != NULL){
		grupoB1->estrategia1(fofo);
	}*/


	// Si la cantidad de suministros usados es igual a la cantidad maxima, se construye un nuevo deposito
	/*if (Broodwar->self()->supplyUsed() == Broodwar->self()->supplyTotal()){
		if (!construyendo(Utilidades::ID_DEPOT))
			goalCantUnidades[Utilidades::INDEX_GOAL_DEPOT]++;
	}*/

	//-----------------------------------------------------

	//construye 'goalLimiteSCV' de SCV, este valor deberia ser seteado por una llamada a setGoal
	if((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_SCV))) < goalCantUnidades[Utilidades::INDEX_GOAL_SCV]) && (Broodwar->self()->minerals()>100) ) {
		trainWorker();
	}
	

	//metodo a corregir, solamente a fin de entrenar marines.
	if((Broodwar->self()->allUnitCount(*barraca))&&(Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_MARINE))) < goalCantUnidades[Utilidades::INDEX_GOAL_MARINE]) && (Broodwar->self()->minerals()>100) ) {
		trainMarine();
	}

	if((Broodwar->self()->allUnitCount(*barraca))&&(Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_MEDIC))) < goalCantUnidades[Utilidades::INDEX_GOAL_MEDIC]) && (Broodwar->self()->minerals()>= 50) && (Broodwar->self()->gas()>= 25)) {
		trainMedic();
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

				// si el reparador esta seteado se deja ese SCV idle
				// si el reparador no esta seteado se manda a trabajar a todos los SCV
				bool condicion1 = ((reparador1 != NULL) && (trabajador->getID() != reparador1->getID())) || reparador1 == NULL;
				bool condicion2 = ((reparador2 != NULL) && (trabajador->getID() != reparador2->getID())) || reparador2 == NULL;

				if (condicion1 && condicion2){
					if(trabajador->isGatheringMinerals()) SCVgatheringCristal++;
					else if (trabajador->isGatheringGas())SCVgatheringGas++;
					else if (trabajador->isIdle()){ sendGatherCristal(trabajador); SCVgatheringCristal++;}
				}
			}	
		}

		if ((Broodwar->self()->completedUnitCount(*(new UnitType(Utilidades::ID_REFINERY)))>0) && (SCVgatheringCristal+SCVgatheringGas>10)){
			if (SCVgatheringGas < 4){
				for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
				{
					if (SCVgatheringGas<4){
						if ((*i)->getType().isWorker()){
							trabajador = (*i);

							// si el reparador esta seteado se deja ese SCV idle
							// si el reparador no esta seteado se manda a trabajar a todos los SCV
							bool condicion1 = ((reparador1 != NULL) && (trabajador->getID() != reparador1->getID())) || reparador1 == NULL;
							bool condicion2 = ((reparador2 != NULL) && (trabajador->getID() != reparador2->getID())) || reparador2 == NULL;

							if (condicion1 && condicion2){
								if(trabajador->isGatheringMinerals()) {
									SCVgatheringCristal--;
									SCVgatheringGas++;
									sendGatherGas(trabajador); 
								}
							}
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


	// ------------------------- construccion bunkers -------------------------

	if((Broodwar->self()->minerals() > 150)&& ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_BUNKER))))<goalCantUnidades[Utilidades::INDEX_GOAL_BUNKER])&&(buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_BUNKER);
		TilePosition *posB = NULL;

		if (grupoB1->getCantBunkers() == 0){
			// Obtiene la posicion del centro del chokepoint a defender y la convierte a una TilePosition (que se 
			// mide en build tiles)
			Position *p = analizador->obtenerCentroChokepoint();
			TilePosition *t = new TilePosition(p->x() / 32, p->y() / 32);
			posB = getTilePositionAviable(building, t);
			
			delete t;
			delete p;
		}
		else{
			// Ubica el nuevo bunker alrededor de la posicion del ultimo bunker creado
			TilePosition *t = new TilePosition(grupoB1->getUltimoBunkerCreado()->getTilePosition().x(), grupoB1->getUltimoBunkerCreado()->getTilePosition().y());
			posB = getTilePositionAviable(building, t);

			delete t;
		}

		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_BUNKER);

			delete posB;
		}
	}
	else if ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_BUNKER))) == goalCantUnidades[Utilidades::INDEX_GOAL_BUNKER]) && (goalCantUnidades[Utilidades::INDEX_GOAL_BUNKER] > 0)){
		if (reparador1 == NULL){
			// setea un SCV para que repare los bunkers
			for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
				if ((*i)->getType().isWorker()){
					reparador1 = (*i);
					Broodwar->printf("Reparador1 seteado...");
					//reparador1->rightClick(*(new Position(grupoB1->getUltimoBunkerCreado()->getPosition().x() - 10, grupoB1->getUltimoBunkerCreado()->getPosition().y() - 10)));
					break;
				}
			}
		}
		
		if (reparador2 == NULL){
			// setea un SCV para que repare los bunkers
			for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
				if (((*i)->getType().isWorker()) && ((*i)->getID() != reparador1->getID())){
					reparador2 = (*i);
					Broodwar->printf("Reparador2 seteado...");
					//reparador2->rightClick(*(new Position(grupoB1->getUltimoBunkerCreado()->getPosition().x() - 10, grupoB1->getUltimoBunkerCreado()->getPosition().y() - 10)));
					break;
				}
			}
		}
	}

	// ------------------------- Fin construccion bunkers -------------------------

	// ------------------------- construccion academia -------------------------

	if((Broodwar->self()->minerals() > 200)&& ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_ACADEMY))))<goalCantUnidades[Utilidades::INDEX_GOAL_ACADEMY])&&(buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_ACADEMY);
		TilePosition* posB = getTilePositionAviable(building);
		buildUnit(posB, Utilidades::ID_ACADEMY);
	}

	// ------------------------- Fin construccion academia -------------------------

	
	
	// ----------------------------------------------------------------------------
	//								Investigaciones
	// ----------------------------------------------------------------------------

	if (!researchDone[Utilidades::INDEX_GOAL_STIMPACK]){
		// stim_pack (se investiga en academia terran)
		if ((cantAcademias > 0) && (Broodwar->self()->minerals() > 100) && (Broodwar->self()->gas() > 100) && (goalResearch[Utilidades::INDEX_GOAL_STIMPACK] == 1)){
			Unit *u;

			u = getUnit(Utilidades::ID_ACADEMY);

			if (u != NULL){

				if (u->isCompleted() && (!u->isResearching()) && (!u->isUpgrading())){
					// Construccion de la academia finalizada, se puede investigar mejoras

					Broodwar->printf("Investigando mejora...");
					TechType *t = new TechType(TechTypes::Stim_Packs);
					u->research(*t);
					delete t;
					researchDone[Utilidades::INDEX_GOAL_STIMPACK] = true;
				}
			}
		}
	}

	if (!researchDone[Utilidades::INDEX_GOAL_U238]){
		// mejora de alcance para marines (se investiga en academia terran)
		if ((cantAcademias > 0) && (Broodwar->self()->minerals() > 150) && (Broodwar->self()->gas() > 150) && (goalResearch[Utilidades::INDEX_GOAL_U238] == 1)){
			Unit *u;

			u = getUnit(Utilidades::ID_ACADEMY);

			if (u != NULL){

				if ((u->isCompleted()) && (!u->isResearching()) && (!u->isUpgrading()) ){
					// Construccion de la academia finalizada, se puede investigar mejoras

					Broodwar->printf("Investigando mejora...");
					UpgradeType *t = new UpgradeType(UpgradeTypes::U_238_Shells);
					u->upgrade(*t);
					delete t;
					
					researchDone[Utilidades::INDEX_GOAL_U238] = true;
				}
			}
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
// el parámetro POS puede ser nulo, lo que indica que tiene que contruir la refineria en el geiser mas cercano
// si el parámetro tiene algun valor, se intenta construir sobre esa ubicación.
	Unit *trabajador;
	TilePosition *geyserPos = NULL;

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

			if ((firstBarrack != NULL) && (Broodwar->canMake(firstBarrack, Utilidades::ID_MARINE)) && (firstBarrack->getTrainingQueue().size() < 5)){
				firstBarrack->train(*(new UnitType(Utilidades::ID_MARINE)));
				break;
			}
		}
	}
}

void unit_Manager::trainMedic(){
	Unit* firstBarrack = NULL;
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		if ((*i)->getType().getID() == Utilidades::ID_BARRACK){
			firstBarrack = (*i);
			
			if ((firstBarrack != NULL) && (Broodwar->canMake(firstBarrack, Utilidades::ID_MEDIC)) && (firstBarrack->getTrainingQueue().size() < 5)){
				firstBarrack->train(*(new UnitType(Utilidades::ID_MEDIC)));
				break;
			}
		}
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

void unit_Manager::sendGatherGas(Unit* worker){
	
	if (worker != NULL){
	Unit* closestGeyser=NULL;
	//busca el mineral más cercano.
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
	
	if (unit->getType().getID() == Utilidades::ID_MARINE){
		if (cantMarines > 12) Otra->asignarUnidad(unit);
		else Easy->asignarUnidad(unit);
	}
	else if (unit->getType().getID() == Utilidades::ID_MEDIC){
		Otra->asignarUnidad(unit);
	}
	else if (unit->getType().getID() == Utilidades::ID_FIREBAT){
		Otra->asignarUnidad(unit);
	}
}


void unit_Manager::repararUnidad(Unit *u){

	if ((reparador1 != NULL) && (u != NULL)){
		// si el reparador esta seteado lo utiliza
		if ((u->getType().isMechanical()) || (u->getType().isBuilding())){
			reparador1->repair(u);
			reparador2->repair(u);
		}
		else{
			Broodwar->printf("No se puede reparar esa unidad");
		}
	}
	else{
		// sino utiliza cualquier SCV para reparar la unidad bajo ataque
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
			if ((*i)->getType().isWorker()){
				if ((u->getType().isMechanical()) || (u->getType().isBuilding())){
					(*i)->repair(u);
				}
				else{
					Broodwar->printf("No se puede reparar esa unidad");
				}
				break;
			}
		}
	}

}


void unit_Manager::verificarBunkers(){
	Unit *u = NULL; // variable temporal
	Unit *atacado = NULL; // puntero a la unidad bajo ataque

	for(std::set<Unit*>::const_iterator i=enemigo->getUnits().begin();i!=enemigo->getUnits().end();i++){
		u = (*i);

		if (u->isAttacking()){
			atacado = u->getOrderTarget();
			if ((atacado != NULL) && (atacado->getType().getID() == Utilidades::ID_BUNKER)){
				// un bunker esta siendo atacado, mando al SCV a repararlo
				//Broodwar->printf("Bunker bajo ataque");
				repararUnidad(atacado);
				Graficos::resaltarUnidad(atacado);

				Otra->atacar(u);

				//grupoB1->estrategia1(atacado);

				break;
			}

			atacado = u->getTarget();
			if ((atacado != NULL) && (atacado->getType().getID() == Utilidades::ID_BUNKER)){
				// un bunker esta siendo atacado, mando al SCV a repararlo
				//Broodwar->printf("Bunker bajo ataque");
				repararUnidad(atacado);
				Graficos::resaltarUnidad(atacado);

				Otra->atacar(u);
				//grupoB1->estrategia1(atacado);

				break;
			}
		}
	}

	/*if (researchDone[Utilidades::INDEX_GOAL_STIMPACK]){
		Easy->aplicarStim();
	}*/
}


/*void unit_Manager::ubicarBunker(Region *r, Chokepoint *c){
//std::set<TilePosition*> unit_Manager::ubicarBunker(Region *r, Chokepoint *c){
	// Calculo la posicion relativa del chokepoint respecto al centro de la region a defender
	// el tamaño de un bunker es 3 x 2 (ancho x alto)
	
	//int offsetX, offsetY;
	//TilePosition *t;
	int factorX = 1, factorY = 1;

	if (r->getCenter().x() < c->getCenter().x()){
		// El centro de la region esta mas a la izquierda que el centro del chokepoint
		//offsetX = -1;
		factorX = -1;
	}
	else{
		factorX = 1;
	}

	if (r->getCenter().y() < c->getCenter().y()){
		// El centro de la region esta mas arriba que el centro del chokepoint
		factorY = -1;
	}
	else{
		factorY = 1;
	}

	/*t = new TilePosition(c->getCenter().x() / 32 + offsetX, c->getCenter().y() / 32 + offsetY);

	if (Broodwar->unitsOnTile(t->x(), t->y()).size() == 0){
		seteado = true;
		return t;
	}
	else{
		delete t;
		t = new TilePosition(c->getCenter().x() / 32 + offsetX + 1, c->getCenter().y() / 32 + offsetY - 2);

		if (Broodwar->unitsOnTile(t->x(), t->y()).size() == 0){
			seteado = true;
			return t;
		}
		else{

			delete t;
			t = new TilePosition(c->getCenter().x() / 32 + offsetX - 1, c->getCenter().y() / 32 + offsetY + 2);

			if (Broodwar->unitsOnTile(t->x(), t->y()).size() == 0){
				seteado = true;
				return t;
			}
			else{
				seteado = false;
				return NULL;
			}
		}
	}*/


	/*Position bordeMasCercano;
	UnitType *marine = new UnitType(Utilidades::ID_MARINE);

	// Busca el extremo del chokepoint mas cercano al centro de la region
	if (c->getSides().first.getDistance(r->getCenter()) < c->getSides().second.getDistance(r->getCenter())){
		bordeMasCercano = c->getSides().first;
	}
	else{
		bordeMasCercano = c->getSides().second;
	}

	int offset; // representa la distancia desde el punto deseado hasta el borde del build tile del bunker
	int anchoBunker = 3 * 32;
	int altoBunker = 2 * 32;
	double raiz2 = sqrt(2.0);
	int rango = marine->seekRange(); // rango es la distancia a la que buscamos ubicar el bunker respecto del chokepoint

	// Cuentita sacada a mano, usando el teorema de pitagoras
	offset = rango / raiz2;

	//std::set<TilePosition*> result;
	Position *pos1, *pos2, *pos3;

	pos1 = new Position(c->getSides().first.x() +  factorX * (offset + anchoBunker), c->getSides().first.y() +  factorY * (offset + altoBunker));
	pos2 = new Position(c->getSides().second.x() +  factorX * (offset + anchoBunker), c->getSides().second.y() +  factorY * (offset + altoBunker));
	pos3 = new Position(c->getCenter().x() +  factorX * (offset + anchoBunker), c->getCenter().y() +  factorY * (offset + altoBunker));

	/*t1 = new TilePosition(pos1->x() / 32, pos1->y() / 32);
	t2 = new TilePosition(pos2->x() / 32, pos2->y() / 32);
	t3 = new TilePosition(pos3->x() / 32, pos3->y() / 32);*/

	/*posiciones.insert(t1);
	posiciones.insert(t2);
	posiciones.insert(t3);*/



	/*delete pos1;
	delete pos2;
	delete pos3;

	delete marine;
	//return result;

}*/



// Calcula el angulo que tiene el chokepoint
/*int unit_Manager::calcularAngulo(Region *r, Chokepoint *c){

}*/


void unit_Manager::nuevaUnidadConstruccion(Unit *u){

	if (u != NULL){
		unidadesEnConstruccion.push_front(u);

		if (u->getType().getID() == Utilidades::ID_BUNKER){
			Broodwar->printf("Agregue un bunker al grupoB1");
			grupoB1->agregarBunker(u);
		}
	}
}


Unit* unit_Manager::controlarFinalizacion(){
	// recorre la lista para ver si alguna unidad finalizo su construccion

	std::list<Unit*>::iterator It1;
	It1 = unidadesEnConstruccion.begin();

	while(It1 != unidadesEnConstruccion.end()){
		if ((*It1)->isCompleted()){
			Unit *u;

			u = (*It1);

			It1 = unidadesEnConstruccion.erase(It1);
			return (u);
		}
		else 
			It1++;
	}

	return NULL;
}

bool unit_Manager::construyendo(int ID){
	std::list<Unit*>::iterator It1;
	It1 = unidadesEnConstruccion.begin();

	while(It1 != unidadesEnConstruccion.end()){
		if ((*It1)->getType().getID() == ID) return true;
		else It1++;
	}

	return false;
}