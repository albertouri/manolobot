#include "CompaniaTransporte.h"

CompaniaTransporte::CompaniaTransporte(Position* baseEnem, Region* regEnem){
	baseEnemiga = baseEnem;
	regionBaseEnemiga = regEnem;

	if (baseEnemiga != NULL){
		//Position *pr = new Position(Broodwar->self()->getStartLocation().x() * TILE_SIZE, Broodwar->self()->getStartLocation().y() * TILE_SIZE);
		Position *pr = new Position((*regionBaseEnemiga->getChokepoints().begin())->getCenter().x(), (*regionBaseEnemiga->getChokepoints().begin())->getCenter().y());
		int masLejano = 0;

		BWTA::Polygon pol = regionBaseEnemiga->getPolygon();
		for(int j = 0; j < (int)pol.size(); j++){
			if (pol[j].getDistance(*pr) > pol[masLejano].getDistance(*pr))
				masLejano = j;
		}

		/*Broodwar->drawLineMap(Broodwar->self()->getStartLocation().x() * TILE_SIZE, Broodwar->self()->getStartLocation().y() * TILE_SIZE, pol[masLejano].x(), pol[masLejano].y(), Colors::White);
		masLejano = (int)pol[masLejano].getDistance(*pr);
		Broodwar->drawCircleMap(pr->x(), pr->y(), masLejano, Colors::White, false);
		Broodwar->drawBoxMap(pr->x(), pr->y(), pr->x() + 16, pr->y() + 16, Colors::White, true);*/
		delete pr;

		bordeMasLejano = new Position(pol[masLejano].x(), pol[masLejano].y());
		
		crearPath();
	}

	liderFormacion = NULL;
	ready = false;

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

	if (Broodwar->getFrameCount() % 100 == 0){
		controlarEliminados();

		if (!ready)
			ready = listaTransportar();

		/*if (ready)
			ejecutarTransporte();*/
	}
}


void CompaniaTransporte::asignarUnidad(Unit* u){
	if (u->getType().getID() == Utilidades::ID_DROPSHIP){
		listDropships.push_back(u);

		if (liderFormacion == NULL)
			liderFormacion = u;
	}
}


bool CompaniaTransporte::listaTransportar(){
	return (listDropships.size() > 1);
}


void CompaniaTransporte::controlarEliminados(){
	std::list<Unit*>::iterator Itd;
	
	Itd = listDropships.begin();
	while (Itd != listDropships.end()){
		if (!(*Itd)->exists())
			listDropships.erase(Itd);
		else
			Itd++;
	}
}


void CompaniaTransporte::ejecutarTransporte(){
	std::list<Unit*>::iterator It;

	if ((liderFormacion != NULL) && (liderFormacion->exists()) && (liderFormacion->isIdle()) && (ItPosiciones != pathBaseEnemiga.end())){
		// falta armar la formacion

		It = listDropships.begin();
		while (It != listDropships.end()){
			(*It)->move(**ItPosiciones);
			It++;
		}
		ItPosiciones++;
	}
}