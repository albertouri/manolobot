#include "CompaniaTransporte.h"

CompaniaTransporte::CompaniaTransporte(Position* baseEnem, Region* regEnem, compania* c){
	baseEnemiga = baseEnem;
	regionBaseEnemiga = regEnem;

	if (baseEnemiga != NULL){
		//-- busca el punto mas lejano de la region enemiga con respecto al chokepoint defendido en la base enemiga
		Position *pr = new Position((*regionBaseEnemiga->getChokepoints().begin())->getCenter().x(), (*regionBaseEnemiga->getChokepoints().begin())->getCenter().y());
		int masLejano = 0;

		BWTA::Polygon pol = regionBaseEnemiga->getPolygon();
		for(int j = 0; j < (int)pol.size(); j++){
			if (pol[j].getDistance(*pr) > pol[masLejano].getDistance(*pr))
				masLejano = j;
		}

		delete pr;

		bordeMasLejano = new Position(pol[masLejano].x(), pol[masLejano].y());


		//-- crea el camino que recorreran los transportes
		crearPath();
	}

	liderFormacion = NULL;
	ready = false;
	aero = c;
	comandanteCargado = false;

	estadoActual = ESPERANDO_CARGAR;
}


CompaniaTransporte::~CompaniaTransporte(void)
{
}



void CompaniaTransporte::crearPath(){
	//-- region donde manolobot inicia el juego
	Region *inicial = BWTA::getStartLocation(Broodwar->self())->getRegion();
	Position *p1, *p2, *centroBase;
	Chokepoint *choke = NULL;

	centroBase = new Position(inicial->getCenter().x(), inicial->getCenter().y());
	pathBaseEnemiga.push_back(centroBase);

	p1 = new Position(inicial->getCenter().x(), bordeMasLejano->y());
	p2 = new Position(bordeMasLejano->x(), inicial->getCenter().y());

	//-- calcula el chokepoint que debe estar defendido en la base enemiga, para evitar pasar por ahi
	if (regionBaseEnemiga->getChokepoints().size() > 1){
		std::set<Chokepoint*>::const_iterator It;

		It = regionBaseEnemiga->getChokepoints().begin();

		while (It != regionBaseEnemiga->getChokepoints().end()){

			if ((*It)->getRegions().first != regionBaseEnemiga){
				//-- si tiene mas de 1 chokepoint, esa region tiene otro punto de entrada, por lo tanto el chokepoint debe estar defendido
				if ((*It)->getRegions().first->getChokepoints().size() > 1){
					choke = (*It);
					break;
				}
			}
			else{
				//-- si tiene mas de 1 chokepoint, esa region tiene otro punto de entrada, por lo tanto el chokepoint debe estar defendido
				if ((*It)->getRegions().second->getChokepoints().size() > 1){
					choke = (*It);
					break;
				}
			}

			It++;
		}
	}
	else
		choke = *regionBaseEnemiga->getChokepoints().begin();

	//-- fin busqueda chokepoint defendido


	if (choke != NULL){
		if (p1->getDistance(choke->getCenter()) > p2->getDistance(choke->getCenter())){
			pathBaseEnemiga.push_back(p1);
			delete p2;
		}
		else{
			pathBaseEnemiga.push_back(p2);
			delete p1;
		}

		pathBaseEnemiga.push_back(bordeMasLejano);
	}


	ItPosiciones = pathBaseEnemiga.begin();
	
}


void CompaniaTransporte::dibujarPath(){
	std::list<Position*>::iterator It;
	Position *anterior = new Position(BWTA::getStartLocation(Broodwar->self())->getRegion()->getCenter().x(), BWTA::getStartLocation(Broodwar->self())->getRegion()->getCenter().y());

	It = pathBaseEnemiga.begin();
	while (It != pathBaseEnemiga.end()){
		Broodwar->drawLineMap(anterior->x(), anterior->y(), (*It)->x(), (*It)->y(), Colors::White);

		if (It == pathBaseEnemiga.begin())
			delete anterior;	

		anterior = (*It);

		It++;
	}
}


void CompaniaTransporte::onFrame(){
	dibujarPath();

	if ((liderFormacion != NULL) && (liderFormacion->exists()))
		Graficos::resaltarUnidad(liderFormacion, Colors::Green);

	if ((Broodwar->getFrameCount() % 12 == 0)){
		if ((liderFormacion == NULL) || (!liderFormacion->exists())){
			controlarEliminados();
			reasignarLiderFormacion();
		}
	}
	else if (Broodwar->getFrameCount() % 30 == 0){
		if (estadoActual == ESPERANDO_CARGAR){
			if (listaTransportar())
				estadoActual = TRANSPORTANDO;
		}
		else if (estadoActual == TRANSPORTANDO){
			ejecutarTransporte();
		}
		else if (estadoActual == DESEMBARCANDO){
			if (desembarcoListo())
				estadoActual = RETORNANDO_BASE;
		}
		else if (estadoActual == RETORNANDO_BASE){
			retornarBase();
		}
	}
	else if (Broodwar->getFrameCount() % 50 == 0){
		controlarEliminados();
	}
}


void CompaniaTransporte::asignarUnidad(Unit* u){
	controlarEliminados();

	if ((u != NULL) && (u->exists())){
		if (u->getType().getID() == Utilidades::ID_DROPSHIP){
			listDropships.push_back(u);

			if ((aero != NULL) && (aero->getComandante() != NULL) && (!comandanteCargado)){
				(*listDropships.begin())->load(aero->getComandante());
				comandanteCargado = true;
			}
		}
		else if(u->getType().getID() == Utilidades::ID_WRAITH){
			listWraiths.push_back(u);

			if (liderFormacion == NULL)
				liderFormacion = u;
		}
	}
}


bool CompaniaTransporte::listaTransportar(){
	/*Broodwar->printf("tengo %d dropships, necesito %d  - tengo %d marines", listDropships.size(), aero->cantidadTransportes(), aero->countMarines());

	if (aero->listaParaAtacar())
		Broodwar->printf("compañia lista para atacar");
	else
		Broodwar->printf("compañia NO ESTA lista para atacar");

	Broodwar->printf("----------------------------------------------");*/


	if ((listDropships.size() == aero->cantidadTransportes()) && (listWraiths.size() == 2) && (aero->listaParaAtacar())){
		aero->abordarTransporte(&listDropships);
		return aero->companiaAbordo();
	}
	else
		return false;
}


void CompaniaTransporte::controlarEliminados(){
	
	if (!listDropships.empty()){
		std::list<Unit*>::iterator Itd;
		
		Itd = listDropships.begin();
		while (Itd != listDropships.end()){
			if (!(*Itd)->exists()){
				listDropships.erase(Itd);
				Itd = listDropships.begin();
			}
			else{
				Itd++;
			}
		}
	}

	if (!listWraiths.empty()){
		std::list<Unit*>::iterator Itd;
		
		Itd = listWraiths.begin();
		while (Itd != listWraiths.end()){
			if (!(*Itd)->exists()){
				listWraiths.erase(Itd);
				Itd = listWraiths.begin();
			}
			else{
				Itd++;
			}
		}
	}
}


void CompaniaTransporte::ejecutarTransporte(){
	std::list<Unit*>::iterator It;

	if ((liderFormacion != NULL) && (liderFormacion->exists()) && (liderFormacion->isIdle()) && (ItPosiciones != pathBaseEnemiga.end())){
		// falta armar la formacion

		It = listWraiths.begin();
		while (It != listWraiths.end()){
			if ((*It)->exists())
				(*It)->move(**ItPosiciones);

			It++;
		}

		It = listDropships.begin();
		while (It != listDropships.end()){
			if ((*It)->exists())
				(*It)->move(**ItPosiciones);

			It++;
		}
		ItPosiciones++;
	}

	if ((liderFormacion != NULL) && (liderFormacion->exists())){
		//Broodwar->printf("intento desembarcar, distancia %lf", liderFormacion->getPosition().getDistance((*bordeMasLejano)));

		if (liderFormacion->getPosition().getDistance(*bordeMasLejano) < 120.0){
			Position *p;

			if (bordeMasLejano->x() < regionBaseEnemiga->getCenter().x())
				p = new Position(bordeMasLejano->x() + 64, bordeMasLejano->y());
			else
				p = new Position(bordeMasLejano->x() - 64, bordeMasLejano->y());

			It = listDropships.begin();
			while (It != listDropships.end()){
				if ((*It)->exists()){
					(*It)->unloadAll(*p);
				}

				It++;
			}

			It = listWraiths.begin();
			while (It != listWraiths.end()){
				if ((*It)->exists()){
					(*It)->attackMove(regionBaseEnemiga->getCenter());
				}

				It++;
			}
		}
	}
}


bool CompaniaTransporte::desembarcoListo(){
	std::list<Unit*>::iterator It = listDropships.begin();

	while (It != listDropships.end()){
		if (((*It) != NULL) && ((*It)->exists()) && (!(*It)->getLoadedUnits().empty()))
			return false;

		It++;
	}

	return true;
}


void CompaniaTransporte::retornarBase(){
	std::list<Unit*>::iterator It;

	if ((liderFormacion != NULL) && (liderFormacion->exists()) && (liderFormacion->isIdle()) && (ItPosiciones != pathBaseEnemiga.begin())){

		/*It = listWraiths.begin();
		while (It != listWraiths.end()){
			if ((*It)->exists())
				(*It)->move(**ItPosiciones);

			It++;
		}*/

		It = listDropships.begin();
		while (It != listDropships.end()){
			if ((*It)->exists())
				(*It)->move(**ItPosiciones);

			It++;
		}
		ItPosiciones++;
	}
	else
		estadoActual = ESPERANDO_CARGAR;
}


void CompaniaTransporte::reasignarLiderFormacion(){
	std::list<Unit*>::iterator It;
	liderFormacion = NULL;

	//-- recorro la lista de wraiths buscando alguna unidad existente y la asigno como nuevo lider de la formacion
	if (!listWraiths.empty()){
		It = listWraiths.begin();

		while (It != listWraiths.end()){
			if (((*It) != NULL) && ((*It)->exists())){
				liderFormacion = (*It);
				break;
			}

			It++;
		}
	}

	//-- si no encontre ningun wraith existente, recorro la lista de dropships buscando alguna unidad existente y la 
	//-- asigno como nuevo lider de la formacion
	if ((!listDropships.empty()) && (liderFormacion == NULL)){
		It = listDropships.begin();

		while (It != listDropships.end()){
			if (((*It) != NULL) && ((*It)->exists()) && (!(*It)->isLockedDown())){
				liderFormacion = (*It);
				break;
			}

			It++;
		}
	}
}