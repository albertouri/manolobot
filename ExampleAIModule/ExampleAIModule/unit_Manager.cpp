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
	Easy = new compania(Colors::Red);
	Otra = new compania(Colors::Yellow);

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

	for (int x = 0; x < 34; x++){
		this->cantUnidades[x] = 0;
	}
	
	cantUnidades[Utilidades::INDEX_GOAL_SCV] = 4;
	cantUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER] = 1;

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

	Easy->onFrame();
	Otra->onFrame();

	// verifica si se termino de construir alguna unidad en este frame
	ultimaFinalizada = controlarFinalizacion();

	/*if (ultimaFinalizada != NULL){
		if (ultimaFinalizada->getType().getID() == Utilidades::ID_MARINE) cantMarines++;
		else if (ultimaFinalizada->getType().getID() == Utilidades::ID_MEDIC) cantMedics++;
		else if (ultimaFinalizada->getType().getID() == Utilidades::ID_FIREBAT) cantFirebats++;
	}*/

	// verifica daños en los bunkers
	verificarBunkers();

	if (analizador->analisisListo()){
		TilePosition *t111 = NULL;
		
		t111 = analizador->calcularPrimerTile(analizador->regionInicial(), analizador->obtenerChokepoint());
		if (t111 != NULL){
			Graficos::dibujarCuadro(t111, 3, 2);
			Broodwar->drawLine(CoordinateType::Map, analizador->obtenerCentroChokepoint()->x(), analizador->obtenerCentroChokepoint()->y(), t111->x() * 32 + 16, t111->y() * 32 + 16, Colors::Yellow);
		}
	}


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
			
			//Position *p = analizador->obtenerCentroChokepoint();
			//TilePosition *t = new TilePosition(p->x() / 32, p->y() / 32);
			posB = analizador->calcularPrimerTile(analizador->regionInicial(), analizador->obtenerChokepoint());
			//posB = getTilePositionAviable(building, t);
			
			//delete t;
			//delete p;
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
	/*else if ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_BUNKER))) == goalCantUnidades[Utilidades::INDEX_GOAL_BUNKER]) && (goalCantUnidades[Utilidades::INDEX_GOAL_BUNKER] > 0)){
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
	}*/

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

					Broodwar->printf("Investigando mejora stim pack en academia...");
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

					Broodwar->printf("Investigando mejora U238 en academia...");
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
		if (cantUnidades[Utilidades::INDEX_GOAL_MARINE] > 12) Otra->asignarUnidad(unit);
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
			if (u->exists()){
				reparador1->repair(u);
				reparador2->repair(u);
			}
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
					if (u->exists()){
						(*i)->repair(u);
					}
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

				//Otra->atacar(u);
				Easy->atacar(u);

				//grupoB1->estrategia1(atacado);

				break;
			}

			atacado = u->getTarget();
			if ((atacado != NULL) && (atacado->getType().getID() == Utilidades::ID_BUNKER)){
				// un bunker esta siendo atacado, mando al SCV a repararlo
				//Broodwar->printf("Bunker bajo ataque");
				repararUnidad(atacado);
				Graficos::resaltarUnidad(atacado);

				//Otra->atacar(u);
				Easy->atacar(u);
				//grupoB1->estrategia1(atacado);

				break;
			}
		}
	}

	/*if (researchDone[Utilidades::INDEX_GOAL_STIMPACK]){
		Easy->aplicarStim();
	}*/
}


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

	if (unidadesEnConstruccion.size() > 0){
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

// mueve las unidades que estan ubicadas en el build tile pasado como parametro a otro build tile para poder construir ahi
void unit_Manager::moverUnidades(TilePosition *t){

	if (Broodwar->unitsOnTile(t->x(), t->y()).size() > 0){
		int x = 1, y = 0;
		std::set<Unit*>::const_iterator It1;
		Position *pos = NULL;
		
		It1 = Broodwar->unitsOnTile(t->x(), t->y()).begin();

		// los parametros de isWalkable son coordenadas en walk tiles
		while (!(Broodwar->isWalkable((t->x() - x) * 4, (t->y() - y) * 4)) && (Broodwar->mapWidth() < (t->x() - x)) && (Broodwar->mapHeight() < (t->y() - y))){
			x ++;
			y ++;
		}

		if ((Broodwar->mapWidth() < (t->x() - x)) && (Broodwar->mapHeight() < (t->y() - y))){

			pos = new Position((t->x() - x) * 32, (t->y() - y) * 32);

			while (It1 != Broodwar->unitsOnTile(t->x(), t->y()).end()){
				// si no es un edificio lo mueve
				if (!(*It1)->getType().isBuilding()){
					(*It1)->rightClick(*pos);
				}
			}

			delete pos;

		}

	}
}

void unit_Manager::onUnitCreate(Unit *u){

	switch (u->getType().getID()){
		case Utilidades::ID_ACADEMY:
			cantUnidades[Utilidades::INDEX_GOAL_ACADEMY]++;
			break;
		case Utilidades::ID_BARRACK:
			cantUnidades[Utilidades::INDEX_GOAL_BARRACK]++;
			break;
		case Utilidades::ID_BUNKER:
			cantUnidades[Utilidades::INDEX_GOAL_BUNKER]++;
			break;
		case Utilidades::ID_DEPOT:
			cantUnidades[Utilidades::INDEX_GOAL_DEPOT]++;
			break;
		case Utilidades::ID_FIREBAT:
			cantUnidades[Utilidades::INDEX_GOAL_FIREBAT]++;
			asignarUnidadACompania(u);
			break;
		case Utilidades::ID_MARINE:
			cantUnidades[Utilidades::INDEX_GOAL_MARINE]++;
			asignarUnidadACompania(u);
			break;
		case Utilidades::ID_MEDIC:
			cantUnidades[Utilidades::INDEX_GOAL_MEDIC]++;
			asignarUnidadACompania(u);
			break;
		case Utilidades::ID_REFINERY:
			cantUnidades[Utilidades::INDEX_GOAL_REFINERY]++;
			break;
		case Utilidades::ID_SCV:
			cantUnidades[Utilidades::INDEX_GOAL_SCV]++;
			break;
	}

	resetBuildingSemaphore();
	nuevaUnidadConstruccion(u);
}

void unit_Manager::onUnitDestroy(Unit *u){
	switch (u->getType().getID()){
		case Utilidades::ID_ACADEMY:
			cantUnidades[Utilidades::INDEX_GOAL_ACADEMY]--;
			break;
		case Utilidades::ID_BARRACK:
			cantUnidades[Utilidades::INDEX_GOAL_BARRACK]--;
			break;
		case Utilidades::ID_BUNKER:
			cantUnidades[Utilidades::INDEX_GOAL_BUNKER]--;
			break;
		case Utilidades::ID_DEPOT:
			cantUnidades[Utilidades::INDEX_GOAL_DEPOT]--;
			break;
		case Utilidades::ID_FIREBAT:
			cantUnidades[Utilidades::INDEX_GOAL_FIREBAT]--;
			break;
		case Utilidades::ID_MARINE:
			cantUnidades[Utilidades::INDEX_GOAL_MARINE]--;
			break;
		case Utilidades::ID_MEDIC:
			cantUnidades[Utilidades::INDEX_GOAL_MEDIC]--;
			break;
		case Utilidades::ID_REFINERY:
			cantUnidades[Utilidades::INDEX_GOAL_REFINERY]--;
			break;
		case Utilidades::ID_SCV:
			cantUnidades[Utilidades::INDEX_GOAL_SCV]--;
			break;
	}
}