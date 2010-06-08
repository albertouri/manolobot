#include "unit_Manager.h"
#include <BWAPI.h>
#include "compania.h"
#include "Scout.h"

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

compania* Easy;
Scout* magallanes;
Player* enemigo;

TilePosition *centroComando;	// mantiene la posicion del centro de comando
Unit *centroDeComando; //puntero a la posicion del centro;
UnitType *barraca; // puntero a la unidad que actualmente esta construyendo algo


bool seteado = false;


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

	for (int x = 0; x < Utilidades::maxResearch; x++){
		goalResearch[x] = 0;
		researchDone[x] = false;
	}

	reparador = NULL;


	// supongo que jugamos contra un solo enemigo
	// TODO: arreglar para que se puede jugar contra varios enemigos
	enemigo = Broodwar->enemy();

}

void unit_Manager::executeActions(AnalizadorTerreno *analizador){
	
	// manda al scout a explorar el mapa
	magallanes->explorar();

	resaltarUnidad(reparador);

	// verifica daños en los bunkers
	verificarBunkers();

	/*if (seteado){
		dibujarCuadro(ubicarBunker(analizador->regionInicial(), analizador->obtenerChokepoint()), 3, 2);
	}*/

	//construye 'goalLimiteSCV' de SCV, este valor deberia ser seteado por una llamada a setGoal
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

				// si el reparador esta seteado se deja ese SCV idle
				// si el reparador no esta seteado se manda a trabajar a todos los SCV
				bool condicion = ((reparador != NULL) && (trabajador->getID() != reparador->getID())) || reparador == NULL;

				if (condicion){
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
							bool condicion = ((reparador != NULL) && (trabajador->getID() != reparador->getID())) || reparador == NULL;

							if (condicion){
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


	// construye bunker
	if((Broodwar->self()->minerals() > 150)&& ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_BUNKER))))<goalCantUnidades[Utilidades::INDEX_GOAL_BUNKER])&&(buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_BUNKER);

		// Obtiene la posicion del centro del chokepoint a defender y la convierte a una TilePosition (que se 
		// mide en build tiles)
		/*Position *p = analizador->obtenerCentroChokepoint();
		TilePosition *t = new TilePosition(p->x() / 32, p->y() / 32);
		TilePosition* posB = getTilePositionAviable(building, t);

		delete t;*/

		TilePosition *posB = ubicarBunker(analizador->regionInicial(), analizador->obtenerChokepoint());

		//TilePosition* posB = getTilePositionAviable(building);
		if (posB != NULL)
			buildUnit(posB, Utilidades::ID_BUNKER);
	}


	// construye academia
	if((Broodwar->self()->minerals() > 200)&& ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_ACADEMY))))<goalCantUnidades[Utilidades::INDEX_GOAL_ACADEMY])&&(buildingSemaphore == 0)){
		
		if (reparador == NULL){
			// setea un SCV para que repare los bunkers
			for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
				if ((*i)->getType().isWorker()){
					reparador = (*i);
					Broodwar->printf("Reparador seteado...");
					break;
				}
			}
		}

		UnitType* building = new UnitType(Utilidades::ID_ACADEMY);
		TilePosition* posB = getTilePositionAviable(building);
		buildUnit(posB, Utilidades::ID_ACADEMY);
	}

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
	Easy->asignarUnidad(unit);
}


void unit_Manager::repararUnidad(Unit *u){

	if ((reparador != NULL) && (u != NULL)){
		// si el reparador esta seteado lo utiliza
		if ((u->getType().isMechanical()) || (u->getType().isBuilding())){
			reparador->repair(u);
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
				Broodwar->printf("Bunker bajo ataque");
				repararUnidad(atacado);
				resaltarUnidad(atacado);
				break;
			}

			atacado = u->getTarget();
			if ((atacado != NULL) && (atacado->getType().getID() == Utilidades::ID_BUNKER)){
				// un bunker esta siendo atacado, mando al SCV a repararlo
				Broodwar->printf("Bunker bajo ataque");
				repararUnidad(atacado);
				resaltarUnidad(atacado);
				break;
			}
		}
	}
}


void unit_Manager::resaltarUnidad(Unit *u){

	if (u != NULL){
		int alto = u->getType().tileHeight() * 32;
		int ancho = u->getType().tileWidth() * 32;
		Broodwar->drawBox(CoordinateType::Map, reparador->getTilePosition().x() * 32 + 24, reparador->getTilePosition().y() * 32 + 8, reparador->getTilePosition().x() * 32 + ancho + 24, reparador->getTilePosition().y() * 32 + alto + 8, Colors::Blue, false);
	}

}

TilePosition* unit_Manager::ubicarBunker(Region *r, Chokepoint *c){
	// Calculo la posicion relativa del chokepoint respecto al centro de la region a defender
	// el tamaño de un bunker es 3 x 2 (ancho x alto)

	int offsetX, offsetY;
	TilePosition *t;

	if (r->getCenter().x() < c->getCenter().x()){
		// El centro de la region esta mas a la izquierda que el centro del chokepoint
		offsetX = -1;
	}
	else{
		offsetX = 1;
	}

	if (r->getCenter().y() < c->getCenter().y()){
		// El centro de la region esta mas arriba que el centro del chokepoint
		offsetY = -2;
	}
	else{
		offsetY = 2;
	}

	t = new TilePosition(c->getCenter().x() / 32 + offsetX, c->getCenter().y() / 32 + offsetY);

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
	}
}

void unit_Manager::dibujarCuadro(TilePosition* p, int tilesAncho, int tilesAlto){
	Broodwar->drawBox(CoordinateType::Map, p->x() * 32, p->y() * 32, p->x() * 32 + tilesAncho * 32, p->y() * 32 + tilesAlto * 32, Colors::Orange, false);
}