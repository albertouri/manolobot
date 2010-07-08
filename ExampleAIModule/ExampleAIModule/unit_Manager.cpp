#include "unit_Manager.h"
#include "compania.h"
#include "Scout.h"
#include "Graficos.h"
#include <math.h>
#include "GrupoBunkers.h"

int goalLimiteGeiser = 1;
int goalLimiteSCV = 8;
int goalLimiteBarracas = 1;

int SCVgatheringMinerals= 0, SCVgatheringGas = 0;
int frameLatency;
int buildingSemaphore =0;

int goalCantUnidades[34] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 

// Compañias de unidades
compania* Easy;
compania* Otra;

Scout* magallanes;
Player* enemigo;

TilePosition *centroComando;	// mantiene la posicion del centro de comando
Unit *centroDeComando; //puntero a la posicion del centro;

// Grupos de bunkers
GrupoBunkers *grupoB1;

std::list<Unit*> unidadesEnConstruccion; // Lista de unidades que estan en construccion actualmente
Unit *ultimaFinalizada = NULL; // puntero a la ultima unidad finalizada, se calcula en cada frame con una llamada a controlarFinalizacion()

//int tiempoProxFinalizacion = 0; // mantiene el tiempo hasta la proxima finalizacion de la construccion o entrenamiento de una unidad para evitar ejecutar en todos los frames el metodo controlarFinalizacion
//int contProxFinalizacion = 0; // contador que se incrementa en cada frame, para controlar la finalizacion de una construccion o entrenamiento

unit_Manager::unit_Manager()
{
	//Broodwar->printf("necesito %d, tengo %d", goalCantUnidades[Utilidades::INDEX_GOAL_MARINE], cantUnidades[Utilidades::INDEX_GOAL_MARINE]);
	Easy = new compania(Colors::Red);
	//Otra = new compania(Colors::Yellow);

	magallanes = new Scout(getWorker());

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
	
	cantUnidades[Utilidades::INDEX_GOAL_SCV] = Broodwar->self()->completedUnitCount(*(new UnitType(Utilidades::ID_SCV)));;
	cantUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER] = Broodwar->self()->completedUnitCount(*(new UnitType(Utilidades::ID_COMMANDCENTER)));;

	reparador1 = NULL;
	reparador2 = NULL;


	// supongo que jugamos contra un solo enemigo
	// TODO: arreglar para que se puede jugar contra varios enemigos
	enemigo = Broodwar->enemy();
	grupoB1 = NULL;

}

void unit_Manager::executeActions(AnalizadorTerreno *analizador){

	// Crea un nuevo grupo de bunkers

	if (analizador->analisisListo()){
		if (grupoB1 == NULL)
			grupoB1 = new GrupoBunkers(analizador);
		else
			grupoB1->onFrame();
	}
	

	// -----------------------------------
	// si la unidad apuntada no esta reparando, setea el apuntador reparador a NULL, para que esa unidad vuelva a recolectar recursos

	if (reparador1 != NULL){
		Graficos::resaltarUnidad(reparador1, Colors::Yellow);
		if (!reparador1->isRepairing())
			reparador1 = NULL;
	}
	
	if (reparador2 != NULL){
		Graficos::resaltarUnidad(reparador2, Colors::Yellow);
		if (!reparador2->isRepairing())
			reparador2 = NULL;
	}

	// -----------------------------------

	Easy->onFrame();
	
	// manda al scout a explorar el mapa
	magallanes->explorar();
	//Otra->onFrame();

	// verifica si se termino de construir alguna unidad en este frame
	//ultimaFinalizada = controlarFinalizacion();

	// verifica daños en los bunkers 
	verificarBunkers();

	/*if (analizador->analisisListo()){

		TilePosition *t111 = NULL;
		
		t111 = analizador->calcularPrimerTile(analizador->regionInicial(), analizador->obtenerChokepoint(), 1);
		if (t111 != NULL){
			Graficos::dibujarCuadro(t111, 3, 2);
			Broodwar->drawLine(CoordinateType::Map, analizador->obtenerCentroChokepoint()->x(), analizador->obtenerCentroChokepoint()->y(), t111->x() * 32 + 16, t111->y() * 32 + 16, Colors::Yellow);
		}
	}*/

	//-----------------------------------------------------

	//construye 'goalLimiteSCV' de SCV, este valor deberia ser seteado por una llamada a setGoal
	//if((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_SCV))) < goalCantUnidades[Utilidades::INDEX_GOAL_SCV]) && (Broodwar->self()->minerals()>100) ) {
	if ((cantUnidades[Utilidades::INDEX_GOAL_SCV] < goalCantUnidades[Utilidades::INDEX_GOAL_SCV]) && (Broodwar->self()->minerals() > 100)) {
		trainWorker();
	}
	
	//metodo a corregir, solamente a fin de entrenar marines.
	//if ((Broodwar->self()->allUnitCount(*barraca))&&(Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_MARINE))) < goalCantUnidades[Utilidades::INDEX_GOAL_MARINE]) && (Broodwar->self()->minerals()>100) ) {
	if (cantUnidades[Utilidades::INDEX_GOAL_BARRACK] && (cantUnidades[Utilidades::INDEX_GOAL_MARINE] < goalCantUnidades[Utilidades::INDEX_GOAL_MARINE]) && (Broodwar->self()->minerals()>100)){
		trainMarine();
	}

	//if((Broodwar->self()->allUnitCount(*barraca))&&(Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_MEDIC))) < goalCantUnidades[Utilidades::INDEX_GOAL_MEDIC]) && (Broodwar->self()->minerals()>= 50) && (Broodwar->self()->gas()>= 25)) {
	if(cantUnidades[Utilidades::INDEX_GOAL_BARRACK] && (cantUnidades[Utilidades::INDEX_GOAL_MEDIC] < goalCantUnidades[Utilidades::INDEX_GOAL_MEDIC]) && (Broodwar->self()->minerals()>= 50) && (Broodwar->self()->gas()>= 25)) {
		trainMedic();
	}

	if(cantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP] && (cantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE] < goalCantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE]) && (Broodwar->self()->minerals()>= 150) && (Broodwar->self()->gas()>= 100)) {
		trainTankSiege();
	}



	if (frameLatency >= 150){
		int SCVgatheringCristal= 0, SCVgatheringGas = 0;
		Unit* trabajador;
		frameLatency=0;
		buildingSemaphore=0;
		
		
		//-- CODIGO PARA REPARAR UNIDADES 
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
			// si es una edificacion o es una unidad mecanica, verifica si esta dañada y la repara
			if ((((*i)->getType().isBuilding()) || ((*i)->getType().isMechanical())) && ((*i)->isCompleted()) && ((*i)->getType().maxHitPoints() > (*i)->getHitPoints())){
				repararUnidad(*i);
			}
			else if ((*i)->getType().isBuilding() && (!(*i)->isCompleted()) && (!(*i)->isBeingConstructed())){
				// si una unidad es una edificacion, no esta completada y no esta siendo construida por nadie (es decir quedo su construccion incompleta) manda a un SCV a finalizar su construccion
				finalizarConstruccion(*i);
			}
		}
		//-- TERMINA EL CODIGO PARA REPARAR UNIDADES
		

		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
			if ((*i)->getType().isWorker()){
				trabajador = (*i);

				// estas condiciones evitan que un SCV que esta reparando una unidad sea enviado a recolectar recursos
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

							// estas condiciones evitan que un SCV que esta reparando una unidad sea enviado a recolectar recursos
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


	//if((Broodwar->self()->minerals()>200) && ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_REFINERY)))) <goalCantUnidades[Utilidades::INDEX_GOAL_REFINERY])&&(buildingSemaphore == 0)){
	if((Broodwar->self()->minerals()>200) && (cantUnidades[Utilidades::INDEX_GOAL_REFINERY] < goalCantUnidades[Utilidades::INDEX_GOAL_REFINERY]) && (buildingSemaphore == 0)){
		TilePosition* pos = NULL;
		makeRefinery(pos);
	}



	//if((Broodwar->self()->minerals() > 200)&& ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_DEPOT))))<goalCantUnidades[Utilidades::INDEX_GOAL_DEPOT])&&(buildingSemaphore == 0)){
	if((Broodwar->self()->minerals() > 200) && (cantUnidades[Utilidades::INDEX_GOAL_DEPOT] < goalCantUnidades[Utilidades::INDEX_GOAL_DEPOT]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_DEPOT);
		TilePosition* posB = getTilePositionAviable(building);
			
		if (posB != NULL){
			
			buildUnit(posB, Utilidades::ID_DEPOT);
			delete posB;
		}
	}


	//if((Broodwar->self()->minerals() > 200) && ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_BARRACK)))) <goalCantUnidades[Utilidades::INDEX_GOAL_BARRACK])&&(buildingSemaphore == 0)){
	if((Broodwar->self()->minerals() > 200) && (cantUnidades[Utilidades::INDEX_GOAL_BARRACK] < goalCantUnidades[Utilidades::INDEX_GOAL_BARRACK]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_BARRACK);
		TilePosition* posB = getTilePositionAviable(building);
		
		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_BARRACK);
			delete posB;
		}
	}

	if((Broodwar->self()->minerals() > 200) && (cantUnidades[Utilidades::INDEX_GOAL_FACTORY] < goalCantUnidades[Utilidades::INDEX_GOAL_FACTORY]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_FACTORY);
		TilePosition* posB = getTilePositionAviable(building);
		
		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_FACTORY);
			delete posB;
		}
	}

	if((Broodwar->self()->minerals() > 50)&& (Broodwar->self()->gas() > 50) && (cantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP] < goalCantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP]) && (buildingSemaphore == 0)){
		buildUnitAddOn(Utilidades::ID_MACHINESHOP);
	}

	// ------------------------- construccion bunkers -------------------------

	//if((Broodwar->self()->minerals() > 150)&& ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_BUNKER))))<goalCantUnidades[Utilidades::INDEX_GOAL_BUNKER])&&(buildingSemaphore == 0)){
	if((grupoB1 != NULL) && (Broodwar->self()->minerals() > 150) && (cantUnidades[Utilidades::INDEX_GOAL_BUNKER] < goalCantUnidades[Utilidades::INDEX_GOAL_BUNKER]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_BUNKER);
		TilePosition *posB = NULL;

		posB = grupoB1->posicionNuevoBunker();

		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_BUNKER);

			delete posB;
		}
	}

	if((grupoB1 != NULL) && (cantUnidades[Utilidades::INDEX_GOAL_MISSILE_TURRET] < goalCantUnidades[Utilidades::INDEX_GOAL_MISSILE_TURRET]) && (Broodwar->self()->minerals() > 100) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_MISSILE_TURRET);
		TilePosition *posB = NULL;

		posB = grupoB1->posicionNuevaTorreta();

		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_MISSILE_TURRET);

			delete posB;
		}
	}


	// ------------------------- Fin construccion bunkers -------------------------

	// ------------------------- construccion academia -------------------------

	//if((Broodwar->self()->minerals() > 200)&& ((Broodwar->self()->allUnitCount(*(new UnitType(Utilidades::ID_ACADEMY))))<goalCantUnidades[Utilidades::INDEX_GOAL_ACADEMY])&&(buildingSemaphore == 0)){
	if((Broodwar->self()->minerals() > 200) && (cantUnidades[Utilidades::INDEX_GOAL_ACADEMY] < goalCantUnidades[Utilidades::INDEX_GOAL_ACADEMY]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_ACADEMY);
		TilePosition* posB = getTilePositionAviable(building);

		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_ACADEMY);
			delete posB;
		}
	}


	if((Broodwar->self()->minerals() > 125) && (cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY] < goalCantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_ENGINEERING_BAY);
		TilePosition* posB = getTilePositionAviable(building);

		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_ENGINEERING_BAY);
			delete posB;
		}
	}

	// ------------------------- Fin construccion academia -------------------------

	
	
	// ----------------------------------------------------------------------------
	//								Investigaciones
	// ----------------------------------------------------------------------------

	if (!researchDone[Utilidades::INDEX_GOAL_STIMPACK]){
		// stim_pack (se investiga en academia terran)
		if ((cantUnidades[Utilidades::INDEX_GOAL_ACADEMY] > 0) && (Broodwar->self()->minerals() > 100) && (Broodwar->self()->gas() > 100) && (goalResearch[Utilidades::INDEX_GOAL_STIMPACK] == 1)){
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
		if ((cantUnidades[Utilidades::INDEX_GOAL_ACADEMY] > 0) && (Broodwar->self()->minerals() > 150) && (Broodwar->self()->gas() > 150) && (goalResearch[Utilidades::INDEX_GOAL_U238] == 1)){
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

	if (!researchDone[Utilidades::INDEX_GOAL_TANK_SIEGE_MODE]){
		// mejora de alcance para marines (se investiga en academia terran)
		if ((cantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP] > 0) && (Broodwar->self()->minerals() > 150) && (Broodwar->self()->gas() > 150) && (goalResearch[Utilidades::INDEX_GOAL_TANK_SIEGE_MODE] == 1)){
			Unit *u;

			u = getUnit(Utilidades::ID_MACHINESHOP);

			if (u != NULL){

				if ((u->isCompleted()) && (!u->isResearching()) && (!u->isUpgrading()) ){
					// Construccion de la academia finalizada, se puede investigar mejoras

					Broodwar->printf("Investigando ampliación a Modo Asedio");
					TechType *t = new TechType(TechTypes::Tank_Siege_Mode);
					u->research(*t);
					delete t;
					
					researchDone[Utilidades::INDEX_GOAL_TANK_SIEGE_MODE] = true;
				}
			}
		}
	}

}


unit_Manager::~unit_Manager(void)
{
}


void unit_Manager::buildUnit(TilePosition *pos, int id){
	// posible error???

	Unit* trabajador;
	UnitType *tipo = new UnitType(id);
	if ((Broodwar->self()->minerals()>tipo->mineralPrice())&&(Broodwar->self()->gas()>=tipo->gasPrice())){
		trabajador = getWorker();
		
		if (trabajador!=NULL) {
			if ( Broodwar->canBuildHere(trabajador, /* *(new Position(*pos))*/ *pos, *tipo )){
				buildingSemaphore++;
				trabajador->build((*pos), *tipo);
			}
		}
	}

}

void unit_Manager::buildUnitAddOn(int id){
	
	Unit* trabajador;
	Unit* factory = NULL;
	UnitType *tipo = new UnitType(id);

	if ((Broodwar->self()->minerals()>tipo->mineralPrice())&&(Broodwar->self()->gas()>=tipo->gasPrice())){
		trabajador = getWorker();
	
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
			if ((*i)->getType().getID()==Utilidades::ID_FACTORY){
				factory = (*i);
				break;
			}
		}

		if (factory != NULL) {
			factory->buildAddon(*new UnitType(Utilidades::ID_MACHINESHOP));
		}
	}

}




void unit_Manager::makeRefinery(TilePosition *pos){
// el parámetro POS puede ser nulo, lo que indica que tiene que contruir la refineria en el geiser mas cercano
// si el parámetro tiene algun valor, se intenta construir sobre esa ubicación.
	Unit *trabajador;
	TilePosition *geyserPos = NULL;

	//if ((cantRefinerias < goalLimiteGeiser)&&(Broodwar->self()->minerals()>100)){
	if ((cantUnidades[Utilidades::INDEX_GOAL_REFINERY] < goalLimiteGeiser)&&(Broodwar->self()->minerals()>100)){
		trabajador = getWorker();
		Unit* closestGeiser=NULL;
		if (trabajador != NULL){
			if (pos == NULL){
				
				for(std::set<Unit*>::iterator i=Broodwar->getGeysers().begin();i!=Broodwar->getGeysers().end();i++){
					if (closestGeiser==NULL) closestGeiser=*i;
					if (trabajador->getDistance(*i)<trabajador->getDistance(closestGeiser)) closestGeiser=*i;
				}
				geyserPos = new TilePosition(closestGeiser->getTilePosition());
			}
			else{
				*geyserPos = (*pos);
			}
		
			if (Broodwar->self()->minerals() > 150) {
				trabajador->build((*geyserPos), *(new UnitType(Utilidades::ID_REFINERY)));
				//cantRefinerias++;
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
			//cantSCV++;		
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

void unit_Manager::trainTankSiege(){
	Unit* firstFactory = NULL;
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		if ((*i)->getType().getID() == Utilidades::ID_FACTORY){
			firstFactory = (*i);

			if ((firstFactory != NULL) && (Broodwar->canMake(firstFactory, Utilidades::ID_TANKSIEGE)) && (firstFactory->getTrainingQueue().size() < 5)){
				firstFactory->train(*(new UnitType(Utilidades::ID_TANKSIEGE)));
				break;
			}
		}
	}
}

void unit_Manager::sendGatherCristal(Unit* worker){
	
	if (worker != NULL){
		Unit* closestMineral=NULL;
		//busca el mineral más cercano.
		if (closestMineral == NULL){
			for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++){
				if (closestMineral == NULL || centroDeComando->getDistance(*m)<centroDeComando->getDistance(closestMineral))
					closestMineral = *m;
			}
		}

		if (closestMineral!=NULL) 
			worker->rightClick(closestMineral);	
	}
	else
		Broodwar->printf("el worker es null");

}

void unit_Manager::sendGatherGas(Unit* worker){
	
	if (worker != NULL){
		Unit* closestGeyser=NULL;
		//busca el mineral más cercano.
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
			if ((*i)->getType().isRefinery())
				closestGeyser = (*i);
		}
		
		if (closestGeyser!=NULL) 
			worker->rightClick(closestGeyser);
	} 
	else 
		Broodwar->printf("el worker es null");

}



void unit_Manager::resetBuildingSemaphore(){
	buildingSemaphore=0;
}

// Obtiene un TilePosition disponible en las cercanias del centro de comando
TilePosition* unit_Manager::getTilePositionAviable(UnitType* U){
	TilePosition* pos = NULL;
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
	TilePosition* pos = NULL;
	Unit* worker = getWorker();

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

		if ((grupoB1 != NULL) && (grupoB1->faltanMarines()))
			grupoB1->agregarUnidad(unit);
		else
			Easy->asignarUnidad(unit);


		/*if (Easy->countMarines() < 12) { Easy->asignarUnidad(unit); }
		else { Otra->asignarUnidad(unit); }*/
	}
	else if (unit->getType().getID() == Utilidades::ID_MEDIC){
		Easy->asignarUnidad(unit);
	}
	else if (unit->getType().getID() == Utilidades::ID_FIREBAT){
		Easy->asignarUnidad(unit);
	}
	else if (unit->getType().getID() == Utilidades::ID_TANKSIEGE){
		Easy->asignarUnidad(unit);
	}

}


void unit_Manager::repararUnidad(Unit *u){

	if ((u != NULL) && (u->getType().getID() != 7)){
		if ((reparador1 != NULL) && (!reparador1->isRepairing())){
			// si el reparador esta seteado lo utiliza
			if ((u->getType().isMechanical()) || (u->getType().isBuilding())){
				if ((u->exists())&& reparador1->exists()){
					reparador1->repair(u);
				}
			}
			else
				Broodwar->printf("No se puede reparar esa unidad");
		}
		else if ((reparador2 != NULL) && (!reparador2->isRepairing())){
			// si el reparador esta seteado lo utiliza
			if ((u->getType().isMechanical()) || (u->getType().isBuilding())){
				if ((u->exists())&& reparador2->exists()){
					reparador2->repair(u);
				}
			}
			else
				Broodwar->printf("No se puede reparar esa unidad");
		}
		else{
			if ((reparador1 == NULL) || (reparador2 == NULL)){
				// selecciona cualquier SCV que este recolectando recursos para reparar la unidad
				if (((u->getType().isMechanical()) || (u->getType().isBuilding())) && (u->exists())){
					for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
						if (((*i)->getType().isWorker()) && (!(*i)->isRepairing()) && (!(*i)->isConstructing())){
							if (reparador1 == NULL)
								reparador1 = (*i);
							else
								reparador2 = (*i);
							
							(*i)->repair(u);
							break;
						}
					}
				}
				else
					Broodwar->printf("No se puede reparar esa unidad");
			}
		}
	}
}


void unit_Manager::verificarBunkers(){
	Unit *u = NULL; // variable temporal
	Unit *atacado = NULL; // puntero a la unidad bajo ataque

	for(std::set<Unit*>::const_iterator i=enemigo->getUnits().begin();i!=enemigo->getUnits().end();i++){
		u = (*i);

		if ((u != NULL) && (u->exists()) && (u->isAttacking())){
			atacado = u->getOrderTarget();
			if ((atacado != NULL) && (atacado->getType().getID() == Utilidades::ID_BUNKER)){
				// un bunker esta siendo atacado, mando al SCV a repararlo
				repararUnidad(atacado);
				Easy->atacar(u);
				break;
			}

			atacado = u->getTarget();
			if ((atacado != NULL) && (atacado->getType().getID() == Utilidades::ID_BUNKER)){
				// un bunker esta siendo atacado, mando al SCV a repararlo
				repararUnidad(atacado);
				Easy->atacar(u);
				break;
			}
		}
	}
}


void unit_Manager::nuevaUnidadConstruccion(Unit *u){

	if (u != NULL){
		unidadesEnConstruccion.push_front(u);

		if ((grupoB1 != NULL) && (u->getType().getID() == Utilidades::ID_BUNKER) || (u->getType().getID() == Utilidades::ID_MISSILE_TURRET))
			grupoB1->agregarUnidad(u);
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
	if(Broodwar->self()== u->getPlayer()){

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
			case Utilidades::ID_FACTORY:
				cantUnidades[Utilidades::INDEX_GOAL_FACTORY]++;
				break;	
			case Utilidades::ID_MACHINESHOP:
				cantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP]++;
				break;
			case Utilidades::ID_TANKSIEGE:
				cantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE]++;
				asignarUnidadACompania(u);
				break;
			case Utilidades::ID_TANKSIEGE_SIEGEMODE:
				cantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE]++;
				break;
			case Utilidades::ID_ENGINEERING_BAY:
				cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY]++;
				break;
		}

	}

	resetBuildingSemaphore();
	nuevaUnidadConstruccion(u);
}

void unit_Manager::onUnitDestroy(Unit *u){
	
	if(Broodwar->self()== u->getPlayer()){

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
			case Utilidades::ID_FACTORY:
				cantUnidades[Utilidades::INDEX_GOAL_FACTORY]--;
				break;
			case Utilidades::ID_MACHINESHOP:
				cantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP]--;
				break;
			case Utilidades::ID_TANKSIEGE:
				cantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE]--;
				break;
			case Utilidades::ID_TANKSIEGE_SIEGEMODE:
				cantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE]--;
				break;
			case Utilidades::ID_ENGINEERING_BAY:
				cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY]--;
				break;
		}

	}
}


void unit_Manager::finalizarConstruccion(Unit *u){

	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
		if (((*i)->getType().isWorker()) && (!(*i)->isRepairing()) && (!(*i)->isConstructing())){
			(*i)->rightClick(u);
			break;
		}
	}
}