#include "GrupoBunkers.h"
#include <list>


GrupoBunkers::GrupoBunkers(AnalizadorTerreno *a)
{
	int cuadrante, angulo;
	TilePosition *aux;

	analizador = a;
	choke = a->obtenerChokepoint();
	reg = a->regionInicial();

	// ------------------------------------------------------------------------------------------
	// calcula la posicion de reunion de los soldados, a la cual se dirigiran si el bunker en el que estaban es destruido, para
	// liberar la posicion del bunker asi se puede reconstruir rapidamente

	posEncuentro = NULL;
	cuadrante = a->getCuadrante(reg->getCenter());
	angulo = a->calcularAngulo(choke);

	aux = a->calcularPrimerTile(reg, choke, 1);

	switch (cuadrante){
		case 1:
			if (a->calcularAnguloGrupo(angulo) == 0)
				posEncuentro = new Position(aux->x() * 32 - 192, aux->y() * 32);
			else
				posEncuentro = new Position(aux->x() * 32, aux->y() * 32 - 192);
			break;
		case 2:
			if (a->calcularAnguloGrupo(angulo) == 0)
				posEncuentro = new Position(aux->x() * 32 + 192, aux->y() * 32);
			else
				posEncuentro = new Position(aux->x() * 32, aux->y() * 32 - 192);
			break;
		case 3:
			if (a->calcularAnguloGrupo(angulo) == 0)
				posEncuentro = new Position(aux->x() * 32 - 192, aux->y() * 32);
			else
				posEncuentro = new Position(aux->x() * 32, aux->y() * 32 + 192);
			break;
		case 4:
			if (a->calcularAnguloGrupo(angulo) == 0)
				posEncuentro = new Position(aux->x() * 32 + 192, aux->y() * 32);
			else
				posEncuentro = new Position(aux->x() * 32, aux->y() * 32 + 192);
			break;
	}

	// ------------------------------------------------------------------------------------------

}

GrupoBunkers::~GrupoBunkers(void)
{
	if (posEncuentro != NULL)
		delete posEncuentro;
	delete choke;
	delete reg;
}

void GrupoBunkers::agregarUnidad(Unit* u){
	int cont = 0;

	if (u != NULL){
		if (u->getType().getID() == Utilidades::ID_BUNKER){
			
			if (posicionesLibresBunkers.size() > 0){
				std::set<int>::iterator It1;
		
				// calcula el tile donde se deberia ubicar el bunker de la primera posicion de la lista de posiciones libres
				It1 = posicionesLibresBunkers.begin();

				TilePosition *t = analizador->calcularPrimerTile(reg, choke, *It1);

				// compara si esa posicion es la misma que la que tiene el bunker que se va a agregar al grupo, si es la misma,
				// se elimina esa posicion del conjunto de posiciones libres y se agrega el bunker en esa posicion del grupo
				if ((u->getTilePosition().x() == t->x()) && (u->getTilePosition().y() == t->y())){
					
					std::list<Unit*>::iterator It2;

					It2 = listBunkers.begin();
					cont = (*It1) - 1;

					while (cont > 0){
						It2++;
						cont--;
					}

					listBunkers.insert(It2, u);
					posicionesLibresBunkers.erase(*It1);
					
				}
				else
					listBunkers.push_back(u);
			}
			else
				listBunkers.push_back(u);
			

		}
		else if (u->getType().getID() == Utilidades::ID_MISSILE_TURRET){

			if (posicionesLibresMisileTurrets.size() > 0){
				std::set<int>::iterator It1;
		
				// calcula el tile donde se deberia ubicar el bunker de la primera posicion de la lista de posiciones libres
				It1 = posicionesLibresMisileTurrets.begin();

				TilePosition *t = analizador->calcularPrimerTile(reg, choke, *It1);

				// compara si esa posicion es la misma que la que tiene el bunker que se va a agregar al grupo, si es la misma,
				// se elimina esa posicion del conjunto de posiciones libres y se agrega el bunker en esa posicion del grupo
				if ((u->getTilePosition().x() == t->x()) && (u->getTilePosition().y() == t->y())){
					
					std::list<Unit*>::iterator It2;

					It2 = listMisileTurrets.begin();
					cont = (*It1) - 1;

					while (cont > 0){
						It2++;
						cont--;
					}

					listMisileTurrets.insert(It2, u);
					posicionesLibresMisileTurrets.erase(*It1);
					
				}
				else
					listMisileTurrets.push_back(u);
			}
			else
				listMisileTurrets.push_back(u);


		}
		else if (u->getType().getID() == Utilidades::ID_MARINE){
			listMarines.push_back(u);
		}
		else if (u->getType().getID() == Utilidades::ID_TANKSIEGE){
			listTanks.push_back(u);
			Position *p;
			TilePosition *t;

			t = posicionNuevoTanque();
			p = new Position(t->x() * 32 + 16, t->y() * 32 + 16);
			delete t;

			u->rightClick(*p);
			delete p;
		}
		else
			Broodwar->printf("No se puede agregar ese tipo de unidad a un grupo de bunkers");
	}
}

Unit* GrupoBunkers::getUltimoBunkerCreado(){
	if (listBunkers.size() > 0)
		return listBunkers.back();
	else 
		return NULL;
}

Unit* GrupoBunkers::getPrimerBunkerCreado(){
	if (listBunkers.size() > 0)
		return listBunkers.front();
	else
		return NULL;
}


int GrupoBunkers::getCantBunkers(){

	std::list<Unit*>::iterator It1;
	It1 = listBunkers.begin();
	int cont = 0;

	if (listBunkers.size() > 0){
		while (It1 != listBunkers.end()){
			if ((*It1)->exists())
				cont++;
			
			It1++;
		}
	}

	return cont;
}

int GrupoBunkers::getCantMisileTurrets(){

	std::list<Unit*>::iterator It1;
	It1 = listMisileTurrets.begin();
	int cont = 0;

	if (listMisileTurrets.size() > 0){
		while (It1 != listMisileTurrets.end()){
			if ((*It1)->exists())
				cont++;
			
			It1++;
		}
	}

	return cont;
}

int GrupoBunkers::getCantMarines(){

	std::list<Unit*>::iterator It1;
	It1 = listMarines.begin();
	int cont = 0;

	if (listMarines.size() > 0){
		while (It1 != listMarines.end()){
			if ((*It1)->exists())
				cont++;
			
			It1++;
		}
	}

	return cont;
}

int GrupoBunkers::getCantTanks(){

	std::list<Unit*>::iterator It1;
	It1 = listTanks.begin();
	int cont = 0;

	if (listTanks.size() > 0){
		while (It1 != listTanks.end()){
			if ((*It1)->exists())
				cont++;
			
			It1++;
		}
	}

	return cont;
}


// el bunker atacado es pasado como parametro
void GrupoBunkers::estrategia1(Unit *u){

	if (perteneceBunker(u)){

		if (u->getLoadedUnits().size() > 0){
			std::list<Unit*>::iterator It1;
			std::list<Unit*>::iterator It2;
			std::list<Unit*> temp;
			
			It1 = u->getLoadedUnits().begin();

			//u->unloadAll();

			// si el bunker atacado pertenece al grupo actual, expulsa a los soldados, les aplica el stim pack y los vuelve a ingresar al bunker
			while (It1 != u->getLoadedUnits().end()){
				//if (((*It1)->getType().getID() == Utilidades::ID_MARINE) || ((*It1)->getType().getID() == Utilidades::ID_FIREBAT)){
					
					//if ((*It1)->exists() && ((!(*It1)->isStimmed()) || ((*It1)->getStimTimer() == 0))){
						// agrega al soldado a una lista temporal
						temp.push_front(*It1);
						//u->unload(*It1);
					//}
				//}
				It1++;
			}
		
			u->unloadAll();

			// ahora vuelve a cargar los soldados en el bunker
			It2 = temp.begin();

			while (It2 != temp.end()){
				// si no esta dentro de un contenedor, se aplica el stim pack a la unidad
				/*if (!(*It1)->isLoaded())
					(*It1)->useTech(*(new TechType(TechTypes::Stim_Packs)));*/

				u->load(*It2);
				It2++;
			}
		}
	}
}


bool GrupoBunkers::perteneceBunker(Unit *u){

	if (u != NULL){
		std::list<Unit*>::iterator It1;
		It1 = listBunkers.begin();

		while (It1 != listBunkers.end()){
			if ((*It1)->getID() == u->getID())
				return true;
			It1++;
		}
	}

	return false;
}


TilePosition* GrupoBunkers::posicionNuevoBunker(){
	if (!analizador->analisisListo())
		return NULL;
	
	if (posicionesLibresBunkers.size() == 0){
		return analizador->calcularPrimerTile(reg, choke, getCantBunkers() + 1);
	}
	else{
		std::set<int>::iterator It1;
		It1 = posicionesLibresBunkers.begin();
		return analizador->calcularPrimerTile(reg, choke, *It1);
	}
}


TilePosition* GrupoBunkers::posicionNuevaTorreta(){
	int cuadrante;
	TilePosition *t;
	int x, y;
	int angulo, angulo1;
	int nroTorreta;

	cuadrante = analizador->getCuadrante(reg->getCenter());

	if (posicionesLibresMisileTurrets.size() == 0){
		if (listMisileTurrets.size() == 0)
			nroTorreta = 2;
		else
			nroTorreta = 3;

		t = analizador->calcularPrimerTile(reg, choke, nroTorreta);
	}
	else{
		std::set<int>::iterator It1;

		It1 = posicionesLibresMisileTurrets.begin();
		nroTorreta = *It1;
		t = analizador->calcularPrimerTile(reg, choke, *It1);
	}

	x = t->x();
	y = t->y();
	delete t;

	angulo = analizador->calcularAngulo(choke);
	angulo1 = analizador->calcularAnguloGrupo(angulo);
	
	int offset = 0;

	if (nroTorreta == 2)
		offset = 1;

	switch (cuadrante){
		case 1:
			if (angulo1 == 90)
				return new TilePosition(x + offset, y - 2);
			else
				return new TilePosition(x - 3, y + offset);
			break;
		case 2:
			if (angulo1 == 90)
				return new TilePosition(x + offset, y - 2);
			else
				return new TilePosition(x + 3, y + offset);
			break;
		case 3:
			if (angulo1 == 90)
				return new TilePosition(x + offset, y + 2);
			else
				return new TilePosition(x - 3, y + offset);
			break;
		case 4:
			if (angulo1 == 90)
				return new TilePosition(x + offset, y + 2);
			else
				return new TilePosition(x + 3, y + offset);
			break;
	}
}


TilePosition* GrupoBunkers::posicionNuevoTanque(){
	
	TilePosition *aux;
	int angulo, angulo1, cuadrante;
	int offset;

	aux = analizador->calcularPrimerTile(reg, choke, 1);
	angulo = analizador->calcularAngulo(choke);
	angulo1 = analizador->calcularAnguloGrupo(angulo);
	cuadrante = analizador->getCuadrante(reg->getCenter());

	// el grupo de bunkers esta ubicado en forma horizontal
	if (angulo1 == 90){
		if ((cuadrante == 1) || (cuadrante == 2))
			offset = -2;
		else
			offset = 2;
		
		if (posicionesLibresTanques.size() == 0)
			return (new TilePosition(aux->x() + listTanks.size(), aux->y() + offset));
		else{
			std::set<int>::iterator It;
			It = posicionesLibresTanques.begin();
			int temp = *It;
			posicionesLibresTanques.erase(It);

			return (new TilePosition(aux->x() + temp - 1, aux->y() + offset));
		}

	}
	else if (angulo1 == 0){
		if ((cuadrante == 1) || (cuadrante == 3))
			offset = -2;
		else
			offset = 2;
		
		if (posicionesLibresTanques.size() == 0)
			return (new TilePosition(aux->x() + offset, aux->y() + listTanks.size()));
		else{
			std::set<int>::iterator It;
			It = posicionesLibresTanques.begin();
			int temp = *It;
			posicionesLibresTanques.erase(It);

			return (new TilePosition(aux->x() + offset, aux->y() + temp - 1));
		}
	}
	else
		return NULL;
}



void GrupoBunkers::controlDestruidos(){
	std::list<Unit*>::iterator It1;
	It1 = listBunkers.begin();
	int cont = 1;

	// ----------- controla los bunkers -----------
	while (It1 != listBunkers.end()){
		if (!(*It1)->exists()){
			listBunkers.erase(It1);
			It1 = listBunkers.begin(); // tuve que poner esto porque sino se colgaba el while...

			posicionesLibresBunkers.insert(cont);
			moverSoldadosPosEncuentro();
		}
		else
			It1++;

		cont++;
	}

	// ----------- controla las torretas de misiles -----------
	It1 = listMisileTurrets.begin();
	cont = 2;

	while (It1 != listMisileTurrets.end()){
		if (!(*It1)->exists()){
			listMisileTurrets.erase(It1);
			It1 = listMisileTurrets.begin(); // tuve que poner esto porque sino se colgaba el while...

			posicionesLibresMisileTurrets.insert(cont);
		}
		else
			It1++;

		cont++;
	}

	// ----------- controla los marines -----------
	It1 = listMarines.begin();

	while (It1 != listMarines.end()){
		if (!(*It1)->exists()){
			listMarines.erase(It1);
			It1 = listMarines.begin(); // tuve que poner esto porque sino se colgaba el while...
		}
		else
			It1++;
	}


	// ----------- controla los tanques -----------
	It1 = listTanks.begin();
	while (It1 != listTanks.end()){
		if (!(*It1)->exists()){
			listTanks.erase(It1);
			It1 = listTanks.begin(); // tuve que poner esto porque sino se colgaba el while...

			posicionesLibresTanques.insert(cont);
		}
		else
			It1++;

		cont++;
	}
}


void GrupoBunkers::ponerACubierto(){
	std::list<Unit*>::iterator ItM;
	std::list<Unit*>::iterator ItB;

	ItM = listMarines.begin();

	while (ItM != listMarines.end()){
		// si el marine no esta dentro de un bunker, busca uno para ponerlo a cubierto
		if (((*ItM)->isCompleted()) && (!(*ItM)->isLoaded())){
			ItB = listBunkers.begin();

			while (ItB != listBunkers.end()){
				if (((*ItB)->isCompleted()) && ((*ItB)->getLoadedUnits().size() < 4)){
					(*ItB)->load(*ItM);
					ItB = listBunkers.end();
				}
				else{
					ItB++;
				}
			}
		}
		ItM++;
	}
}


void GrupoBunkers::onFrame(){

	resaltarUnidades();

	if (Broodwar->getFrameCount() % frameLatency == 0){
		//Broodwar->printf("bunker onFrame");

		//if (getCantBunkers() != listBunkers.size()) // es decir hay algun bunker que ya no existe en la lista, se debe actualizar
		controlDestruidos();

		if (getCantBunkers() > 1)
			ponerACubierto();

		ubicarModoSiege();
	}
}

void GrupoBunkers::ubicarModoSiege(){
	std::list<Unit*>::iterator It;

	It = listTanks.begin();

	while (It != listTanks.end()){
		if ((*It)->isIdle() && (!(*It)->isSieged()))
			(*It)->siege();

		It++;
	}
}


bool GrupoBunkers::faltanMarines(){
	return (getCantMarines() < 12);
}

bool GrupoBunkers::faltanTanques(){
	return (getCantTanks() < 3);
}


void GrupoBunkers::moverSoldadosPosEncuentro(){
	if (posEncuentro != NULL){
		std::list<Unit*>::iterator It1;
		It1 = listMarines.begin();

		while (It1 != listMarines.end()){
			if (((*It1)->isCompleted()) && (!(*It1)->isLoaded()))
				(*It1)->rightClick(*posEncuentro);

			It1++;
		}
	}
}


void GrupoBunkers::resaltarUnidades(){
	std::list<Unit*>::iterator It1;

	It1 = listMarines.begin();

	while (It1 != listMarines.end()){
		if (((*It1)->isCompleted()) && (!(*It1)->isLoaded()))
			//Graficos::dibujarCirculo((*It1)->getPosition(), (*It1)->getType().tileWidth(), (*It1)->getType().tileHeight());
			Graficos::resaltarUnidad((*It1), Colors::White);

		It1++;
	}	

	// ---------------------------------------------------------------------

	It1 = listTanks.begin();

	while (It1 != listTanks.end()){
		if (((*It1)->isCompleted()) && (!(*It1)->isLoaded()))
			Graficos::resaltarUnidad((*It1), Colors::White);

		It1++;
	}
}