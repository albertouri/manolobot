#include "GrupoBunkers.h"
#include <list>

bool a = true;

GrupoBunkers::GrupoBunkers(AnalizadorTerreno *a)
{
	int cuadrante, angulo;
	TilePosition *aux;

	analizador = a;
	choke = a->obtenerChokepoint();
	reg = a->regionInicial();

	bunkerCentral = NULL;
	bunkerCentral = posicionPrimerBunker(reg, choke);

	// ------------------------------------------------------------------------------------------
	// calcula la posicion de reunion de los soldados, a la cual se dirigiran si el bunker en el que estaban es destruido, para
	// liberar la posicion del bunker asi se puede reconstruir rapidamente

	posEncuentro = NULL;
	cuadrante = a->getCuadrante(reg->getCenter());
	angulo = a->calcularAngulo(choke);

	aux = bunkerCentral;

	if (aux != NULL){
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
	}
	else
		Broodwar->printf("ERROR 002: No encuentro ubicacion posible para la posicion de encuentro");
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
			
/*			if (posicionesLibresBunkers.size() > 0){
				std::set<int>::iterator It1;
		
				// calcula el tile donde se deberia ubicar el bunker de la primera posicion de la lista de posiciones libres
				It1 = posicionesLibresBunkers.begin();

				//TilePosition *t = posicionPrimerBunker(reg, choke, *It1);
				TilePosition *t = new TilePosition(bunkerCentral->x(), bunkerCentral->y());

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
			else*/
				listBunkers.push_back(u);
			

		}
		else if (u->getType().getID() == Utilidades::ID_MISSILE_TURRET){

			/*if (posicionesLibresMisileTurrets.size() > 0){
				std::set<int>::iterator It1;
		
				// calcula el tile donde se deberia ubicar el bunker de la primera posicion de la lista de posiciones libres
				It1 = posicionesLibresMisileTurrets.begin();

				//TilePosition *t = posicionPrimerBunker(reg, choke, *It1);
				TilePosition *t = new TilePosition(bunkerCentral->x(), bunkerCentral->y());

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
			else*/
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
	TilePosition *t = NULL;

	int angulo, angulo1;

	if (!analizador->analisisListo())
		return NULL;

	angulo = analizador->calcularAngulo(choke);
	angulo1 = analizador->calcularAnguloGrupo(angulo);

	if (bunkerCentral != NULL){
		if (angulo1 == 90){
			if (!ocupado(*bunkerCentral, Utilidades::ID_BUNKER)){
				return (new TilePosition(bunkerCentral->x(), bunkerCentral->y()));
			}
			else{
				t = new TilePosition(bunkerCentral->x() + 3, bunkerCentral->y());
				if (!ocupado(*t, Utilidades::ID_BUNKER)){
					return t;
				}
				else{
					delete t;
					t = new TilePosition(bunkerCentral->x() - 3, bunkerCentral->y());
					if (!ocupado(*t, Utilidades::ID_BUNKER)){
						return t;
					}
					else{
						delete t;
						return NULL;
					}
				}
			}
		}
		else{
			if (!ocupado(*bunkerCentral, Utilidades::ID_BUNKER)){
				return (new TilePosition(bunkerCentral->x(), bunkerCentral->y()));
			}
			else{
				t = new TilePosition(bunkerCentral->x(), bunkerCentral->y() + 2);
				if (!ocupado(*t, Utilidades::ID_BUNKER)){
					return t;
				}
				else{
					delete t;
					t = new TilePosition(bunkerCentral->x(), bunkerCentral->y() - 2);
					if (!ocupado(*t, Utilidades::ID_BUNKER)){
						return t;
					}
					else{
						delete t;
						return NULL;
					}
				}
			}
		}
	}
	else{
		Broodwar->printf("El bunker central es NULL");
		return NULL;
	}
}


TilePosition* GrupoBunkers::posicionNuevaTorreta(){
	int cuadrante;
	TilePosition *t;
	int angulo, angulo1;

	cuadrante = analizador->getCuadrante(reg->getCenter());
	angulo = analizador->calcularAngulo(choke);
	angulo1 = analizador->calcularAnguloGrupo(angulo);

	//-- NUEVO CODIGO
	if (bunkerCentral != NULL){
		switch (cuadrante){
			//-- PRIMER CUADRANTE (ARRIBA A LA IZQUIERDA)
			case 1:
				if (angulo1 == 90){
					// si el angulo del grupo de bunkers es 90 grados, intenta las 2 posiciones posibles para ubicar las misile turrets
					t = new TilePosition(bunkerCentral->x() + 4, bunkerCentral->y() - 2);
					
					//-- intenta en posicion 1
					if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
						return t;
					else{
						delete t;
						t = new TilePosition(bunkerCentral->x() - 3, bunkerCentral->y() - 2);

						//-- intenta en posicion 2
						if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
							return t;
						else{
							delete t;
							return NULL;
						}
					}
				}
				else{
					// angulo == 0
					t = new TilePosition(bunkerCentral->x() - 2, bunkerCentral->y() - 3);
					
					//-- intenta en posicion 1
					if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
						return t;
					else{
						delete t;
						t = new TilePosition(bunkerCentral->x() - 2, bunkerCentral->y() + 3);

						//-- intenta en posicion 2
						if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
							return t;
						else{
							delete t;
							return NULL;
						}
					}
				}
				break;

			//-- SEGUNDO CUADRANTE (ARRIBA A LA DERECHA)
			case 2:
				if (angulo1 == 90){
					// si el angulo del grupo de bunkers es 90 grados, intenta las 2 posiciones posibles para ubicar las misile turrets
					t = new TilePosition(bunkerCentral->x() + 4, bunkerCentral->y() - 2);
					
					//-- intenta en posicion 1
					if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
						return t;
					else{
						delete t;
						t = new TilePosition(bunkerCentral->x() - 3, bunkerCentral->y() - 2);

						//-- intenta en posicion 2
						if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
							return t;
						else{
							delete t;
							return NULL;
						}
					}
				}
				else{
					// angulo == 0
					t = new TilePosition(bunkerCentral->x() + 3, bunkerCentral->y() - 3);
					
					//-- intenta en posicion 1
					if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
						return t;
					else{
						delete t;
						t = new TilePosition(bunkerCentral->x() + 3, bunkerCentral->y() + 3);

						//-- intenta en posicion 2
						if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
							return t;
						else{
							delete t;
							return NULL;
						}
					}
				}
				break;

			//-- TERCER CUADRANTE (ABAJO A LA IZQUIERDA)
			case 3:
				if (angulo1 == 90){
					// si el angulo del grupo de bunkers es 90 grados, intenta las 2 posiciones posibles para ubicar las misile turrets
					t = new TilePosition(bunkerCentral->x() + 4, bunkerCentral->y() + 2);
					
					//-- intenta en posicion 1
					if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
						return t;
					else{
						delete t;
						t = new TilePosition(bunkerCentral->x() - 3, bunkerCentral->y() + 2);

						//-- intenta en posicion 2
						if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
							return t;
						else{
							delete t;
							return NULL;
						}
					}
				}
				else{
					// angulo == 0
					t = new TilePosition(bunkerCentral->x() - 2, bunkerCentral->y() - 3);
					
					//-- intenta en posicion 1
					if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
						return t;
					else{
						delete t;
						t = new TilePosition(bunkerCentral->x() - 2, bunkerCentral->y() + 3);

						//-- intenta en posicion 2
						if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
							return t;
						else{
							delete t;
							return NULL;
						}
					}
				}
				break;

			//-- CUARTO CUADRANTE (ABAJO A LA DERECHA)
			case 4:
				if (angulo1 == 90){
					// si el angulo del grupo de bunkers es 90 grados, intenta las 2 posiciones posibles para ubicar las misile turrets
					t = new TilePosition(bunkerCentral->x() + 4, bunkerCentral->y() + 2);
					
					//-- intenta en posicion 1
					if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
						return t;
					else{
						delete t;
						t = new TilePosition(bunkerCentral->x() - 3, bunkerCentral->y() + 2);

						//-- intenta en posicion 2
						if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
							return t;
						else{
							delete t;
							return NULL;
						}
					}
				}
				else{
					// angulo == 0
					t = new TilePosition(bunkerCentral->x() + 3, bunkerCentral->y() - 3);
					
					//-- intenta en posicion 1
					if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
						return t;
					else{
						delete t;
						t = new TilePosition(bunkerCentral->x() + 3, bunkerCentral->y() + 3);

						//-- intenta en posicion 2
						if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
							return t;
						else{
							delete t;
							return NULL;
						}
					}
				}
				break;
		
		} //-- FIN DEL SWITCH

	}
	//-- FIN NUEVO CODIGO

	return NULL;
}


TilePosition* GrupoBunkers::posicionNuevoTanque(){
	
	TilePosition *aux;
	int angulo, angulo1, cuadrante;
	int offset;

	// obtiene el tilePosition del primer bunker del grupo, para tener como referencia ese tilePosition
	//aux = calcularPrimerTile(reg, choke, 1);
	aux = new TilePosition(bunkerCentral->x(), bunkerCentral->y());
	angulo = analizador->calcularAngulo(choke);
	angulo1 = analizador->calcularAnguloGrupo(angulo);
	cuadrante = analizador->getCuadrante(reg->getCenter());

	if (aux != NULL){
		// el grupo de bunkers esta ubicado en forma horizontal
		if (angulo1 == 90){
			if ((cuadrante == 1) || (cuadrante == 2))
				offset = -1;
			else
				offset = 1;
			
			if (posicionesLibresTanques.size() == 0)
				// la parte offset * (2 +  2 * listTanks.size() % 3))) de la expresion hace que los tanques se ubiquen en filas de 3 tanques
				return (new TilePosition(aux->x() + 2 * (listTanks.size() % 3), aux->y() + offset * (3 +  2 * listTanks.size() % 3)));
			else{
				std::set<int>::iterator It;
				It = posicionesLibresTanques.begin();
				int temp = *It;
				posicionesLibresTanques.erase(It);

				return (new TilePosition(aux->x() + 2 * ((temp - 1) % 3), aux->y() + offset * (3 + 2 * ((temp - 1) % 3))));
			}

		}
		else if (angulo1 == 0){
			if ((cuadrante == 1) || (cuadrante == 3))
				offset = -1;
			else
				offset = 1;
			
			if (posicionesLibresTanques.size() == 0)
				return (new TilePosition(aux->x() + offset * (3 + 2 * listTanks.size() % 3), aux->y() + 2 * (listTanks.size() % 3)));
			else{
				std::set<int>::iterator It;
				It = posicionesLibresTanques.begin();
				int temp = *It;
				posicionesLibresTanques.erase(It);

				return (new TilePosition(aux->x() + offset * (3 + 2 * ((temp - 1) % 3)), aux->y() + 2 * ((temp - 1) % 3) ));
			}
		}
		else
			return NULL;
	}
	else{
		Broodwar->printf("ERROR: No se encontro posicion - Metodo: posicionNuevoTanque - Clase: GrupoBunkers");
		return NULL;
	}
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

			//posicionesLibresBunkers.insert(cont);
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

			//posicionesLibresMisileTurrets.insert(cont);
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

			//posicionesLibresTanques.insert(cont);
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
	else
		Broodwar->printf("ERROR 001: posicion de encuentro no seteada");
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



//-- SECCION DE UBICACION DE BUNKERS

TilePosition* GrupoBunkers::posicionPrimerBunker(Region* r, Chokepoint* c){
	int cuadrante = 0;
	int angulo = 0;

	TilePosition *res = NULL;

	bool encontre = false;
	int cont = 0;

	cuadrante = analizador->getCuadrante(r->getCenter());
	angulo = analizador->calcularAngulo(c);
	res = encontrarPosicion(cuadrante, c->getCenter(), angulo);

	/*if (res == NULL)
		Broodwar->printf("ERROR: No se encontro posicion - Metodo: calcularPrimerTile - Clase: AnalizadorTerreno");*/

	return res;
}

bool GrupoBunkers::puedoConstruir(TilePosition t, UnitType tipo){
	bool res = true;

	// verifica si la posicion esta dentro del mapa
	if ((t.x() > Broodwar->mapWidth()) || (t.y() > Broodwar->mapHeight()))
		return false;

	for (int x = 0; x < tipo.tileWidth(); x++){
		for (int y = 0; y < tipo.tileHeight(); y++)
			res = res && Broodwar->isBuildable(t.x() + x, t.y() + y);
	}

	return res;
}

bool GrupoBunkers::ocupado(TilePosition t, int IDTipo){
	bool ocupado = false;

	//-- recorre la lista de unidades en el tile para verificar si hay un bunker
	std::set<Unit*>::iterator It;
	It = Broodwar->unitsOnTile(t.x(), t.y()).begin();

	while (It != Broodwar->unitsOnTile(t.x(), t.y()).end()){
		if ((*It)->getType().getID() == IDTipo){
			ocupado = true;
			break;
		}
		It++;
	}
	//--

	return ocupado;
}

TilePosition* GrupoBunkers::encontrarPosicion(int cuadrante, Position p, int angulo){
	int factorX, factorY;

	int contX = 0, contY = 0;
	bool encontre = false;
	TilePosition *t, *res = NULL;

	int angulo1 = 0;
	
	// calcula el cuadrante donde se ubicara el grupo de bunkers
	if (cuadrante == 1){
		factorX = -1;
		factorY = -1;
	}
	else if (cuadrante == 2){
		factorX = 1;
		factorY = -1;
	}
	else if (cuadrante == 3){
		factorX = -1;
		factorY = 1;
	}
	else if (cuadrante == 4){
		factorX = 1;
		factorY = 1;
	}

	//Broodwar->printf("cuadrante: %d", cuadrante);

	angulo1 = analizador->calcularAnguloGrupo(angulo);

	//Broodwar->printf("angulo1: %d", angulo1);
	//Broodwar->printf("angulo real: %d", angulo);

	if (angulo1 == 90){
		// si el angulo del chokepoint es totalmente horizontal, intenta construir 3 bunkers alineados sobre el chokepoint,
		// si no puede se mueve un tile en el eje Y, e intenta nuevamente
		// contY lleva la cuenta de los tiles que se movio sobre el ejeY
		// para cada posicion sobre el eje Y, prueba en 5 posiciones sobre el eje X:
		// desde el centro del choquepoint - 2 buildtiles hasta el centro del choquepoint + 2 buildtiles

		contX = -2;
		//Broodwar->printf("Horizontal");
		while (contY < 10){
			t = new TilePosition(p.x() / 32 + contX, p.y() / 32 + contY * factorY);

			if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
				// puedo construir el primer bunker en esa posicion, ahora verifico si puedo construir otro bunker al lado
				//if (nroBunker == 1)
				//if (!ocupado(*t, Utilidades::ID_BUNKER)){
					res = new TilePosition(t->x(), t->y() + factorY);

					/*if (a)
					Broodwar->printf("Posicion 1 libre");*/
				//}

				delete t;
				t = new TilePosition(p.x() / 32 + 3 + contX, p.y() / 32 + contY * factorY);

				if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
					// puedo construir el primer bunker en esa posicion, ahora verifico si puedo construir otro bunker al lado
					//if (nroBunker == 2)
					/*if (!ocupado(*t, Utilidades::ID_BUNKER)){
						res = new TilePosition(t->x(), t->y() + factorY);
						
						/*if (a)
						Broodwar->printf("Posicion 2 libre");
					}*/

					delete t;
					t = new TilePosition(p.x() / 32 - 3 + contX, p.y() / 32 + contY * factorY);

					if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
						//if (nroBunker == 3)
						/*if (!ocupado(*t, Utilidades::ID_BUNKER)){
							res = new TilePosition(t->x(), t->y() + factorY);

							if (a)
							Broodwar->printf("Posicion 3 libre");
						}*/

						delete t;

						return res;
					}
					else delete t;
				}
				else delete t;
			}
			else delete t;

			if (contX == 2){
				contY++;
				contX = -2;
			}
			else
				contX++;

			if (res != NULL){
				delete res;
				res = NULL;
			}
		}
	}
	else if (angulo1 == 0){
		contY = -2;

		while (contX < 10){
			t = new TilePosition(p.x() / 32 + contX * factorX, p.y() / 32 + contY);

			if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
				// puedo construir el primer bunker en esa posicion, ahora verifico si puedo construir otro bunker al lado
				//if (nroBunker == 1)
					res = new TilePosition(t->x() /*- factorX*/, t->y());

				delete t;
				t = new TilePosition(p.x() / 32 + contX * factorX, p.y() / 32 + 2 + contY);

				if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
					// puedo construir el primer bunker en esa posicion, ahora verifico si puedo construir otro bunker al lado
					//if (nroBunker == 2)
						//res = new TilePosition(t->x() /*- factorX*/, t->y());

					delete t;
					t = new TilePosition(p.x() / 32 + contX * factorX, p.y() / 32 - 2 + contY);

					if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
						//if (nroBunker == 3)
							//res = new TilePosition(t->x() /*- factorX*/, t->y());
						
						delete t;

						return res;
					}
					else delete t;
				}
				else delete t;
			}
			else delete t;

			if (contY == 2){
				contX++;
				contY = -2;
			}
			else
				contY++;

			if (res != NULL){
				delete res;
				res = NULL;
			}
		}
	}
	a = false;

	return res;

}




