#include "unit_Manager.h"
#include "GrupoAntiaereo.h"

GrupoAntiaereo *anti = NULL;
int goalLimiteGeiser = 2;

int turnoAsignacionMarines = 0;
int turnoAsignacionMedics = 0;
int turnoAsignacionTanks = 0;
int turnoAsignacionGoliaths = 0;

/*int goalLimiteSCV = 8;
int goalLimiteBarracas = 1;*/



unit_Manager::unit_Manager(AnalizadorTerreno *analizador)
{
	SCVgatheringMinerals = 0;
	SCVgatheringGas = 0;
	//ultimaFinalizada = NULL;
	buildingSemaphore = 0;

	frameLatency = 0;
	latenciaScout = 0;

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
		goalCantUnidades[x] = 0;
		this->cantUnidades[x] = 0;
	}
	
	cantUnidades[Utilidades::INDEX_GOAL_SCV] = Broodwar->self()->completedUnitCount(*(new UnitType(Utilidades::ID_SCV)));;
	cantUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER] = Broodwar->self()->completedUnitCount(*(new UnitType(Utilidades::ID_COMMANDCENTER)));;

	reparador1 = NULL;
	reparador2 = NULL;

	// supongo que jugamos contra un solo enemigo
	// TODO: arreglar para que se puede jugar contra varios enemigos
	enemigo = Broodwar->enemy();
	

	//-- inicializa los objetos creados por nosotros

	Easy = new compania(analizador, Colors::Red);
	Fox = new CompaniaDefensiva(Colors::Yellow); // esta compañia se encargara de atacar a los fantasmas que ataquen la base
	//Fox = NULL;
	//	Charlie = new compania(analizador, Colors::Yellow);
	//	Charlie->setComportanmientoEsperando();

	//magallanes = new Scout(getWorker()); // revisar como genera las posiciones a partir de la 4ta posicion a explorar pq se rompe
	magallanes = NULL;
	grupoB1 = NULL;
	grupoB2 = NULL;
	ct = NULL;
	grf = NULL;

	//-- fin de inicializacion de objetos


	primerConstruccionDescubierta = true;
	baseEnemiga = NULL;
	regionBaseEnemiga = NULL;
	analisisListo = false;

	estadoActual = 0;
}


void unit_Manager::executeActions(){

	if (analizador->analisisListo()){

		if (!analisisListo)
			analisisListo = true;

		if ((ct != NULL) && (estadoActual == 5))
			ct->onFrame();

		if (grf == NULL)
			grf = new Grafo(BWTA::getRegions().size());
		else{
			if (magallanes == NULL)
				magallanes = new Scout(getWorker(), grf);
			else
				magallanes->explorar(); // manda al scout a explorar el mapa
		}

		if (grupoB1 == NULL)
			grupoB1 = new GrupoBunkers(analizador, analizador->obtenerChokepoint() ,analizador->regionInicial());
		else
			grupoB1->onFrame();


		if (grupoB2 != NULL)
			grupoB2->onFrame();
		
		//-- BUSCA LA REGION PARA EL SEGUNDO GRUPO DE BUNKERS
		if (analizador->analisisListo() && (grupoB2 == NULL) && (estadoActual == 5) && (regionBaseEnemiga != NULL)){
			std::set<Region*>::const_iterator It = analizador->regionInicial()->getReachableRegions().begin();
			Region *temp = NULL;

			while (It != analizador->regionInicial()->getReachableRegions().end()){
				if (((*It) != analizador->regionInicial()) && ((*It)->getChokepoints().size() > 1) && ((*It)->getBaseLocations().size() > 0) && (!((*(*It)->getBaseLocations().begin())->isMineralOnly()))){
					if (temp == NULL)
						temp = (*It);
					else {
						if (((*It)->getCenter().getDistance(analizador->regionInicial()->getCenter()) < temp->getCenter().getDistance(analizador->regionInicial()->getCenter())) && ((*It) != analizador->regionInicial()))
							temp = (*It);
					}
				}
				It++;
			}

			if (temp == NULL)
				Broodwar->printf("No hay lugar para una nueva base...");
			else{
				TilePosition* posB = new TilePosition((*temp->getBaseLocations().begin())->getTilePosition().x(), (*temp->getBaseLocations().begin())->getTilePosition().y());
				grupoB2 = new GrupoBunkers(analizador, temp, regionBaseEnemiga);
				delete posB;
			}
		}

		//-- FIN BUSQUEDA DE REGION
		if (anti == NULL)
			anti = new GrupoAntiaereo(analizador->regionInicial());
		else
			anti->onFrame();


		if (Fox != NULL)
			Fox->onFrame();
	}

	if (Broodwar->getFrameCount() % 50 == 0)
		buscarUnidadesOcultas();
	
	// ---------------------------------------------------------------------------
	/*	si la unidad apuntada no esta reparando, setea el apuntador reparador a NULL, para 
		que esa unidad vuelva a recolectar recursos
	*/
	if ((reparador1 != NULL)&& (reparador1->exists())){
		Graficos::resaltarUnidad(reparador1, Colors::Yellow);
		if (!reparador1->isRepairing())
			reparador1 = NULL;
	}
	
	if ((reparador2 != NULL)&& (reparador2->exists())){
		Graficos::resaltarUnidad(reparador2, Colors::Yellow);
		if (!reparador2->isRepairing())
			reparador2 = NULL;
	}

	// ---------------------------------------------------------------------------

	if (Easy != NULL){
		Easy->onFrame();
	}
	else
		Broodwar->printf("ERROR: Easy es NULL");


	// ---------------------------------------------------------------------------
	//--						CODIGO DE REPARACION DE UNIDADES

	if (Broodwar->getFrameCount() % 27 == 0){
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
			if ((*i)->exists()){
				// si es una edificacion o es una unidad mecanica, verifica si esta dañada y la repara
				if ((Broodwar->self()->minerals() > 70) && (((*i)->getType().isBuilding()) || ((*i)->getType().isMechanical())) && ((*i)->isCompleted()) && ((*i)->getType().maxHitPoints() > (*i)->getHitPoints())){
					// evita que reparen el scout, de otro modo los reparadores seguiran al scout a lo largo de toda la exploracion
					if ((magallanes->getExplorador() != NULL) && (magallanes->exists()) && (magallanes->getExplorador()->getID() != (*i)->getID()))
						repararUnidad(*i);
				}
				else if ((*i)->getType().isBuilding() && (!(*i)->isCompleted()) && (!(*i)->isBeingConstructed())){
					// si una unidad es una edificacion, no esta completada y no esta siendo construida por nadie (es decir quedo su construccion incompleta) manda a un SCV a finalizar su construccion
					finalizarConstruccion(*i);
				}
			}
		}
	}

	// ---------------------------------------------------------------------------
	//--					CODIGO DE GESTION DE RECOLECCION DE RECURSOS

	if (frameLatency >= 100){
		int SCVgatheringCristal= 0, SCVgatheringGas = 0;
		Unit* trabajador;
		frameLatency=0;
		buildingSemaphore=0;
		int limiteSCVbuscandoGas = 4;
		
		if (Broodwar->self()->gas() > Broodwar->self()->minerals()) limiteSCVbuscandoGas =1;


		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
			if ((*i)->getType().isWorker()){
				trabajador = (*i);
				
				if(Broodwar->self()->completedUnitCount(Utilidades::ID_SCV)>5){
					if(trabajador->isGatheringGas()){
						SCVgatheringGas++;
					}

					if ((Broodwar->self()->completedUnitCount(Utilidades::ID_REFINERY) > 0) && (SCVgatheringGas <limiteSCVbuscandoGas)){
						if((trabajador->isIdle())||(trabajador->isGatheringMinerals())){
							SCVgatheringGas++;
							sendGatherGas(trabajador);
						}
					}
					else {
						if(trabajador->isIdle()||((SCVgatheringGas>limiteSCVbuscandoGas)&&(trabajador->isGatheringGas()))){
							SCVgatheringCristal++;
							sendGatherCristal(trabajador);
						}
					}
				}
				else{
					if((trabajador->isGatheringGas())||(trabajador->isIdle()))
						sendGatherCristal(trabajador);
				}
			}
		}
	}
	else{
		frameLatency++;
	}

	// ---------------------------------------------------------------------------
	//--						CODIGO PARA CONSTRUCCION DE UNIDADES

	if (Broodwar->getFrameCount() % 24 == 0)
		ejecutarConstrucciones();
	
	//--						FIN CODIGO PARA CONSTRUCCION DE UNIDADES
	// ---------------------------------------------------------------------------
}


unit_Manager::~unit_Manager(void)
{
}


void unit_Manager::ejecutarConstrucciones(){

	//-- COMMAND CENTER
	if ((cantUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER] < goalCantUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER]) && (Broodwar->self()->minerals() > 400) && (buildingSemaphore == 0)){

		if (cantUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER] == 0){
			UnitType* building = new UnitType(Utilidades::ID_COMMANDCENTER);
			TilePosition* posB = NULL;

			if (analizador->analisisListo())
				posB = new TilePosition((*analizador->regionInicial()->getBaseLocations().begin())->getTilePosition().x(), (*analizador->regionInicial()->getBaseLocations().begin())->getTilePosition().y());
			
			if (posB != NULL){
				buildUnit(posB, Utilidades::ID_COMMANDCENTER);
				delete posB;
			}
			delete building;
		}
		else{
			// Verifica donde esta el command center y construye en otra posicion
			if (analizador->analisisListo()){
				Unit *cc = getUnit(Utilidades::ID_COMMANDCENTER);
				if (cc != NULL){
					if ((cc->getTilePosition().x() != (*analizador->regionInicial()->getBaseLocations().begin())->getTilePosition().x()) || (cc->getTilePosition().y() != (*analizador->regionInicial()->getBaseLocations().begin())->getTilePosition().y())){
						UnitType* building = new UnitType(Utilidades::ID_COMMANDCENTER);
						TilePosition* posB = NULL;

						if (analizador->analisisListo())
							posB = new TilePosition((*analizador->regionInicial()->getBaseLocations().begin())->getTilePosition().x(), (*analizador->regionInicial()->getBaseLocations().begin())->getTilePosition().y());
						
						if (posB != NULL){
							buildUnit(posB, Utilidades::ID_COMMANDCENTER);
							delete posB;
						}
						delete building;
					}
					else{
						std::set<Region*>::const_iterator It = analizador->regionInicial()->getReachableRegions().begin();
						Region *temp = NULL;

						while (It != analizador->regionInicial()->getReachableRegions().end()){
							if (((*It) != analizador->regionInicial()) && ((*It)->getChokepoints().size() > 1) && ((*It)->getBaseLocations().size() > 0) && (!((*(*It)->getBaseLocations().begin())->isMineralOnly()))){
								if (temp == NULL)
									temp = (*It);
								else {
									if (((*It)->getCenter().getDistance(analizador->regionInicial()->getCenter()) < temp->getCenter().getDistance(analizador->regionInicial()->getCenter())) && ((*It) != analizador->regionInicial()))
										temp = (*It);
								}
							}
							It++;
						}

						if (temp == NULL)
							Broodwar->printf("No hay lugar para una nueva base...");
						else{
							UnitType* building = new UnitType(Utilidades::ID_COMMANDCENTER);
							TilePosition* posB = new TilePosition((*temp->getBaseLocations().begin())->getTilePosition().x(), (*temp->getBaseLocations().begin())->getTilePosition().y());

							if (posB != NULL){
								buildUnit(posB, Utilidades::ID_COMMANDCENTER);
								delete posB;
							}
							delete building;
						}
					}
				}
			}
		}
	}

	//-- COMSAT STATION (ADD-ON DE COMMAND CENTER)
	if((cantUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER] > 0) && (Broodwar->self()->minerals() > 50) && (Broodwar->self()->gas() > 50) && (cantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION] < goalCantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION]) /*&& (buildingSemaphore == 0)*/ && (cantUnidades[Utilidades::INDEX_GOAL_ACADEMY] > 0)){
		buildUnitAddOn(Utilidades::ID_COMSAT_STATION);
	}

	//-- SCV (construye 'goalLimiteSCV' de SCV), este valor deberia ser seteado por una llamada a setGoal
	if ((cantUnidades[Utilidades::INDEX_GOAL_SCV] < goalCantUnidades[Utilidades::INDEX_GOAL_SCV]) && (Broodwar->self()->minerals() > 100)) {
		trainWorker();
	}
	
	//-- MARINES (metodo a corregir, solamente a fin de entrenar marines)
	if (cantUnidades[Utilidades::INDEX_GOAL_BARRACK] && (cantUnidades[Utilidades::INDEX_GOAL_MARINE] < goalCantUnidades[Utilidades::INDEX_GOAL_MARINE]) && (Broodwar->self()->minerals()>100)){
		trainMarine();
	}

	//-- MEDICS
	if(cantUnidades[Utilidades::INDEX_GOAL_BARRACK] && (cantUnidades[Utilidades::INDEX_GOAL_MEDIC] < goalCantUnidades[Utilidades::INDEX_GOAL_MEDIC]) && (Broodwar->self()->minerals()>= 50) && (Broodwar->self()->gas()>= 25)) {
		trainMedic();
	}

	//-- GHOSTS
	if(cantUnidades[Utilidades::INDEX_GOAL_BARRACK] && (Fox != NULL) && (Fox->faltanGhosts()) && (Broodwar->self()->minerals()>= 25) && (Broodwar->self()->gas()>= 75)) {
		trainGhost();
	}

	//-- GOLIATHS
	if(cantUnidades[Utilidades::INDEX_GOAL_ARMORY] && (cantUnidades[Utilidades::INDEX_GOAL_GOLIATH] < goalCantUnidades[Utilidades::INDEX_GOAL_GOLIATH]) && (Broodwar->self()->minerals()>= 100) && (Broodwar->self()->gas()>= 50)) {
		trainGoliath();
	}

	//-- SIEGE TANKS
	if(cantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP] && (cantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE] < goalCantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE]) && (Broodwar->self()->minerals()>= 150) && (Broodwar->self()->gas()>= 100)) {
		trainTankSiege();
	}

	//-- VULTURES
	if(cantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP] && (cantUnidades[Utilidades::INDEX_GOAL_VULTURE] < goalCantUnidades[Utilidades::INDEX_GOAL_VULTURE]) && (Broodwar->self()->minerals()>= 150) && (Broodwar->self()->gas()>= 100)) {	
		trainVulture();
	}

	//-- REFINERY
	if((Broodwar->self()->minerals()>200) && (cantUnidades[Utilidades::INDEX_GOAL_REFINERY] < goalCantUnidades[Utilidades::INDEX_GOAL_REFINERY]) && (buildingSemaphore == 0)){
		if (cantUnidades[Utilidades::INDEX_GOAL_REFINERY] == 0){
			TilePosition* pos = NULL;
			makeRefinery(pos);
		}
		else{
			Unit* ref = getUnit(Utilidades::ID_REFINERY);
			if (analizador->analisisListo() && (ref != NULL) && (((*BWTA::getStartLocation(Broodwar->self())->getGeysers().begin())->getTilePosition().x() != ref->getTilePosition().x()) || ((*BWTA::getStartLocation(Broodwar->self())->getGeysers().begin())->getTilePosition().y() != ref->getTilePosition().y()))){
				TilePosition* pos = new TilePosition((*BWTA::getStartLocation(Broodwar->self())->getGeysers().begin())->getTilePosition().x(), (*BWTA::getStartLocation(Broodwar->self())->getGeysers().begin())->getTilePosition().y());
				makeRefinery(pos);
				delete pos;
			}
			else{
				std::set<Region*>::const_iterator It = analizador->regionInicial()->getReachableRegions().begin();
				Region *temp = NULL;

				while (It != analizador->regionInicial()->getReachableRegions().end()){
					if (((*It) != analizador->regionInicial()) && ((*It)->getChokepoints().size() > 1) && ((*It)->getBaseLocations().size() > 0) && (!((*(*It)->getBaseLocations().begin())->isMineralOnly()))){
						if (temp == NULL)
							temp = (*It);
						else {
							if (((*It)->getCenter().getDistance(analizador->regionInicial()->getCenter()) < temp->getCenter().getDistance(analizador->regionInicial()->getCenter())) && ((*It) != analizador->regionInicial()))
								temp = (*It);
						}
					}
					It++;
				}

				if (temp == NULL)
					Broodwar->printf("No hay lugar para una nueva base...");
				else{
					TilePosition* pos = new TilePosition((*(*temp->getBaseLocations().begin())->getGeysers().begin())->getTilePosition().x(), (*(*temp->getBaseLocations().begin())->getGeysers().begin())->getTilePosition().y());
					makeRefinery(pos);
					delete pos;
				}
			}
		}
	}


	//-- SUPPLY DEPOT
	if((Broodwar->self()->minerals() > 200) && (cantUnidades[Utilidades::INDEX_GOAL_DEPOT] < goalCantUnidades[Utilidades::INDEX_GOAL_DEPOT]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_DEPOT);
		TilePosition* posB = getTilePositionAviable(building);
		TilePosition* posicionNueva = getTilePositionForSupply(analizador);

		if (posicionNueva!=NULL) posB = posicionNueva;
			
		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_DEPOT);
			delete posB;
		}
		delete building;
	}


	//-- BARRACK
	if((Broodwar->self()->minerals() > 200) && (cantUnidades[Utilidades::INDEX_GOAL_BARRACK] < goalCantUnidades[Utilidades::INDEX_GOAL_BARRACK]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_BARRACK);
		TilePosition* posB = getTilePositionAviable(building);
		
		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_BARRACK);
			delete posB;
		}
		delete building;
	}


	//-- FACTORY
	if((Broodwar->self()->minerals() > 200) && (cantUnidades[Utilidades::INDEX_GOAL_FACTORY] < goalCantUnidades[Utilidades::INDEX_GOAL_FACTORY]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_FACTORY);
		TilePosition* posB = getTilePositionAviable(building);
		
		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_FACTORY);
			delete posB;
		}
		delete building;
	}


	//-- MACHINE SHOP (ADD-ON DE FACTORY)
	if((Broodwar->self()->minerals() > 50) && (Broodwar->self()->gas() > 50) && (cantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP] < goalCantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP]) && (buildingSemaphore == 0)){
		buildUnitAddOn(Utilidades::ID_MACHINESHOP);
	}

	//-- BUNKER
	if ((grupoB1 != NULL) && grupoB1->faltanBunkers() && (Broodwar->self()->minerals() > 150) && (cantUnidades[Utilidades::INDEX_GOAL_BUNKER] < goalCantUnidades[Utilidades::INDEX_GOAL_BUNKER]) && (buildingSemaphore == 0)){
		TilePosition *posB = NULL;

		posB = grupoB1->posicionNuevoBunker();

		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_BUNKER);
			delete posB;
		}
		else
			Broodwar->printf("ERROR: No encuentro posicion para construir el bunker");

	}
	else if ((grupoB2 != NULL) && grupoB2->faltanBunkers() && (Broodwar->self()->minerals() > 150) && (cantUnidades[Utilidades::INDEX_GOAL_BUNKER] < goalCantUnidades[Utilidades::INDEX_GOAL_BUNKER]) && (buildingSemaphore == 0)){
		TilePosition *posB = NULL;

		posB = grupoB2->posicionNuevoBunker();

		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_BUNKER);
			delete posB;
		}
		else
			Broodwar->printf("ERROR: No encuentro posicion para construir el bunker");
	}


	//-- MISILE TURRET
	int cantTurrets = 0;

	if (grupoB1 != NULL)
		cantTurrets += grupoB1->cantMaximaTurrets();

	if (anti != NULL)
		cantTurrets += anti->cantMaximaTurrets();

	if (grupoB2 != NULL)
		cantTurrets += grupoB2->cantMaximaTurrets();

	if ((cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY] > 0) && (cantUnidades[Utilidades::INDEX_GOAL_MISSILE_TURRET] < cantTurrets)){
		TilePosition *posB = NULL;

		if ((grupoB1 != NULL) && (grupoB1->faltanMisileTurrets()) && (Broodwar->self()->minerals() > 100) && (buildingSemaphore == 0)){
			posB = grupoB1->posicionNuevaTorreta();

			if (posB != NULL){
				buildUnit(posB, Utilidades::ID_MISSILE_TURRET);
				delete posB;
			}
		}
		else if ((anti != NULL) && (grupoB1 != NULL) && (!grupoB1->faltanTanques()) && (anti->faltanMisileTurrets()) && (Broodwar->self()->minerals() > 100) && (buildingSemaphore == 0)){
			posB = anti->getPosicionMisileTurret();

			if (posB != NULL){
				if (Broodwar->isExplored(*posB))
					buildUnit(posB, Utilidades::ID_MISSILE_TURRET);
				else{
					Unit *un = getUnit(Utilidades::INDEX_GOAL_COMSAT_STATION);
					if ((un != NULL) && (un->getEnergy() > 50)){
						Position *p = new Position(posB->x() / TILE_SIZE, posB->y() / TILE_SIZE);
						un->useTech(TechTypes::Scanner_Sweep, *p);
						delete p;
					}
				}

				delete posB;
			}
		}
		else if ((grupoB2 != NULL) && (grupoB1 != NULL) && (!grupoB1->faltanTanques()) && (grupoB2->faltanMisileTurrets()) && (Broodwar->self()->minerals() > 100) && (buildingSemaphore == 0)){
			posB = grupoB2->posicionNuevaTorreta();

			if (posB != NULL){
				buildUnit(posB, Utilidades::ID_MISSILE_TURRET);

				delete posB;
			}
		}
		// Borrar esto despues de probar...
		/*else if ((grupoB2 != NULL) && (grupoB2->faltanMisileTurrets()) && (Broodwar->self()->minerals() > 100) && (buildingSemaphore == 0)){
			posB = grupoB2->posicionNuevaTorreta();

			if (posB != NULL){
				buildUnit(posB, Utilidades::ID_MISSILE_TURRET);

				delete posB;
			}
		}*/
	}
	
	

	//-- ACADEMY
	if((Broodwar->self()->minerals() > 200) && (cantUnidades[Utilidades::INDEX_GOAL_ACADEMY] < goalCantUnidades[Utilidades::INDEX_GOAL_ACADEMY]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_ACADEMY);
		TilePosition* posB = getTilePositionAviable(building);

		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_ACADEMY);
			delete posB;
		}
		delete building;
	}


	//-- ENGINEERING BAY
	if((Broodwar->self()->minerals() > 125) && (cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY] < goalCantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_ENGINEERING_BAY);
		TilePosition* posB = getTilePositionAviable(building);

		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_ENGINEERING_BAY);
			delete posB;
		}
		delete building;
	}

	//-- ARMORY
	if((Broodwar->self()->minerals() > 100) && (Broodwar->self()->gas() > 50) && (cantUnidades[Utilidades::INDEX_GOAL_ARMORY] < goalCantUnidades[Utilidades::INDEX_GOAL_ARMORY]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_ARMORY);
		TilePosition* posB = getTilePositionAviable(building);

		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_ARMORY);
			delete posB;
		}
		delete building;
	}

	//-- STARPORT
	if((Broodwar->self()->minerals() > 150) && (Broodwar->self()->gas() > 100) && (cantUnidades[Utilidades::INDEX_GOAL_STARPORT] < goalCantUnidades[Utilidades::INDEX_GOAL_STARPORT]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_STARPORT);
		TilePosition* posB = getTilePositionAviable(building);

		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_STARPORT);
			delete posB;
		}
		delete building;
	}

	//-- CONTROL TOWER (ADD-ON DE STARPORT)
	if((Broodwar->self()->minerals() > 50) && (Broodwar->self()->gas() > 50) && (cantUnidades[Utilidades::INDEX_GOAL_CONTROL_TOWER] < goalCantUnidades[Utilidades::INDEX_GOAL_CONTROL_TOWER])){
		buildUnitAddOn(Utilidades::ID_CONTROL_TOWER);
	}

	//-- SCIENCE FACILITY
	if((Broodwar->self()->minerals() > 100) && (Broodwar->self()->gas() > 150) && (cantUnidades[Utilidades::INDEX_GOAL_SCIENCE_FACILITY] < goalCantUnidades[Utilidades::INDEX_GOAL_SCIENCE_FACILITY]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_SCIENCE_FACILITY);
		TilePosition* posB = getTilePositionAviable(building);

		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_SCIENCE_FACILITY);
			delete posB;
		}
		delete building;
	}

	//-- COVERT OPS (ADD-ON DE SCIENSE FACILITY)
	if((Broodwar->self()->minerals() > 50) && (Broodwar->self()->gas() > 50) && (cantUnidades[Utilidades::INDEX_GOAL_COVERT_OPS] < goalCantUnidades[Utilidades::INDEX_GOAL_COVERT_OPS])){
		buildUnitAddOn(Utilidades::ID_COVERT_OPS);
	}
	
	//-- NAVE DE LA CIENCIA - SCIENCE VESSEL
	if(cantUnidades[Utilidades::INDEX_GOAL_STARPORT] && (cantUnidades[Utilidades::INDEX_GOAL_SCIENCE_VESSEL] < goalCantUnidades[Utilidades::INDEX_GOAL_SCIENCE_VESSEL]) && (Broodwar->self()->minerals()>= 100) && (Broodwar->self()->gas()>= 225)) {
		trainUnit(Utilidades::ID_SCIENCE_VESSEL);
	}

	//-- DROPSHIP
	if((Broodwar->self()->minerals() > 100) && (Broodwar->self()->gas() > 100) && ((estadoActual == 5) && (ct != NULL) && (ct->faltanDropships())) && (buildingSemaphore == 0)){
		trainUnit(Utilidades::ID_DROPSHIP);
	}

	//-- WRAITH
	if((Broodwar->self()->minerals() > 150) && (Broodwar->self()->gas() > 100) && ((estadoActual == 5) && (ct != NULL) && (ct->faltanWraiths())) && (buildingSemaphore == 0)){
		trainUnit(Utilidades::ID_WRAITH);
	}


	// ----------------------------------------------------------------------------
	//								Investigaciones
	// ----------------------------------------------------------------------------

	//-- STIM PACKS
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

	//-- U238 SHELLS
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

	//-- RESTORATION
	if (!researchDone[Utilidades::INDEX_GOAL_RESTORATION]){
		// Recupera a las maquinas de lás paralisis
		if ((cantUnidades[Utilidades::INDEX_GOAL_ACADEMY] > 0) && (Broodwar->self()->minerals() > 150) && (Broodwar->self()->gas() > 150) && (goalResearch[Utilidades::INDEX_GOAL_RESTORATION] == 1)){
			Unit *u;

			u = getUnit(Utilidades::ID_ACADEMY);

			if (u != NULL){

				if ((u->isCompleted()) && (!u->isResearching()) && (!u->isUpgrading()) ){
					// Construccion de la academia finalizada, se puede investigar mejoras

					Broodwar->printf("Investigando Restoration en academia...");
					TechType *t = new TechType(TechTypes::Restoration);
					u->research(*t);
					delete t;
					
					researchDone[Utilidades::INDEX_GOAL_RESTORATION] = true;
				}
			}
		}
	}


	//-- SIEGE MODE
	if (!researchDone[Utilidades::INDEX_GOAL_TANK_SIEGE_MODE]){
		// investigacion de modo asedio de tanques
		if ((cantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP] > 0) && (Broodwar->self()->minerals() > 150) && (Broodwar->self()->gas() > 150) && (goalResearch[Utilidades::INDEX_GOAL_TANK_SIEGE_MODE] == 1)){
			Unit *u;

			u = getUnit(Utilidades::ID_MACHINESHOP);

			if (u != NULL){

				if ((u->isCompleted()) && (!u->isResearching()) && (!u->isUpgrading()) ){
					Broodwar->printf("Investigando ampliación a Modo Asedio");
					TechType *t = new TechType(TechTypes::Tank_Siege_Mode);
					u->research(*t);
					delete t;
					
					researchDone[Utilidades::INDEX_GOAL_TANK_SIEGE_MODE] = true;
				}
			}
		}
	}

	//-- INFANTRY WEAPONS LEVEL 1
	if (!researchDone[Utilidades::INDEX_GOAL_INFANTRY_WEAPONS_LVL1]){
		// mejora de armamento de marines nivel 1 (se investiga en bahia de ingenieria)
		if ((cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY] > 0) && (Broodwar->self()->minerals() > 100) && (Broodwar->self()->gas() > 100) && (goalResearch[Utilidades::INDEX_GOAL_INFANTRY_WEAPONS_LVL1] == 1)){
			Unit *u;
			u = getUnit(Utilidades::ID_ENGINEERING_BAY);

			if (u != NULL){

				if ((u->isCompleted()) && (!u->isResearching()) && (!u->isUpgrading()) ){
					Broodwar->printf("Investigando mejora de armamento de marines (Nivel 1)");
					UpgradeType *t = new UpgradeType(UpgradeTypes::Terran_Infantry_Weapons);
					u->upgrade(*t);
					delete t;
					
					researchDone[Utilidades::INDEX_GOAL_INFANTRY_WEAPONS_LVL1] = true;
				}
			}
		}
	}

	//-- INFANTRY ARMOR LEVEL 1
	if (!researchDone[Utilidades::INDEX_GOAL_INFANTRY_ARMOR_LVL1]){
		// mejora de armamento de marines nivel 1 (se investiga en bahia de ingenieria)
		if ((cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY] > 0) && (Broodwar->self()->minerals() > 100) && (Broodwar->self()->gas() > 100) && (goalResearch[Utilidades::INDEX_GOAL_INFANTRY_ARMOR_LVL1] == 1)){
			Unit *u;
			u = getUnit(Utilidades::ID_ENGINEERING_BAY);

			if (u != NULL){

				if ((u->isCompleted()) && (!u->isResearching()) && (!u->isUpgrading()) ){
					Broodwar->printf("Investigando mejora de armadura de marines (Nivel 1)");
					UpgradeType *t = new UpgradeType(UpgradeTypes::Terran_Infantry_Armor);
					u->upgrade(*t);
					delete t;
					
					researchDone[Utilidades::INDEX_GOAL_INFANTRY_ARMOR_LVL1] = true;
				}
			}
		}
	}

	//-- INFANTRY WEAPONS LEVEL 2
	if ((cantUnidades[Utilidades::INDEX_GOAL_SCIENCE_FACILITY]>0)&&(researchDone[Utilidades::INDEX_GOAL_INFANTRY_WEAPONS_LVL1])&&(!researchDone[Utilidades::INDEX_GOAL_INFANTRY_WEAPONS_LVL2])){
		// mejora de armamento de marines nivel 3 (se investiga en bahia de ingenieria)
		if ((cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY] > 0) && (Broodwar->self()->minerals() > 175) && (Broodwar->self()->gas() > 175) && (goalResearch[Utilidades::INDEX_GOAL_INFANTRY_WEAPONS_LVL2] == 1)){
			Unit *u;
			u = getUnit(Utilidades::ID_ENGINEERING_BAY);

			if (u != NULL){

				if ((u->isCompleted()) && (!u->isResearching()) && (!u->isUpgrading()) ){
					Broodwar->printf("Investigando mejora de armamento de marines (Nivel 2)");
					UpgradeType *t = new UpgradeType(UpgradeTypes::Terran_Infantry_Weapons);
					u->upgrade(*t);
					delete t;
					
					researchDone[Utilidades::INDEX_GOAL_INFANTRY_WEAPONS_LVL2] = true;
				}
			}
		}
	}

	//-- INFANTRY WEAPONS LEVEL 3
	if ((cantUnidades[Utilidades::INDEX_GOAL_SCIENCE_FACILITY]>0)&&(researchDone[Utilidades::INDEX_GOAL_INFANTRY_WEAPONS_LVL2])&&(!researchDone[Utilidades::INDEX_GOAL_INFANTRY_WEAPONS_LVL3])){
		// mejora de armamento de marines nivel 3 (se investiga en bahia de ingenieria)
		if ((cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY] > 0) && (Broodwar->self()->minerals() > 250) && (Broodwar->self()->gas() > 250) && (goalResearch[Utilidades::INDEX_GOAL_INFANTRY_WEAPONS_LVL3] == 1)){
			Unit *u;
			u = getUnit(Utilidades::ID_ENGINEERING_BAY);

			if (u != NULL){

				if ((u->isCompleted()) && (!u->isResearching()) && (!u->isUpgrading()) ){
					Broodwar->printf("Investigando mejora de armamento de marines (Nivel 3)");
					UpgradeType *t = new UpgradeType(UpgradeTypes::Terran_Infantry_Weapons);
					u->upgrade(*t);
					delete t;
					
					researchDone[Utilidades::INDEX_GOAL_INFANTRY_WEAPONS_LVL3] = true;
				}
			}
		}
	}

	//-- INFANTRY ARMOR LEVEL 2
	if ((cantUnidades[Utilidades::INDEX_GOAL_SCIENCE_FACILITY]>0)&&(researchDone[Utilidades::INDEX_GOAL_INFANTRY_ARMOR_LVL1])&&(!researchDone[Utilidades::INDEX_GOAL_INFANTRY_ARMOR_LVL2])){
		// mejora de armamento de marines nivel 1 (se investiga en bahia de ingenieria)
		if ((cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY] > 0) && (Broodwar->self()->minerals() > 175) && (Broodwar->self()->gas() > 175) && (goalResearch[Utilidades::INDEX_GOAL_INFANTRY_ARMOR_LVL2] == 1)){
			Unit *u;
			u = getUnit(Utilidades::ID_ENGINEERING_BAY);

			if (u != NULL){

				if ((u->isCompleted()) && (!u->isResearching()) && (!u->isUpgrading()) ){
					Broodwar->printf("Investigando mejora de armadura de marines (Nivel 2)");
					UpgradeType *t = new UpgradeType(UpgradeTypes::Terran_Infantry_Armor);
					u->upgrade(*t);
					delete t;
					
					researchDone[Utilidades::INDEX_GOAL_INFANTRY_ARMOR_LVL2] = true;
				}
			}
		}
	}

	//-- INFANTRY ARMOR LEVEL 3
	if ((cantUnidades[Utilidades::INDEX_GOAL_SCIENCE_FACILITY]>0)&&(researchDone[Utilidades::INDEX_GOAL_INFANTRY_ARMOR_LVL2])&&(!researchDone[Utilidades::INDEX_GOAL_INFANTRY_ARMOR_LVL3])){
		// mejora de armamento de marines nivel 1 (se investiga en bahia de ingenieria)
		if ((cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY] > 0) && (Broodwar->self()->minerals() > 250) && (Broodwar->self()->gas() > 250) && (goalResearch[Utilidades::INDEX_GOAL_INFANTRY_ARMOR_LVL3] == 1)){
			Unit *u;
			u = getUnit(Utilidades::ID_ENGINEERING_BAY);

			if (u != NULL){

				if ((u->isCompleted()) && (!u->isResearching()) && (!u->isUpgrading()) ){
					Broodwar->printf("Investigando mejora de armadura de marines (Nivel 3)");
					UpgradeType *t = new UpgradeType(UpgradeTypes::Terran_Infantry_Armor);
					u->upgrade(*t);
					delete t;
					
					researchDone[Utilidades::INDEX_GOAL_INFANTRY_ARMOR_LVL3] = true;
				}
			}
		}
	}


	//-- VEHICLE ARMOR LEVEL 1
	if (!researchDone[Utilidades::INDEX_GOAL_VEHICLE_PLATING_LVL1]){
		// mejora de armamento de vehiculos nivel 1 (se investiga en armory
		if ((cantUnidades[Utilidades::INDEX_GOAL_ARMORY] > 0) && (Broodwar->self()->minerals() > 100) && (Broodwar->self()->gas() > 100) && (goalResearch[Utilidades::INDEX_GOAL_VEHICLE_PLATING_LVL1] == 1)){
			Unit *u;
			u = getUnit(Utilidades::ID_ARMORY);
			if (u != NULL){
				if ((u->isCompleted()) && (!u->isResearching()) && (!u->isUpgrading()) ){
					Broodwar->printf("Investigando mejora de armadura de vehiculos (Nivel 1)");
					UpgradeType *t = new UpgradeType(UpgradeTypes::Terran_Vehicle_Plating);
					u->upgrade(*t);
					delete t;
					
					researchDone[Utilidades::INDEX_GOAL_VEHICLE_PLATING_LVL1] = true;
				}
			}
		}
	}

	//-- VEHICLE WEAPONS LEVEL 1
	if (!researchDone[Utilidades::INDEX_GOAL_VEHICLE_WEAPONS_LVL1]){
		// mejora de armamento de vehiculos nivel 1 (se investiga en armory
		if ((cantUnidades[Utilidades::INDEX_GOAL_ARMORY] > 0) && (Broodwar->self()->minerals() > 150) && (Broodwar->self()->gas() > 150) && (goalResearch[Utilidades::INDEX_GOAL_VEHICLE_WEAPONS_LVL1] == 1)){
			Unit *u;
			u = getUnit(Utilidades::ID_ARMORY);

			if (u != NULL){
				if ((u->isCompleted()) && (!u->isResearching()) && (!u->isUpgrading()) ){
					Broodwar->printf("Investigando mejora de armamento de vehiculos (Nivel 1)");
					UpgradeType *t = new UpgradeType(UpgradeTypes::Terran_Vehicle_Weapons);
					u->upgrade(*t);
					delete t;
					
					researchDone[Utilidades::INDEX_GOAL_VEHICLE_WEAPONS_LVL1] = true;
				}
			}
		}
	}


	//-- VEHICLE WEAPONS LEVEL 2
	if ((cantUnidades[Utilidades::INDEX_GOAL_SCIENCE_FACILITY]>0)&& (researchDone[Utilidades::INDEX_GOAL_VEHICLE_WEAPONS_LVL1])&& (!researchDone[Utilidades::INDEX_GOAL_VEHICLE_WEAPONS_LVL2]) ){
		// mejora de armamento de vehiculos nivel 1 (se investiga en armory
		if ((cantUnidades[Utilidades::INDEX_GOAL_ARMORY] > 0) && (Broodwar->self()->minerals() > 175) && (Broodwar->self()->gas() > 175) && (goalResearch[Utilidades::INDEX_GOAL_VEHICLE_WEAPONS_LVL2] == 1)){
			Unit *u;
			u = getUnit(Utilidades::ID_ARMORY);

			if (u != NULL){
				if ((u->isCompleted()) && (!u->isResearching()) && (!u->isUpgrading()) ){
					Broodwar->printf("Investigando mejora de armamento de vehiculos (Nivel 2)");
					UpgradeType *t = new UpgradeType(UpgradeTypes::Terran_Vehicle_Weapons);
					u->upgrade(*t);
					delete t;
					
					researchDone[Utilidades::INDEX_GOAL_VEHICLE_WEAPONS_LVL2] = true;
				}
			}
		}
	}



	//-- OPTICAL FLARE
	if (!researchDone[Utilidades::INDEX_GOAL_OPTICAL_FLARE]){
		// mejora de bengalas opticas para los medicos (se investiga en academia terran)
		if ((cantUnidades[Utilidades::INDEX_GOAL_ACADEMY] > 0) && (Broodwar->self()->minerals() > 100) && (Broodwar->self()->gas() > 100) && (goalResearch[Utilidades::INDEX_GOAL_OPTICAL_FLARE] == 1)){
			Unit *u = getUnit(Utilidades::ID_ACADEMY);

			if (u != NULL){
				if ((u->isCompleted()) && (!u->isResearching()) && (!u->isUpgrading()) ){
					// Construccion de la academia finalizada, se puede investigar mejoras

					Broodwar->printf("Investigando mejora de bengala optica...");
					TechType *t = new TechType(TechTypes::Optical_Flare);
					u->research(*t);
					delete t;
					
					researchDone[Utilidades::INDEX_GOAL_OPTICAL_FLARE] = true;
				}
			}
		}
	}

	//-- LOCKDOWN
	if (!researchDone[Utilidades::INDEX_GOAL_LOCKDOWN]){
		// mejora de lockdown para los fantasmas
		if ((cantUnidades[Utilidades::ID_COVERT_OPS] > 0) && (Broodwar->self()->minerals() >= 200) && (Broodwar->self()->gas() >= 200) && (goalResearch[Utilidades::INDEX_GOAL_LOCKDOWN] == 1)){
			Unit *u = getUnit(Utilidades::ID_COVERT_OPS);

			if (u != NULL){
				if ((u->isCompleted()) && (!u->isResearching()) && (!u->isUpgrading()) ){
					Broodwar->printf("Investigando lockdown...");
					TechType *t = new TechType(TechTypes::Lockdown);
					u->research(*t);
					delete t;
					
					researchDone[Utilidades::INDEX_GOAL_LOCKDOWN] = true;
				}
			}
		}
	}

}



void unit_Manager::buildUnit(TilePosition *pos, int id){

	Unit* trabajador;
	UnitType *tipo = new UnitType(id);

	Position *p = NULL;
	TilePosition *t = NULL;

	if ((Broodwar->self()->minerals()>tipo->mineralPrice())&&(Broodwar->self()->gas()>=tipo->gasPrice())){
		trabajador = getWorker();
		
		if ((trabajador!=NULL) && (trabajador->exists()) && (!trabajador->isConstructing()) && (trabajador->isCompleted())) {
			if (Broodwar->canBuildHere(trabajador, *pos, *tipo)){

				//-- si la zona no fue explorada todavia, manda el scv a explorar la zona y para poder construir en la 
				//-- proxima llamada al metodo
				if (!Broodwar->isExplored(*pos)){
					Position *aux = new Position(pos->x() * TILE_SIZE, pos->y() * TILE_SIZE);
					trabajador->move(*aux);
					delete aux;
				}
				else{
					buildingSemaphore++;
					trabajador->build((*pos), *tipo);
				}
			}
			else{
				// verifica si hay unidades en cada tile que ocupara la construccion, y si las hay, las mueve a otra posicion
				for (int x = 0; x < tipo->tileWidth(); x++){
					for (int y = 0; y < tipo->tileHeight(); y++){

						// referencia al tile actual
						t = new TilePosition(pos->x() + x, pos->y() + y);
						
						// si hay unidades en el tile actual, pregunta a que compañia pertenecen para moverlas a otra posicion
						if (!Broodwar->unitsOnTile(t->x(), t->y()).empty()){

							// posicion hacia donde mover las unidades
							p = new Position((pos->x() + tipo->tileWidth() + 1) * 32, (pos->y() + tipo->tileHeight() + 1) * 32);

							// verifica a que compañia pertenecen las unidades
							std::set<Unit*>::const_iterator It1 = Broodwar->unitsOnTile(t->x(), t->y()).begin();
							
							// mueve cada unidad en el tile a otra posicion (solamente si la unidad en cuestion no es una construccion fija)
							while (It1 != Broodwar->unitsOnTile(t->x(), t->y()).end()){
								if ((Easy != NULL) && (Easy->pertenece(*It1))){
									Easy->moverCompania(*p);
								}
								else if ((grupoB1 != NULL) && (grupoB1->perteneceMarine(*It1))){
									grupoB1->moverSoldadosPosEncuentro();
								}
								else if ((grupoB2 != NULL) && (grupoB2->perteneceMarine(*It1))){
									grupoB2->moverSoldadosPosEncuentro();
								}
								else{
									if (((*It1)->exists()) && (!(*It1)->getType().isBuilding()))
										(*It1)->move(*p);
								}
								It1++;
							}

							delete p;
						}

						delete t;
					}
				}
			}
		}
	}

}


void unit_Manager::buildUnitAddOn(int id){
	
	Unit* owner = NULL;
	UnitType *tipo = new UnitType(id);
	

	if((tipo->mineralPrice() < Broodwar->self()->minerals()) && (tipo->gasPrice() < Broodwar->self()->gas())){

		if (id == Utilidades::ID_MACHINESHOP){
			owner = getUnit(Utilidades::ID_FACTORY);
		}
		else if (id == Utilidades::ID_COMSAT_STATION){
			owner = getUnit(Utilidades::ID_COMMANDCENTER);
		}
		else if (id == Utilidades::ID_CONTROL_TOWER){
			owner = getUnit(Utilidades::ID_STARPORT);
		}
		else if (id == Utilidades::ID_COVERT_OPS){
			owner = getUnit(Utilidades::ID_SCIENCE_FACILITY);
		}

		if ((owner != NULL) && (owner->exists()) && (owner->isCompleted())){
			Unit* bunk = getUnit(Utilidades::ID_BUNKER);
			if (!owner->buildAddon(*tipo)){
				int x=0;
				bool libre = true;
				while ((x<2)&&(libre)){
					if (owner->getTilePosition().x()+ tipo->tileWidth()+x < Broodwar->mapWidth()){
						int y=0;
						while((y < tipo->tileHeight())&& (libre)){
							
							if (Broodwar->isBuildable(owner->getTilePosition().x()+ owner->getType().tileWidth()+x, owner->getTilePosition().y()+y)){
								std::set< Unit* > unidadesEnTile = Broodwar->unitsOnTile(owner->getTilePosition().x()+ owner->getType().tileWidth()+x, owner->getTilePosition().y()+y);
								std::set<Unit*>::iterator It1;
								It1 = unidadesEnTile.begin();
								while((It1 != unidadesEnTile.end())&& (libre)){
									if((*It1)->getType().isBuilding()) {
										libre = false;
										//owner->lift();
									}
									else{
										if ((bunk!=NULL)&&(bunk->exists()))
											(*It1)->rightClick(bunk->getTilePosition());
										It1++;
									}
								}
							}
							else{
								libre = false;
								Broodwar->printf("no es buildable");
								//owner->lift();
							}
							y++;
						}
					}
					else{
						libre = false;
						Broodwar->printf("fuera del mapa");
						//owner->lift();
					}
					x++;
				}
							

			}
		}


		delete tipo;
		
	}
}


TilePosition* unit_Manager::getPosicionDistinta(TilePosition actual){
	Unit * centro = getUnit(Utilidades::ID_COMMANDCENTER);

	if ((centro!=NULL)&&(centro->exists())){
		int orientacionEsteOeste = centro->getTilePosition().x() - actual.x();
		int orientacionNorteSur = centro->getTilePosition().y() - actual.y();
		
				
		if (abs(orientacionEsteOeste)>abs(orientacionNorteSur)){
			if (orientacionEsteOeste < 0) {
				if (actual.x()+1 + 6 < Broodwar->mapWidth()*4){
					return new TilePosition(actual.x()+1, actual.y());
				}
				else{
					if (orientacionNorteSur < 0) {
						if (actual.y()+1 + 3 < Broodwar->mapHeight()*4)
							return new TilePosition(actual.x(), actual.y()+1);
					}
					else{
						if (actual.y()-1 > 1)
							return new TilePosition(actual.x(), actual.y()-1);
					}				
				}
			}
			else{
				if (actual.x()-1 > 1){
					return new TilePosition(actual.x()-1, actual.y());
				}
				else{
					if (orientacionNorteSur < 0) {
						if (actual.y()+1 + 3 < Broodwar->mapHeight()*4)
							return new TilePosition(actual.x(), actual.y()+1);
					}
					else{
						if (actual.y()-1 > 1)
							return new TilePosition(actual.x(), actual.y()-1);
					}
				}
			}
		}
		else{
			if (orientacionNorteSur < 0) {
				if (actual.y()+1 + 3 < Broodwar->mapHeight()*4){
					return new TilePosition(actual.x(), actual.y()+1);
				}
				else{
					if (orientacionEsteOeste < 0) {
						if (actual.x()+1 + 6 < Broodwar->mapWidth()*4){
							return new TilePosition(actual.x()+1, actual.y());
						}
										
					}
					else{
						if (actual.x()-1 > 1){
							return new TilePosition(actual.x()-1, actual.y());
						}
						
					}
				
				}
			}
			else{
				if (actual.y()-1 > 1)
					return new TilePosition(actual.x(), actual.y()-1);
			}		
		
		}
	}
	Broodwar->printf("no es posicion válida");
	return new TilePosition(actual.x(), actual.y());
}


void unit_Manager::makeRefinery(TilePosition *pos){
// el parámetro POS puede ser nulo, lo que indica que tiene que contruir la refineria en el geiser mas cercano
// si el parámetro tiene algun valor, se intenta construir sobre esa ubicación.
	Unit *trabajador;
	TilePosition *geyserPos = NULL;

	if ((cantUnidades[Utilidades::INDEX_GOAL_REFINERY] < goalLimiteGeiser) && (Broodwar->self()->minerals()>50)){
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
				geyserPos = pos;
			}
		
			if (Broodwar->self()->minerals() > 150) {
				trabajador->build((*geyserPos), *(new UnitType(Utilidades::ID_REFINERY)));
			}
		}

	}
}

Unit* unit_Manager::getWorker(){
	Unit* trabajador = NULL;
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
		if (((*i)->exists()) && ((*i)->getType().isWorker())){
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

	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
		if ((*i)->getType().getID() == IDTipo){
			return (*i);
		}
	}

	return NULL;
}


void unit_Manager::trainWorker(){
	if (centroDeComando->exists()){
		if((Broodwar->self()->minerals()>50) && (centroDeComando->getTrainingQueue().size() < 2)){	
			centroDeComando->train(Broodwar->self()->getRace().getWorker());
		}
	}
}

void unit_Manager::trainMarine(){
	Unit* firstBarrack = NULL;
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		if ((*i)->getType().getID() == Utilidades::ID_BARRACK){
			firstBarrack = (*i);

			if ((firstBarrack != NULL) && (Broodwar->canMake(firstBarrack, Utilidades::ID_MARINE)) && (firstBarrack->getTrainingQueue().size() < 2)){
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
			
			if ((firstBarrack != NULL) && (Broodwar->canMake(firstBarrack, Utilidades::ID_MEDIC)) && (firstBarrack->getTrainingQueue().size() < 2)){
				firstBarrack->train(*(new UnitType(Utilidades::ID_MEDIC)));
				break;
			}
		}
	}
}

void unit_Manager::trainGhost(){
	Unit* firstBarrack = NULL;
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		if ((*i)->getType().getID() == Utilidades::ID_BARRACK){
			firstBarrack = (*i);
			
			if ((firstBarrack != NULL) && (Broodwar->canMake(firstBarrack, Utilidades::ID_GHOST)) && (firstBarrack->getTrainingQueue().size() == 0)){
				firstBarrack->train(*(new UnitType(Utilidades::ID_GHOST)));
				break;
			}
		}
	}
}


void unit_Manager::trainGoliath(){
	Unit* firstFactory = NULL;
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		if ((*i)->getType().getID() == Utilidades::ID_FACTORY){
			firstFactory = (*i);

			if ((firstFactory != NULL) && (Broodwar->canMake(firstFactory, Utilidades::ID_GOLIATH)) && (firstFactory->getTrainingQueue().size() < 2)){
				firstFactory->train(*(new UnitType(Utilidades::ID_GOLIATH)));
				break;
			}
		}
	}
}



void unit_Manager::trainVulture(){
Unit* firstFactory = NULL;
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		if ((*i)->getType().getID() == Utilidades::ID_FACTORY){
			firstFactory = (*i);

			if ((firstFactory != NULL) && (Broodwar->canMake(firstFactory, Utilidades::ID_VULTURE)) && (firstFactory->getTrainingQueue().size() < 1)){
				firstFactory->train(*(new UnitType(Utilidades::ID_VULTURE)));
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

			if ((firstFactory != NULL) && (Broodwar->canMake(firstFactory, Utilidades::ID_TANKSIEGE)) && (firstFactory->getTrainingQueue().size() == 0)){
				firstFactory->train(*(new UnitType(Utilidades::ID_TANKSIEGE)));
				break;
			}
		}
	}
}




void unit_Manager::trainUnit(int id){
	if ((id == Utilidades::ID_DROPSHIP)|| (id == Utilidades::ID_SCIENCE_VESSEL) || (id == Utilidades::ID_WRAITH)){
		Unit *constructor = getUnit(Utilidades::ID_STARPORT);
		UnitType *tipo = new UnitType(id);

		if ((constructor != NULL) && (constructor->exists()) && (constructor->getTrainingQueue().size() == 0) && (Broodwar->canMake(constructor, *tipo))){
			constructor->train(*tipo);
		}

		delete tipo;
	}
}

void unit_Manager::sendGatherCristal(Unit* worker){
	
	if (worker != NULL){
		Unit* closestMineral=NULL;
		int menordistancia = 10000;

		//busca el mineral más cercano.
		for(std::set<Unit*>::const_iterator centroCmd=Broodwar->self()->getUnits().begin();centroCmd!=Broodwar->self()->getUnits().end();centroCmd++){
			if((*centroCmd)->getType().getID()== Utilidades::ID_COMMANDCENTER){
				for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++){
					if (closestMineral == NULL || ((*centroCmd)->getDistance(*m)<menordistancia)){
						closestMineral = *m;
						menordistancia = (int)(*centroCmd)->getDistance(*m);
					}
				}
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
				if ((closestGeyser == NULL) || ((*i)->getResources()>0))
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

TilePosition* unit_Manager::getTilePositionForSupply(AnalizadorTerreno *analizador){
	TilePosition* ubicacion = NULL;
	if(analizador->analisisListo()){
		
		TilePosition* choke = new TilePosition(*analizador->obtenerCentroChokepoint());
		TilePosition centroRegion = analizador->regionInicial()->getCenter();
		int distanciaEste = centroRegion.x();
		int distanciaOeste = (Broodwar->mapWidth()) - centroRegion.x();
		int distanciaSur = (Broodwar->mapHeight()) - centroRegion.y();
		int distanciaNorte = centroRegion.y();
		UnitType* supply = new UnitType(Utilidades::ID_DEPOT);
		Unit* centrocomando = getUnit(Utilidades::ID_COMMANDCENTER);

		int x=0;
		int y=0;
		bool dentroDeRegion = true;
		
		int distanciaVertical, distanciaHorizontal;
		if(distanciaEste > distanciaOeste)
			distanciaHorizontal = distanciaOeste;
		else distanciaHorizontal = distanciaEste;

		if (distanciaNorte > distanciaSur)
			distanciaVertical = distanciaSur;
		else distanciaVertical = distanciaNorte;

		Unit* worker = getWorker();
		if (distanciaHorizontal<distanciaVertical){
			
			if(distanciaEste < distanciaOeste){
				if(choke->y()>centroRegion.y()){
					
					while(ubicacion == NULL){
						y=0;
						dentroDeRegion = true;
						while((centroRegion.y()+y>=0)&&(ubicacion == NULL) && (dentroDeRegion)){
							
							ubicacion = new TilePosition(0+x,centroRegion.y()+y);
							if(isInsideRegion(analizador,supply,ubicacion)){
								if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)/32>5)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
									return ubicacion;
								}
								else{	
									ubicacion = NULL;
								}
							}
							else{
								dentroDeRegion = false;
								ubicacion = NULL;
							}
							y--;	
						}
						x++;
					}
				}
				else{
					while(ubicacion == NULL){
						y=0;
						dentroDeRegion = true;
						while((centroRegion.y()+y<=Broodwar->mapHeight())&&(ubicacion == NULL) && (dentroDeRegion)){
							
							ubicacion = new TilePosition(0+x,centroRegion.y()+y);
							if(isInsideRegion(analizador,supply,ubicacion)){
								if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)/32>5)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
									return ubicacion;
								}
								else{	
									ubicacion = NULL;
								}
							}
							else{
								dentroDeRegion = false;
								ubicacion = NULL;
							}
							y++;	
						}
						x++;
					}

				}



			}
			else{
				x= -3;
				if(choke->y()>centroRegion.y()){
						
						while(ubicacion == NULL){
							y=0;
							dentroDeRegion = true;
							while((centroRegion.y()+y>=0)&&(ubicacion == NULL) && (dentroDeRegion)){
								
								ubicacion = new TilePosition(Broodwar->mapWidth()+x,centroRegion.y()+y);
								if(isInsideRegion(analizador,supply,ubicacion)){
									if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)/32>5)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
										return ubicacion;
									}
									else{	
										ubicacion = NULL;
									}
								}
								else{
									dentroDeRegion = false;
									ubicacion = NULL;
								}
								y--;	
							}
							x--;
						}
					}
					else{
						while(ubicacion == NULL){
							y=0;
							dentroDeRegion=true;
							while((centroRegion.y()+y<=Broodwar->mapHeight())&&(ubicacion == NULL) && (dentroDeRegion)){
								
								ubicacion = new TilePosition(Broodwar->mapWidth()+x,centroRegion.y()+y);
								if(isInsideRegion(analizador,supply,ubicacion)){
									if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)/32>5)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
										return ubicacion;
									}
									else{	
										ubicacion = NULL;
									}
								}
								else{
									dentroDeRegion = false;
									ubicacion = NULL;
								}
								y++;	
							}
							x--;
						}
					}
				}
			}
		else{
			if(distanciaNorte < distanciaSur){
				if(choke->x()>centroRegion.x()){
					
					while(ubicacion == NULL){
						x=0;
						dentroDeRegion = true;
						while((centroRegion.x()+x>=0)&&(ubicacion == NULL) && (dentroDeRegion)){
							
							ubicacion = new TilePosition(centroRegion.x()+x,0+y);
							if(isInsideRegion(analizador,supply,ubicacion)){
								if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)/32>5)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
									return ubicacion;
								}
								else{	
									ubicacion = NULL;
								}
							}
							else{
								dentroDeRegion = false;
								ubicacion = NULL;
							}
							x--;	
						}
						y++;
					}
				}
				else{
					while(ubicacion == NULL){
						x=0;
						dentroDeRegion = true;
						while((centroRegion.x()+x<=Broodwar->mapWidth())&&(ubicacion == NULL) && (dentroDeRegion)){
							
							ubicacion = new TilePosition(centroRegion.x()+x,0+y);
							if(isInsideRegion(analizador,supply,ubicacion)){
								if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)/32>5)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
									return ubicacion;
								}
								else{	
									ubicacion = NULL;
								}
							}
							else{
								dentroDeRegion = false;
								ubicacion = NULL;
							}
							x++;	
						}
						y++;
					}

				}



			}
			else{
				y= -2;
				if(choke->x()>centroRegion.x()){
						
						while(ubicacion == NULL){
							x=0;
							dentroDeRegion=true;
							while((centroRegion.x()+x>=0)&&(ubicacion == NULL) && (dentroDeRegion==true)){
								ubicacion = new TilePosition(centroRegion.x()+x,Broodwar->mapHeight()+y);
								
								if(isInsideRegion(analizador,supply,ubicacion)){
									if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)/32>5)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
										return ubicacion;
									}
									else{	
										ubicacion = NULL;
									}
								}
								else{
									dentroDeRegion = false;
									ubicacion = NULL;
								}
								x--;	
							}
							y--;
						}
					}
					else{
						while(ubicacion == NULL){
							x=0;
							dentroDeRegion=true;
							while((centroRegion.x()+x<=Broodwar->mapWidth())&&(ubicacion == NULL) && (dentroDeRegion==true)){
								ubicacion = new TilePosition(centroRegion.x()+x,Broodwar->mapHeight()+y);
								if(isInsideRegion(analizador,supply,ubicacion)){
									if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)/32>5)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
										return ubicacion;
									}
									else{	
										ubicacion = NULL;
									}
								}
								else{
									dentroDeRegion = false;
									ubicacion = NULL;
								}
								x++;	
							}
							y--;
						}
					}
				}
		}


	}
	return ubicacion;
}


bool unit_Manager::isInsideRegion(AnalizadorTerreno *analizador, UnitType* U, TilePosition* P){
	
	bool dentro = true;
	int x = 0;
	int y = 0;
	TilePosition* pos;
	while((dentro)&&(x <U->tileWidth())){
		while((dentro)&&(y <U->tileHeight())){
			pos = new TilePosition(P->x()+x,P->y()+y);
			if((!(analizador->regionInicial()->getPolygon().isInside(*new Position(*pos)))))
				return false;
			y++;
		}
		x++;
	}
	return dentro;
}


// Obtiene un TilePosition disponible en las cercanias del centro de comando
TilePosition* unit_Manager::getTilePositionAviable(UnitType* U){
	TilePosition* pos = NULL;
	int i = 6;
	if ((U->getID() == Utilidades::ID_FACTORY) || (U->getID() == Utilidades::ID_STARPORT))
		i +=2;
	Unit* worker = getWorker();
	int x = centroComando->x();
	int y = centroComando->y();
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
							if ((Broodwar->canBuildHere(worker, *pos, *U))&& (isFreeOfBuildingsRightAndLeft(U, pos))) {encontre = 1;}
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
							if ((Broodwar->canBuildHere(worker, *pos, *U))&& (isFreeOfBuildingsRightAndLeft(U, pos))) {encontre = 1;}
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
						if ((Broodwar->canBuildHere(worker, *pos, *U))&& (isFreeOfBuildingsRightAndLeft(U, pos))) {encontre = 1;}
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
						if ((Broodwar->canBuildHere(worker, *pos, *U))&& (isFreeOfBuildingsRightAndLeft(U, pos))) {encontre = 1;}
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
	Unit* centro = getUnit(Utilidades::ID_COMMANDCENTER);
	if ((centro!=NULL) && (centro->exists())){
		int i = (int) ceil(centro->getTilePosition().getDistance(*t) + 0.1);

		Broodwar->printf("LA DISTANCIA ES: %d", i);
		int x = t->x();
		int y = t->y();
		int j, k;
		int encontre=0;
		if ((worker != NULL)&& (worker->exists())) {
			while (encontre==0){
				j = -i;
				if (x+j>=0){
					k = i;
					while((k>=-i) && (encontre==0)){
						if ((y+k>=0)&& (!((x+j>x-1) && (x+j<x+5) && (y+k>y-1) && (y+k<y+4)))){
							pos = new TilePosition(x + j, y + k);
							if(Broodwar->isExplored(*pos)){
								if (Broodwar->canBuildHere(worker, *pos, *U)) {encontre = 1;	}
								
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
	}
	return pos;

}

bool unit_Manager::isFreeOfBuildingsRightAndLeft(UnitType* U, TilePosition* t){

	int x = 1;
	int y = 0;
	bool free = true;

//reviso a la izquierda de la construccion
	while ((x<=2) && (free == true)){
		if (t->x()-x>0){
			y=0;
			while((y < U->tileHeight()) && (free == true)){
				std::set< Unit* > unidadesEnTile = Broodwar->unitsOnTile(t->x()-x, t->y()+y);
				std::set<Unit*>::iterator It1;
				It1 = unidadesEnTile.begin();
				while(It1 != unidadesEnTile.end()){
					if((*It1)->getType().isBuilding()) return false;
					else {It1++;}
				}
				y++;
			}
		}
		x++;
	}
//reviso a la derecha de la construccion
	x=0;
	while (x<2){
		if (t->x()+ U->tileWidth()+x < Broodwar->mapWidth()){
			y=0;
			while(y < U->tileHeight()){
				
				if (Broodwar->isBuildable(t->x()+ U->tileWidth()+x, t->y()+y)){
					std::set< Unit* > unidadesEnTile = Broodwar->unitsOnTile(t->x()+ U->tileWidth()+x, t->y()+y);
					std::set<Unit*>::iterator It1;
					It1 = unidadesEnTile.begin();
					while(It1 != unidadesEnTile.end()){
						if((*It1)->getType().isBuilding()) {
							return false;
						}
						else{
							It1++;
						}
					}
				}
				else{
					return false;
				}
				y++;
			}
		}
		else{
			return false;
		}
		x++;
	}


	
	return true;
}


void unit_Manager::setGoals(int goals[34]){
	for (int i=0; i<34; i++){
		goalCantUnidades[i] = goals[i];
	}
}

void unit_Manager::setResearchs(int researchs[Utilidades::maxResearch]){
	for (int i=0; i < Utilidades::maxResearch; i++){
		goalResearch[i] = researchs[i];
	}
}

void unit_Manager::asignarUnidadACompania(Unit* unit){
	
	if (unit->getType().getID() == Utilidades::ID_MARINE){

		if ((grupoB1 != NULL) && (grupoB1->faltanMarines()))
			grupoB1->agregarUnidad(unit);
		else if ((grupoB2 != NULL) && (grupoB2->faltanMarines()))
			grupoB2->agregarUnidad(unit);
		else{
			if ((cantUnidades[Utilidades::INDEX_GOAL_MARINE] > 16) && (Fox != NULL) && (Fox->faltanMarines()))
				Fox->asignarUnidad(unit);
			else {
			//	if ((Easy != NULL)&& (Charlie!=NULL))
			//		if((turnoAsignacionMarines == 1)&&(Charlie->faltanMarines())){
			//			Charlie->asignarUnidad(unit);
			//			turnoAsignacionMarines= 0;
			//		}
			//		else{
						Easy->asignarUnidad(unit);
			//			turnoAsignacionMarines= 1;
			//		}
			}
		}
	}
	else if (unit->getType().getID() == Utilidades::ID_MEDIC){
		if ((Fox != NULL) && (Fox->faltanMedics()))
			Fox->asignarUnidad(unit);
		else {
		//	if ((Easy != NULL)&& (Charlie!=NULL))
		//		if((turnoAsignacionMedics == 1)&&(Charlie->faltanMedicos())){
		//				Charlie->asignarUnidad(unit);
		//				turnoAsignacionMarines= 0;
		//			}
		//		else{
					Easy->asignarUnidad(unit);
		//			turnoAsignacionMarines= 1;
		//		}
		}
	}
	else if (unit->getType().getID() == Utilidades::ID_FIREBAT){
		if (Easy != NULL) 
			Easy->asignarUnidad(unit);
	}
	else if (unit->getType().getID() == Utilidades::ID_TANKSIEGE){
		if ((grupoB1 != NULL) && (grupoB1->faltanTanques()))
			grupoB1->agregarUnidad(unit);
		else if ((grupoB2 != NULL) && (grupoB2->faltanTanques()))
			grupoB2->agregarUnidad(unit);
		else {
		//	if ((Easy != NULL)&& (Charlie != NULL))
		//		if((turnoAsignacionTanks == 1)&&(Charlie->faltanTanks())){
		//			Charlie->asignarUnidad(unit);
		//			turnoAsignacionTanks= 0;
		//		}
		//		else{
					Easy->asignarUnidad(unit);
		//			turnoAsignacionTanks= 1;
		//		}
		}
	}
	else if (unit->getType().getID() == Utilidades::ID_GOLIATH){
	//	if ((Easy != NULL) &&(Charlie != NULL))
	//		if((turnoAsignacionGoliaths == 1)&&(Charlie->faltanGoliaths())){
	//			Charlie->asignarUnidad(unit);
	//			turnoAsignacionGoliaths= 0;
	//		}
	//		else{
				Easy->asignarUnidad(unit);
	//			turnoAsignacionGoliaths= 1;
	//		}
	}
	else if (unit->getType().getID() == Utilidades::ID_SCIENCE_VESSEL){
		if (Easy != NULL) 
			Easy->asignarUnidad(unit);
	}
	else if (unit->getType().getID() == Utilidades::ID_DROPSHIP){
		if (ct != NULL)
			ct->asignarUnidad(unit);
	}
	else if (unit->getType().getID() == Utilidades::ID_WRAITH){
		if (ct != NULL)
			ct->asignarUnidad(unit);
	}
	else if (unit->getType().getID() == Utilidades::ID_GHOST){
		if ((Fox != NULL) && (Fox->faltanGhosts()))
			Fox->asignarUnidad(unit);
	}

}


void unit_Manager::repararUnidad(Unit *u){

	if ((u != NULL) && (u->getType().getID() != 7)){
		if ((reparador1 != NULL) && (reparador1->exists()) && (!reparador1->isRepairing())){
			// si el reparador esta seteado lo utiliza
			if ((u->getType().isMechanical()) || (u->getType().isBuilding())){
				if ((u->exists())&& reparador1->exists()){
					reparador1->repair(u);
				}
			}
			else
				Broodwar->printf("No se puede reparar esa unidad");
		}
		else if ((reparador2 != NULL) && (reparador2->exists()) &&(!reparador2->isRepairing())){
			// si el reparador esta seteado lo utiliza
			if ((u->getType().isMechanical()) || (u->getType().isBuilding())){
				if ((u->exists())&& (reparador2->exists())){
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

			if ((u->isCloaked() || u->isBurrowed()) && (!u->isDetected()) && (cantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION] > 0)){
				Unit *comsat = getUnit(Utilidades::ID_COMSAT_STATION);

				if ((comsat != NULL) && (comsat->exists()) && (comsat->getEnergy() >= 50)){
					comsat->useTech(TechTypes::Scanner_Sweep, u->getPosition());
				}
			}

			atacado = u->getOrderTarget();
			if ((atacado != NULL) && (atacado->getType().getID() == Utilidades::ID_BUNKER)){
				// un bunker esta siendo atacado, mando al SCV a repararlo
				repararUnidad(atacado);
				if (Easy != NULL) 
					Easy->atacar(u);

				break;
			}

			atacado = u->getTarget();
			if ((atacado != NULL) && (atacado->getType().getID() == Utilidades::ID_BUNKER)){
				// un bunker esta siendo atacado, mando al SCV a repararlo
				repararUnidad(atacado);

				if (Easy != NULL)
					Easy->atacar(u);

				break;
			}
		}
	}
}


void unit_Manager::nuevaUnidadConstruccion(Unit *u){

	if (u != NULL){
		unidadesEnConstruccion.push_front(u);

		if ((grupoB1 != NULL) && (grupoB1->getCantBunkers() < 3) && (u->getType().getID() == Utilidades::ID_BUNKER))
			grupoB1->agregarUnidad(u);
		else if ((grupoB2 != NULL) && (grupoB2->getCantBunkers() < 3) && (u->getType().getID() == Utilidades::ID_BUNKER))
			grupoB2->agregarUnidad(u);

		if ((grupoB1 != NULL) && (grupoB1->getCantMisileTurrets() < 2) && (u->getType().getID() == Utilidades::ID_MISSILE_TURRET))
			grupoB1->agregarUnidad(u);
		else if ((anti != NULL) && (anti->faltanMisileTurrets()) && (u->getType().getID() == Utilidades::ID_MISSILE_TURRET))
			anti->agregarUnidad(u);
		else if ((grupoB2 != NULL) && (grupoB2->getCantMisileTurrets() < 2) && (u->getType().getID() == Utilidades::ID_MISSILE_TURRET))
			grupoB2->agregarUnidad(u);
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
				if ((cantUnidades[Utilidades::INDEX_GOAL_ACADEMY] + 1) > goalCantUnidades[Utilidades::INDEX_GOAL_ACADEMY]){
					u->getBuildUnit()->haltConstruction();
					u->getBuildUnit()->stop();
					u->cancelConstruction();
				}
				else
					cantUnidades[Utilidades::INDEX_GOAL_ACADEMY]++;

				break;
			case Utilidades::ID_BARRACK:
				if ((cantUnidades[Utilidades::INDEX_GOAL_BARRACK] + 1) > goalCantUnidades[Utilidades::INDEX_GOAL_BARRACK]){
					u->getBuildUnit()->haltConstruction();
					u->getBuildUnit()->stop();
					u->cancelConstruction();
				}
				else
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
				if ((cantUnidades[Utilidades::INDEX_GOAL_FACTORY] + 1) > goalCantUnidades[Utilidades::INDEX_GOAL_FACTORY]){
					u->getBuildUnit()->haltConstruction();
					u->getBuildUnit()->stop();
					u->cancelConstruction();
				}
				else
					cantUnidades[Utilidades::INDEX_GOAL_FACTORY]++;

				break;	
			case Utilidades::ID_MACHINESHOP:
				cantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP]++;
				break;
			case Utilidades::ID_TANKSIEGE:
				cantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE]++;
				asignarUnidadACompania(u);
				break;
			case Utilidades::ID_ENGINEERING_BAY:
				if ((cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY] + 1) > goalCantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY]){
					u->getBuildUnit()->haltConstruction();
					u->getBuildUnit()->stop();
					u->cancelConstruction();
				}
				else
					cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY]++;

				break;
			case Utilidades::ID_ARMORY:
				if ((cantUnidades[Utilidades::INDEX_GOAL_ARMORY] + 1) > goalCantUnidades[Utilidades::INDEX_GOAL_ARMORY]){
					u->getBuildUnit()->haltConstruction();
					u->getBuildUnit()->stop();
					u->cancelConstruction();
				}
				else
					cantUnidades[Utilidades::INDEX_GOAL_ARMORY]++;

				break;
			case Utilidades::ID_STARPORT:
				cantUnidades[Utilidades::INDEX_GOAL_STARPORT]++;
				break;
			case Utilidades::ID_COMSAT_STATION:
				cantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION]++;
				break;
			case Utilidades::ID_MISSILE_TURRET:
				cantUnidades[Utilidades::INDEX_GOAL_MISSILE_TURRET]++;
				break;			
			case Utilidades::ID_GOLIATH:
				cantUnidades[Utilidades::INDEX_GOAL_GOLIATH]++;
				asignarUnidadACompania(u);
				break;
			case Utilidades::ID_CONTROL_TOWER:
				cantUnidades[Utilidades::INDEX_GOAL_CONTROL_TOWER]++;
				break;
			case Utilidades::ID_DROPSHIP:
				cantUnidades[Utilidades::INDEX_GOAL_DROPSHIP]++;
				asignarUnidadACompania(u);
				break;
			case Utilidades::ID_SCIENCE_FACILITY:
				cantUnidades[Utilidades::INDEX_GOAL_SCIENCE_FACILITY]++;
				break;
			case Utilidades::ID_SCIENCE_VESSEL:
				cantUnidades[Utilidades::INDEX_GOAL_SCIENCE_VESSEL]++;
				asignarUnidadACompania(u);
				break;
			case Utilidades::ID_WRAITH:
				cantUnidades[Utilidades::INDEX_GOAL_WRAITH]++;
				asignarUnidadACompania(u);
				break;
			case Utilidades::ID_COVERT_OPS:
				cantUnidades[Utilidades::INDEX_GOAL_COVERT_OPS]++;
				break;
			case Utilidades::ID_GHOST:
				cantUnidades[Utilidades::INDEX_GOAL_GHOST]++;
				asignarUnidadACompania(u);
				break;
			case Utilidades::ID_COMMANDCENTER:
				cantUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER]++;
				break;
			case Utilidades::ID_VULTURE:
				cantUnidades[Utilidades::INDEX_GOAL_VULTURE]++;
				magallanes->setExplorador(u);
				Broodwar->printf("asigné a magallanes");

				break;
		}
	}

	resetBuildingSemaphore();
	nuevaUnidadConstruccion(u);
}

void unit_Manager::onUnitDestroy(Unit *u){
	
	// TODO: si un edificio por ejemplo la academia terran es destruido, y se tenia solo un edificio de ese tipo,
	// se pierden todas las mejoras investigadas anteriormente, y se deberan investigar de nuevo
	// Esto se debera actualizar solamente en el metodo onUnitDestroy de la clase unit_Manager

	if (grupoB1 != NULL)
		grupoB1->onUnitDestroy(u);

	if (grupoB2 != NULL)
		grupoB2->onUnitDestroy(u);

	if(Broodwar->self()== u->getPlayer()){

		switch (u->getType().getID()){
			case Utilidades::ID_ACADEMY:
				cantUnidades[Utilidades::INDEX_GOAL_ACADEMY]--;

				if (cantUnidades[Utilidades::INDEX_GOAL_ACADEMY] == 0){
					researchDone[Utilidades::INDEX_GOAL_U238] = false;
					researchDone[Utilidades::INDEX_GOAL_STIMPACK] = false;
				}
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

				if (cantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP] == 0){
					researchDone[Utilidades::INDEX_GOAL_TANK_SIEGE_MODE] = false;
				}
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
			case Utilidades::ID_ARMORY:
				cantUnidades[Utilidades::INDEX_GOAL_ARMORY]--;
				break;
			case Utilidades::ID_STARPORT:
				cantUnidades[Utilidades::INDEX_GOAL_STARPORT]--;
				break;
			case Utilidades::ID_COMSAT_STATION:
				cantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION]--;
				break;
			case Utilidades::ID_MISSILE_TURRET:
				cantUnidades[Utilidades::INDEX_GOAL_MISSILE_TURRET]--;
				break;
			case Utilidades::ID_GOLIATH:
				cantUnidades[Utilidades::INDEX_GOAL_GOLIATH]--;
				break;
			case Utilidades::ID_CONTROL_TOWER:
				cantUnidades[Utilidades::INDEX_GOAL_CONTROL_TOWER]--;
				break;
			case Utilidades::ID_DROPSHIP:
				cantUnidades[Utilidades::INDEX_GOAL_DROPSHIP]--;
				break;
			case Utilidades::ID_SCIENCE_FACILITY:
				cantUnidades[Utilidades::INDEX_GOAL_SCIENCE_FACILITY]--;
				break;
			case Utilidades::ID_SCIENCE_VESSEL:
				cantUnidades[Utilidades::INDEX_GOAL_SCIENCE_VESSEL]--;
				break;
			case Utilidades::ID_WRAITH:
				cantUnidades[Utilidades::INDEX_GOAL_WRAITH]--;
				break;
			case Utilidades::ID_COVERT_OPS:
				cantUnidades[Utilidades::INDEX_GOAL_COVERT_OPS]--;
				break;
			case Utilidades::ID_GHOST:
				cantUnidades[Utilidades::INDEX_GOAL_GHOST]--;
				break;
			case Utilidades::ID_COMMANDCENTER:
				cantUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER]--;
				break;
			case Utilidades::ID_VULTURE:
				cantUnidades[Utilidades::INDEX_GOAL_VULTURE]--;
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


bool* unit_Manager::getResearchsDone(){
	return researchDone;
}


void unit_Manager::onNukeDetect(Position p){

	//-- busca el fantasma que este mas cerca del lugar donde impactara el misil nuclear y manda a la compañia defensiva a que lo ataque
	if (!Broodwar->enemy()->getUnits().empty()){
		std::set<Unit*>::const_iterator It;
		Unit *masCercana = NULL;

		It = Broodwar->enemy()->getUnits().begin();
		while (It != Broodwar->enemy()->getUnits().end()){
			if (((*It) != NULL) && ((*It)->exists()) && ((*It)->getType().getID() == Utilidades::ID_GHOST)){
				//Broodwar->setScreenPosition((*It)->getPosition());

				if (masCercana == NULL)
					masCercana = (*It);
				else if (p.getApproxDistance((*It)->getPosition()) < p.getApproxDistance(masCercana->getPosition())){
					masCercana = (*It);
				}
			}

			It++;
		}

		if (masCercana != NULL){
			Unit *detector = getUnit(Utilidades::ID_COMSAT_STATION);

			// si el fantasma que se encontro esta a menor o igual distancia que el rango de vision del fantasma deberia ser
			// el que lanzo el ataque nuclear, de otra manera no es el fantasma que se esta buscando
			if ((int)p.getDistance(masCercana->getPosition()) <= (masCercana->getType().sightRange() + 64)){
				// sumo 64 al sightRange para tener en cuenta la mejora de vision del fantasma
				if ((detector != NULL) && (detector->exists()) && (detector->getEnergy() >= 50) && (!masCercana->isDetected()))
					detector->useTech(TechTypes::Scanner_Sweep, masCercana->getPosition());

				//Broodwar->printf("Fantasma detectado, atacando...");

				if (Fox != NULL) 
					Fox->atacar(masCercana->getPosition());
			}
			else{
				Unit *detector = getUnit(Utilidades::ID_COMSAT_STATION);

				if ((detector != NULL) && (detector->exists()) && (detector->getEnergy() >= 50))
					detector->useTech(TechTypes::Scanner_Sweep, p);

				//Broodwar->printf("Fantasma detectado, movimiento de ataque...");

				if (Fox != NULL) 
					Fox->atacar(p);
			}
		}
		else{
			// si no encontro ningun fantasma, quiere decir que el fantasma que lanzo el ataque nuclear esta en una zona oculta
			// por el fog of war, entonces muevo la compañia a la zona de impacto y confio en que puedan detectar a la unidad enemiga
			Unit *detector = getUnit(Utilidades::ID_COMSAT_STATION);

			if ((detector != NULL) && (detector->exists()) && (detector->getEnergy() >= 50))
				detector->useTech(TechTypes::Scanner_Sweep, p);

			//Broodwar->printf("Fantasma detectado, movimiento de ataque...");

			if (Fox != NULL) 
				Fox->atacar(p);
		}
	}
}


void unit_Manager::onUnitShow(Unit *u){
	// calcula la posicion donde esta ubicada la base enemiga, solamente realiza el calculo cuando la primer edificacion enemiga se hace visible
	if ((u != NULL) && (u->exists()) && (Broodwar->self()->isEnemy(u->getPlayer())) && (u->getType().isBuilding()) && (primerConstruccionDescubierta) && (analisisListo)){
		primerConstruccionDescubierta = false;
		bool encontre = false;

		std::set<Region*>::const_iterator It = BWTA::getRegions().begin();

		while ((It != BWTA::getRegions().end()) && (!encontre)){
			// obtiene la region donde esta ubicada la construccion enemiga
			if ((*It)->getPolygon().isInside(u->getPosition())){

				regionBaseEnemiga = (*It);
				// obtiene la posicion del centro de comando de esa region
				if (!(*It)->getBaseLocations().empty()){
					baseEnemiga = new Position((*(*It)->getBaseLocations().begin())->getPosition().x(), (*(*It)->getBaseLocations().begin())->getPosition().y());

					if (Easy != NULL)
						Easy->setBasesEnemigas(new TilePosition(regionBaseEnemiga->getCenter()));
				}
				
				// crea la compañia de transporte
				//ct = new CompaniaTransporte(baseEnemiga, regionBaseEnemiga, Easy);

				encontre = true;
			}
			It++;
		}
	}
	else if ((u != NULL) && (u->exists()) && (Broodwar->self()->isEnemy(u->getPlayer())) && ((u->isCloaked()) || (u->isBurrowed())) && (!u->isDetected())){
		// si la unidad que genero el evento es "invisible" o esta oculta debajo de la tierra realiza un scan de la posicion de esa unidad con el comsat station
		if (cantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION] > 0){
			Unit *detector = getUnit(Utilidades::ID_COMSAT_STATION);

			if ((detector != NULL) && (detector->exists()) && (detector->getEnergy() > 50))
				detector->useTech(TechTypes::Scanner_Sweep, u->getPosition());
		}
	}

}


void unit_Manager::buscarUnidadesOcultas(){
	
	if (!Broodwar->enemy()->getUnits().empty()){
		std::set<Unit*>::const_iterator It;
		Unit *detector = getUnit(Utilidades::ID_COMSAT_STATION);

		if ((detector != NULL) && (detector->exists()) && (detector->getEnergy() >= 50)){
		
			It = Broodwar->enemy()->getUnits().begin();
			while (It != Broodwar->enemy()->getUnits().end()){
				if (((*It) != NULL) && ((*It)->exists()) && (((*It)->isBurrowed()) || ((*It)->isCloaked())) && ((*It)->isVisible()) && (!(*It)->isDetected())){
					if (detector->getEnergy() >= 50){
						detector->useTech(TechTypes::Scanner_Sweep, (*It)->getPosition());
					}
				}

				It++;
			}
		}
	}
}


void unit_Manager::setEstadoActual(int estado){
	estadoActual = estado;
}