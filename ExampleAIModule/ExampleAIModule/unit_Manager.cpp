#include "unit_Manager.h"
#include "GrupoAntiaereo.h"

GrupoAntiaereo *anti = NULL;
int goalLimiteGeiser = 1;
/*int goalLimiteSCV = 8;
int goalLimiteBarracas = 1;*/

//int tiempoProxFinalizacion = 0; // mantiene el tiempo hasta la proxima finalizacion de la construccion o entrenamiento de una unidad para evitar ejecutar en todos los frames el metodo controlarFinalizacion
//int contProxFinalizacion = 0; // contador que se incrementa en cada frame, para controlar la finalizacion de una construccion o entrenamiento

unit_Manager::unit_Manager()
{
	primerScan = true;
	SCVgatheringMinerals = 0;
	SCVgatheringGas = 0;
	ultimaFinalizada = NULL;
	buildingSemaphore = 0;

	Easy = new compania(Colors::Red);
	Fox = new CompaniaDefensiva(Colors::Yellow); // esta compañia se encargara de atacar a los fantasmas que ataquen la base

	magallanes = new Scout(getWorker()); // revisar como genera las posiciones a partir de la 4ta posicion a explorar pq se rompe

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
	
	grupoB1 = NULL;
	grupoB2 = NULL;

	//-- --//
	//Position *pos = new Position(enemigo->getStartLocation().x() / 32, enemigo->getStartLocation().y() / 32);
	//Broodwar->pingMinimap(*pos);
	//Broodwar->pingMinimap(4,4);
	//Broodwar->printf("Hice ping en el minimap");
	//delete pos;

	
}

void unit_Manager::executeActions(AnalizadorTerreno *analizador){

	// Crea un nuevo grupo de bunkers

	if (analizador->analisisListo()){
		if (grupoB1 == NULL){
			grupoB1 = new GrupoBunkers(analizador, analizador->obtenerChokepoint() ,analizador->regionInicial());
			//Broodwar->printf("Angulo B1: %d", grupoB1->getAngulo());
		}
		else
			grupoB1->onFrame();

		//-- MEJORAR ESTO PARA EL MAPA DESTINATION QUE TENGO VARIOS CHOKEPOINTS CERCA Y ES MEDIO RANDOM LA ELECCION DEL CHOKE A DEFENDER
		/*if (grupoB2 == NULL){
			if (analizador->obtenerChokepoint()->getRegions().first != analizador->regionInicial()){
				std::set<Chokepoint*>::const_iterator It = analizador->obtenerChokepoint()->getRegions().first->getChokepoints().begin();

				while (It != analizador->obtenerChokepoint()->getRegions().first->getChokepoints().end()){
					if ((*It) != analizador->obtenerChokepoint()){
						//Graficos::dibujarCuadro(new TilePosition((*It)->getCenter().x() / 32, (*It)->getCenter().y() / 32), 1, 1);
						grupoB2 = new GrupoBunkers(analizador, *It, analizador->obtenerChokepoint()->getRegions().first);
					}
					It++;
				}
			}
			else{
				std::set<Chokepoint*>::const_iterator It = analizador->obtenerChokepoint()->getRegions().second->getChokepoints().begin();

				while (It != analizador->obtenerChokepoint()->getRegions().second->getChokepoints().end()){
					if ((*It) != analizador->obtenerChokepoint()){
						grupoB2 = new GrupoBunkers(analizador, *It, analizador->obtenerChokepoint()->getRegions().second);
						//Graficos::dibujarCuadro(new TilePosition((*It)->getCenter().x() / 32, (*It)->getCenter().y() / 32), 1, 1);
					}
					It++;
				}
			}

			Broodwar->printf("Angulo B2: %d", grupoB2->getAngulo());
		}
		else
			grupoB2->onFrame();*/


		if (anti == NULL)
			anti = new GrupoAntiaereo(analizador->regionInicial());
		else
			anti->onFrame();
	}

	magallanes->explorar(); // manda al scout a explorar el mapa

	
	//-- busca unidades ocultas cerca del grupo de bunkers

	/*if (Broodwar->getFrameCount() % 300 == 0){
		if (cantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION] > 0){
			Unit *z = getUnit(Utilidades::ID_COMSAT_STATION);

			if ((z != NULL) && (z->exists())){
				Broodwar->printf("Energia comsat: %d", z->getEnergy());

				if (primerScan){
					if (z->getEnergy() >= 50){
						if (grupoB1 != NULL){
							z->useTech(TechTypes::Scanner_Sweep, grupoB1->getChoke()->getCenter());
							primerScan = false;
						}
						else if (grupoB2 != NULL){
							z->useTech(TechTypes::Scanner_Sweep, grupoB2->getChoke()->getCenter());
							primerScan = false;
						}
					}
				}
				else{
					if (z->getEnergy() > 150){
						if (grupoB1 != NULL)
							z->useTech(TechTypes::Scanner_Sweep, grupoB1->getChoke()->getCenter());
						else if (grupoB2 != NULL)
							z->useTech(TechTypes::Scanner_Sweep, grupoB2->getChoke()->getCenter());
					}
				}
			}
		}
	}*/

	//--

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

	// error raro numero 1: puse un if y empezo a funcionar magicamente...
	if (Easy != NULL)
		Easy->onFrame();
	else
		Broodwar->printf("ERROR: Easy es NULL");

	Fox->onFrame();


	// verifica si se termino de construir alguna unidad en este frame
	//ultimaFinalizada = controlarFinalizacion();

	verificarBunkers(); // verifica daños en los bunkers 

	//-- seccion que imprime algunos graficos en pantalla, solo a fines de debugging
	if ((analizador->analisisListo()) && (grupoB1 != NULL)){

		TilePosition *t111 = NULL;
		TilePosition *t222 = NULL;
		TilePosition *t333 = NULL;
		
		t111 = grupoB1->posicionNuevoBunker();
		
		if (t111 != NULL){
			Graficos::dibujarCuadro(t111, 3, 2);
			Broodwar->drawLine(CoordinateType::Map, analizador->obtenerCentroChokepoint()->x(), analizador->obtenerCentroChokepoint()->y(), t111->x() * 32 + 16, t111->y() * 32 + 16, Colors::Yellow);
		}

		t222 = grupoB1->posicionNuevaTorreta();
		if (t222 != NULL){
			Graficos::dibujarCuadro(t222, 2, 2);
			//Broodwar->drawLine(CoordinateType::Map, analizador->obtenerCentroChokepoint()->x(), analizador->obtenerCentroChokepoint()->y(), t111->x() * 32 + 16, t111->y() * 32 + 16, Colors::Yellow);
		}

		t333 = grupoB1->posicionNuevoTanque();		
		if (t333 != NULL){
			Graficos::dibujarCuadro(t333, 1, 1);
			//Broodwar->drawLine(CoordinateType::Map, analizador->obtenerCentroChokepoint()->x(), analizador->obtenerCentroChokepoint()->y(), t111->x() * 32 + 16, t111->y() * 32 + 16, Colors::Yellow);
			delete t333;
		}
		/*else
			Broodwar->printf("posicion nuevo tanque es NULL");*/
		
	}

	if ((analizador->analisisListo()) && (grupoB2 != NULL)){

		TilePosition *t111 = NULL;
		TilePosition *t222 = NULL;
		TilePosition *t333 = NULL;
		
		t111 = grupoB2->posicionNuevoBunker();
		
		if (t111 != NULL){
			Graficos::dibujarCuadro(t111, 3, 2);
			Broodwar->drawLine(CoordinateType::Map, grupoB2->getChoke()->getCenter().x(), grupoB2->getChoke()->getCenter().y(), t111->x() * 32 + 16, t111->y() * 32 + 16, Colors::Yellow);
		}

		t222 = grupoB2->posicionNuevaTorreta();
		if (t222 != NULL){
			Graficos::dibujarCuadro(t222, 2, 2);
			//Broodwar->drawLine(CoordinateType::Map, analizador->obtenerCentroChokepoint()->x(), analizador->obtenerCentroChokepoint()->y(), t111->x() * 32 + 16, t111->y() * 32 + 16, Colors::Yellow);
		}

		t333 = grupoB2->posicionNuevoTanque();		
		if (t333 != NULL){
			Graficos::dibujarCuadro(t333, 1, 1);
			//Broodwar->drawLine(CoordinateType::Map, analizador->obtenerCentroChokepoint()->x(), analizador->obtenerCentroChokepoint()->y(), t111->x() * 32 + 16, t111->y() * 32 + 16, Colors::Yellow);
			delete t333;
		}
		/*else
			Broodwar->printf("posicion nuevo tanque es NULL");*/
		
	}


	//--
	Unit *k = getUnit(Utilidades::ID_ACADEMY);
	if ((k != NULL) && (k->getUpgradeLevel(UpgradeTypes::U_238_Shells))){
		Broodwar->printf("U238 investigadouuu");
	}
	//--

	// ---------------------------------------------------------------------------
	//--						CODIGO DE REPARACION DE UNIDADES

	if (Broodwar->getFrameCount() % 50 == 0){
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
			// si es una edificacion o es una unidad mecanica, verifica si esta dañada y la repara
			if ((Broodwar->self()->minerals() > 70) && (((*i)->getType().isBuilding()) || ((*i)->getType().isMechanical())) && ((*i)->isCompleted()) && ((*i)->getType().maxHitPoints() > (*i)->getHitPoints())){
				repararUnidad(*i);
			}
			else if ((*i)->getType().isBuilding() && (!(*i)->isCompleted()) && (!(*i)->isBeingConstructed())){
				// si una unidad es una edificacion, no esta completada y no esta siendo construida por nadie (es decir quedo su construccion incompleta) manda a un SCV a finalizar su construccion
				finalizarConstruccion(*i);
			}
		}
	}

	// ---------------------------------------------------------------------------
	//--					CODIGO DE GESTION DE RECOLECCION DE RECURSOS

	if (frameLatency >= 150){
		int SCVgatheringCristal= 0, SCVgatheringGas = 0;
		Unit* trabajador;
		frameLatency=0;
		buildingSemaphore=0;
		
		
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

		//if ((Broodwar->self()->completedUnitCount(*(new UnitType(Utilidades::ID_REFINERY)))>0) && (SCVgatheringCristal+SCVgatheringGas>10)){
		if ((cantUnidades[Utilidades::ID_REFINERY] > 0) && (SCVgatheringCristal+SCVgatheringGas > 8/*10*/)){
			if (SCVgatheringGas < 3/*4*/){
				for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
				{
					if (SCVgatheringGas < 3/*4*/){
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

	// ---------------------------------------------------------------------------
	//--						CODIGO PARA CONSTRUCCION DE UNIDADES

	//-- COMMAND CENTER
	if ((cantUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER] < goalCantUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER]) && (Broodwar->self()->minerals() > 400)){
		UnitType* building = new UnitType(Utilidades::ID_COMMANDCENTER);
		TilePosition* posB = getTilePositionAviable(building);
		
		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_DEPOT);
			delete posB;
		}
		delete building;
	}

	//-- COMSAT STATION (ADD-ON DE COMMAND CENTER)
	if((cantUnidades[Utilidades::INDEX_GOAL_COMMANDCENTER] > 0) && (Broodwar->self()->minerals() > 50) && (Broodwar->self()->gas() > 50) && (cantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION] < goalCantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION]) /*&& (buildingSemaphore == 0)*/ && (cantUnidades[Utilidades::ID_ACADEMY] > 0)){
		//Broodwar->printf("Entra a comsat");
		buildUnitAddOn(Utilidades::ID_COMSAT_STATION);
	}
	/*else
		Broodwar->printf("No entra a comsat");*/

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

	//-- GOLIATHS
	if(cantUnidades[Utilidades::INDEX_GOAL_ARMORY] && (cantUnidades[Utilidades::INDEX_GOAL_GOLIATH] < goalCantUnidades[Utilidades::INDEX_GOAL_GOLIATH]) && (Broodwar->self()->minerals()>= 100) && (Broodwar->self()->gas()>= 50)) {
		trainGoliath();
	}

	//-- SIEGE TANKS
	if(cantUnidades[Utilidades::INDEX_GOAL_MACHINESHOP] && (cantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE] < goalCantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE]) && (Broodwar->self()->minerals()>= 150) && (Broodwar->self()->gas()>= 100)) {
		trainTankSiege();
	}


	//-- REFINERY
	if((Broodwar->self()->minerals()>200) && (cantUnidades[Utilidades::INDEX_GOAL_REFINERY] < goalCantUnidades[Utilidades::INDEX_GOAL_REFINERY]) && (buildingSemaphore == 0)){
		TilePosition* pos = NULL;
		makeRefinery(pos);
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
	if((grupoB1 != NULL) && grupoB1->faltanBunkers() && (Broodwar->self()->minerals() > 150) && (cantUnidades[Utilidades::INDEX_GOAL_BUNKER] < goalCantUnidades[Utilidades::INDEX_GOAL_BUNKER]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_BUNKER);
		TilePosition *posB = NULL;

		posB = grupoB1->posicionNuevoBunker();

		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_BUNKER);

			delete posB;
		}
		else
			Broodwar->printf("ERROR: No encuentro posicion para construir el bunker");
		delete building;
	}//-- NUEVO
	else if((grupoB2 != NULL) && grupoB2->faltanBunkers() && (Broodwar->self()->minerals() > 150) && (cantUnidades[Utilidades::INDEX_GOAL_BUNKER] < goalCantUnidades[Utilidades::INDEX_GOAL_BUNKER]) && (buildingSemaphore == 0)){
		UnitType* building = new UnitType(Utilidades::ID_BUNKER);
		TilePosition *posB = NULL;

		posB = grupoB2->posicionNuevoBunker();

		if (posB != NULL){
			buildUnit(posB, Utilidades::ID_BUNKER);

			delete posB;
		}
		else
			Broodwar->printf("ERROR: No encuentro posicion para construir el bunker");
		delete building;
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
		UnitType* building = new UnitType(Utilidades::ID_MISSILE_TURRET);
		TilePosition *posB = NULL;

		if((grupoB1 != NULL) && (grupoB1->faltanMisileTurrets()) && (Broodwar->self()->minerals() > 100) && (buildingSemaphore == 0)){
			posB = grupoB1->posicionNuevaTorreta();

			if (posB != NULL){
				buildUnit(posB, Utilidades::ID_MISSILE_TURRET);

				delete posB;
			}
		}
		else if ((anti != NULL) && (!grupoB1->faltanTanques()) && (anti->faltanMisileTurrets()) && (Broodwar->self()->minerals() > 100) && (buildingSemaphore == 0)){
			posB = anti->getPosicionMisileTurret();

			if (posB != NULL){
				buildUnit(posB, Utilidades::ID_MISSILE_TURRET);

				delete posB;
			}
		}
		else if((grupoB2 != NULL) && (!grupoB1->faltanTanques()) && (grupoB2->faltanMisileTurrets()) && (Broodwar->self()->minerals() > 100) && (buildingSemaphore == 0)){
			posB = grupoB2->posicionNuevaTorreta();

			if (posB != NULL){
				buildUnit(posB, Utilidades::ID_MISSILE_TURRET);

				delete posB;
			}
		}

		delete building;
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

	//-- VEHICLE WEAPONS LEVEL 1
	if (!researchDone[Utilidades::INDEX_GOAL_VEHICLE_WEAPONS_LVL1]){
		/*Broodwar->printf("Cantidad armory: %d", cantUnidades[Utilidades::INDEX_GOAL_ARMORY]);
		Broodwar->printf("minerales: %d", Broodwar->self()->minerals());
		Broodwar->printf("gas: %d", Broodwar->self()->gas());
		Broodwar->printf("goal research armamento vehiculos: %d", goalResearch[Utilidades::INDEX_GOAL_VEHICLE_WEAPONS_LVL1]);
*/

		// mejora de armamento de vehiculos nivel 1 (se investiga en armory
		if ((cantUnidades[Utilidades::INDEX_GOAL_ARMORY] > 0) && (Broodwar->self()->minerals() > 100) && (Broodwar->self()->gas() > 100) && (goalResearch[Utilidades::INDEX_GOAL_VEHICLE_WEAPONS_LVL1] == 1)){
			//Broodwar->printf("Intenta mejorar armamento vehiculos");
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

	//--						FIN CODIGO PARA CONSTRUCCION DE UNIDADES
	// ---------------------------------------------------------------------------
}


unit_Manager::~unit_Manager(void)
{
}


void unit_Manager::buildUnit(TilePosition *pos, int id){

	Unit* trabajador;
	UnitType *tipo = new UnitType(id);

	Position *p = NULL; 
	TilePosition *t = NULL;

	//if (id == Utilidades::ID_BUNKER) Broodwar->printf("Intento construir bunker");

	if ((Broodwar->self()->minerals()>tipo->mineralPrice())&&(Broodwar->self()->gas()>=tipo->gasPrice())){
		trabajador = getWorker();
		
		if ((trabajador!=NULL) && (trabajador->exists()) && (!trabajador->isConstructing()) && (trabajador->isCompleted())) {
			if (Broodwar->canBuildHere(trabajador, *pos, *tipo)){
				
				// mejorar esto, calcular una buena posicion para mover la compañia
				
				// verifica si hay unidades en cada tile que ocupara la construccion, y si las hay, las mueve a otra posicion
				for (int x = 0; x < tipo->tileWidth(); x++){
					for (int y = 0; y < tipo->tileHeight(); y++){
						// referencia al tile actual
						t = new TilePosition(pos->x() + x, pos->y() + y);
						
						// si hay unidades en el tile actual, pregunta a que compañia pertenecen para moverlas a otra posicion
						if (Broodwar->unitsOnTile(t->x(), t->y()).size() > 0){
							// posicion hacia donde mover las unidades
							p = new Position((pos->x() + tipo->tileWidth() + 1) * 32, (pos->y() + tipo->tileHeight() + 1) * 32);

							// verifica a que compañia pertenecen las unidades
							std::set<Unit*>::iterator It1 = Broodwar->unitsOnTile(t->x(), t->y()).begin();
							if (Easy->pertenece(*It1)){
								Easy->moverCompania(*p);
							}
							else if ((grupoB1 != NULL) && (grupoB1->perteneceMarine(*It1))){
								grupoB1->moverSoldadosPosEncuentro();
							}
							else{
								// mueve cada unidad en el tile a otra posicion (solamente si la unidad en cuestion no es una construccion fija)
								while (It1 != Broodwar->unitsOnTile(t->x(), t->y()).end()){
									if (((*It1)->exists()) && (!(*It1)->getType().isBuilding()))
										(*It1)->move(*p);

									It1++;
								}
							}

							delete p;
							Broodwar->printf("muevo los soldadillos");
						}
						delete t;
					}
				}

				buildingSemaphore++;
				trabajador->build((*pos), *tipo);
			}
		}
	}

}

void unit_Manager::buildUnitAddOn(int id){
	
	Unit* owner = NULL;
	UnitType *tipo = new UnitType(id);

	if (id == Utilidades::ID_MACHINESHOP){
		if ((Broodwar->self()->minerals() > tipo->mineralPrice()) && (Broodwar->self()->gas() >= tipo->gasPrice())) {
			owner = getUnit(Utilidades::ID_FACTORY);

			if ((owner != NULL) && (owner->exists()) && (owner->isCompleted())&&(!owner->isMoving()))
				if (owner->isLifted()){
					TilePosition actual = owner->getTilePosition();
					TilePosition* nuevaPos = getTilePositionAviable(tipo, new TilePosition(actual));
					if (!owner->land(*nuevaPos)){
						owner->rightClick(*new Position(*nuevaPos));
					}
				}
				else{
					if (!owner->buildAddon(*new UnitType(Utilidades::ID_MACHINESHOP)))
						owner->lift();
					else{
						Broodwar->printf("estoy levantado en la pos %d,%d", owner->getTilePosition().x(), owner->getTilePosition().y());
					}
				}
		}

		delete tipo;
	}
	else if (id == Utilidades::ID_COMSAT_STATION){
		if ((Broodwar->self()->minerals() > tipo->mineralPrice()) && (Broodwar->self()->gas() >= tipo->gasPrice())) {
			owner = getUnit(Utilidades::ID_COMMANDCENTER);

			if ((owner != NULL) && (owner->exists()) && (owner->isCompleted()))
				owner->buildAddon(*new UnitType(Utilidades::ID_COMSAT_STATION));
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
				*geyserPos = (*pos);
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



void unit_Manager::trainTankSiege(){
	Unit* firstFactory = NULL;
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		if ((*i)->getType().getID() == Utilidades::ID_FACTORY){
			firstFactory = (*i);

			if ((firstFactory != NULL) && (Broodwar->canMake(firstFactory, Utilidades::ID_TANKSIEGE)) && (firstFactory->getTrainingQueue().size() < 2)){
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
								if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)>7)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
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
								if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)>7)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
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
									if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)>7)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
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
									if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)>7)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
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
								if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)>7)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
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
								if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)>7)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
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
									if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)>7)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
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
									if ((worker!=NULL)&&(worker->exists())&&(centrocomando!=NULL)&&(centrocomando->getDistance(*ubicacion)>7)&&(Broodwar->canBuildHere(worker, *ubicacion, *supply))){
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
	Unit* centro = getUnit(Utilidades::ID_COMMANDCENTER);
	if ((centro!=NULL) && (centro->exists())){
		int i = ceil(centro->getTilePosition().getDistance(*t)+0.1);
		int x = t->x();
		int y = t->y();
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
	}
	return pos;

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
			if ((cantUnidades[Utilidades::INDEX_GOAL_MARINE] > 16) && (Fox->countMarines() < 5))
				Fox->asignarUnidad(unit);
			else
				Easy->asignarUnidad(unit);
		}
	}
	else if (unit->getType().getID() == Utilidades::ID_MEDIC){
		Easy->asignarUnidad(unit);
	}
	else if (unit->getType().getID() == Utilidades::ID_FIREBAT){
		Easy->asignarUnidad(unit);
	}
	else if (unit->getType().getID() == Utilidades::ID_TANKSIEGE){
		if ((grupoB1 != NULL) && (grupoB1->faltanTanques()))
			grupoB1->agregarUnidad(unit);
		else if ((grupoB2 != NULL) && (grupoB2->faltanTanques()))
			grupoB2->agregarUnidad(unit);
		else
			Easy->asignarUnidad(unit);
	}
	else if (unit->getType().getID() == Utilidades::ID_GOLIATH){
		Easy->asignarUnidad(unit);
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

			if ((u->isCloaked()) && (cantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION] > 0)){
				Unit *comsat = getUnit(Utilidades::ID_COMSAT_STATION);

				if ((comsat != NULL) && (comsat->exists()) && (comsat->getEnergy() >= 50)){
					comsat->useTech(TechTypes::Scanner_Sweep, u->getPosition());
				}
			}

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
			case Utilidades::ID_ENGINEERING_BAY:
				cantUnidades[Utilidades::INDEX_GOAL_ENGINEERING_BAY]++;
				break;
			case Utilidades::ID_ARMORY:
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
		}

	}

	resetBuildingSemaphore();
	nuevaUnidadConstruccion(u);
}

void unit_Manager::onUnitDestroy(Unit *u){
	
	// TODO: si un edificio por ejemplo la academia terran es destruido, y se tenia solo un edificio de ese tipo,
	// se pierden todas las mejoras investigadas anteriormente, y se deberan investigar de nuevo
	// Esto se debera actualizar solamente en el metodo onUnitDestroy de la clase unit_Manager

	grupoB1->onUnitDestroy(u);

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
				//Broodwar->printf("mataron a boby sin siege, quedan %d tanques",cantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE] );
				break;
			case Utilidades::ID_TANKSIEGE_SIEGEMODE:
				cantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE]--;
				//Broodwar->printf("mataron a kenny, quedan %d tanques",cantUnidades[Utilidades::INDEX_GOAL_TANKSIEGE] );
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
	if (cantUnidades[Utilidades::INDEX_GOAL_COMSAT_STATION] > 0){
		Unit *detector = getUnit(Utilidades::ID_COMSAT_STATION);

		if ((detector != NULL) && (detector->exists()) && (detector->getEnergy() > 50)){
			detector->useTech(TechTypes::Scanner_Sweep, p);
			Fox->atacar(p);
			Broodwar->pingMinimap(p);
		}
	}
}