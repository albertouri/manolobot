#include "GrupoBunkers.h"

Region *e = NULL;

GrupoBunkers::GrupoBunkers(AnalizadorTerreno *a, Chokepoint *c, Region *r)
{
	int cuadrante;

	analizador = a;
	choke = c;
	reg = r;

	bunkerCentral = posicionPrimerBunker(reg, choke);

	//contadorMovimientos = 0;
	faltaMover = false;

	// ------------------------------------------------------------------------------------------
	// calcula la posicion de reunion de los soldados, a la cual se dirigiran si el bunker en el que estaban es destruido, para
	// liberar la posicion del bunker asi se puede reconstruir rapidamente

	posEncuentro = NULL;
	cuadrante = a->getCuadrante(reg->getCenter());

	if (bunkerCentral != NULL){
		switch (cuadrante){
			case 1:
				if (a->calcularAnguloGrupo(anguloGrupo) == 0)
					posEncuentro = new Position(bunkerCentral->x() * 32 - 192, bunkerCentral->y() * 32);
				else
					posEncuentro = new Position(bunkerCentral->x() * 32, bunkerCentral->y() * 32 - 192);
				break;
			case 2:
				if (a->calcularAnguloGrupo(anguloGrupo) == 0)
					posEncuentro = new Position(bunkerCentral->x() * 32 + 192, bunkerCentral->y() * 32);
				else
					posEncuentro = new Position(bunkerCentral->x() * 32, bunkerCentral->y() * 32 - 192);
				break;
			case 3:
				if (a->calcularAnguloGrupo(anguloGrupo) == 0)
					posEncuentro = new Position(bunkerCentral->x() * 32 - 192, bunkerCentral->y() * 32);
				else
					posEncuentro = new Position(bunkerCentral->x() * 32, bunkerCentral->y() * 32 + 192);
				break;
			case 4:
				if (a->calcularAnguloGrupo(anguloGrupo) == 0)
					posEncuentro = new Position(bunkerCentral->x() * 32 + 192, bunkerCentral->y() * 32);
				else
					posEncuentro = new Position(bunkerCentral->x() * 32, bunkerCentral->y() * 32 + 192);
				break;
		}
	}
	else
		Broodwar->printf("ERROR 002: No encuentro ubicacion posible para la posicion de encuentro");
	// ------------------------------------------------------------------------------------------

}


GrupoBunkers::GrupoBunkers(AnalizadorTerreno *a, Region *r, Region *regionEnemiga)
{
	e = regionEnemiga;
	Chokepoint *chokeEnemigo = NULL;
	Chokepoint *c2 = NULL;

	// direccion mantiene la orientacion que tendra el grupo de bunkers, la calcula desde la posicion del command center con respecto al/los chokepoint/s a defender
	// valores: 1: abajo - 2: izquierda - 3: arriba - 4: derecha
	int direccion = 0;

	TilePosition *inicio, *fin1, *fin2;
	inicio = new TilePosition(analizador->regionInicial()->getCenter().x() / TILE_SIZE, analizador->regionInicial()->getCenter().y() / TILE_SIZE);
	
	std::set<Chokepoint*>::const_iterator It = r->getChokepoints().begin();
	// busco el choke mas cercano a la region inicial, ese no debera defenderse y sera el choke de referencia
	while (It != r->getChokepoints().end()){
		if (c2 == NULL)
			c2 = (*It);
		else{
			fin1 = new TilePosition((*It)->getCenter().x() / TILE_SIZE, (*It)->getCenter().y() / TILE_SIZE);
			fin2 = new TilePosition(c2->getCenter().x() / TILE_SIZE, c2->getCenter().y() / TILE_SIZE);

			if ((BWTA::getGroundDistance(*inicio, *fin1)) < (BWTA::getGroundDistance(*inicio, *fin2)))
				c2 = (*It);

			delete fin1;
			delete fin2;
		}
		
		It++;
	}

	//-- si hay solo 2 chokepoints en la region, selecciona el mas lejano a la base y ahi ubica los bunkers (recordar que el mas cercano ya lo tenemos en la referencia c2)
	if (r->getChokepoints().size() == 2){
		reg = r;
		It = r->getChokepoints().begin();
		while (It != r->getChokepoints().end()){
			if ((*It) != c2)
				choke = (*It);
			It++;
		}

		cantChokes = 2;
		bunkerCentral = posicionPrimerBunker2(reg, choke, false, regionEnemiga);
	}
	else{
		//-- si hay mas de 2 chokepoints en la region, busca el mas lejano a la base y ahi ubica los bunkers
		cantChokes = r->getChokepoints().size();

		It = r->getChokepoints().begin();
		while (It != r->getChokepoints().end()){
			if ((*It) != c2){
				if (chokeEnemigo == NULL)
					chokeEnemigo = (*It);
				else{
					if (Broodwar->mapName() == "| iCCup | Destination 1.1"){
						if ((*It)->getCenter().x() > chokeEnemigo->getCenter().x())
							chokeEnemigo = (*It);
					}
					else{
						fin1 = new TilePosition((*It)->getCenter().x() / TILE_SIZE, (*It)->getCenter().y() / TILE_SIZE);
						fin2 = new TilePosition(chokeEnemigo->getCenter().x() / TILE_SIZE, chokeEnemigo->getCenter().y() / TILE_SIZE);
						
						//if ((BWTA::getGroundDistance(*inicio, *fin1)) > (BWTA::getGroundDistance(*inicio, *fin2)))
						if (abs((*It)->getCenter().x() - ((Broodwar->mapWidth() * TILE_SIZE) / 2)) < abs(chokeEnemigo->getCenter().x() - ((Broodwar->mapWidth() * TILE_SIZE) / 2)))
							chokeEnemigo = (*It);

						delete fin1;
						delete fin2;
					}
				}
			}

			It++;
		}

		reg = r;
		choke = chokeEnemigo;
		bunkerCentral = posicionPrimerBunker2(reg, choke, false, regionEnemiga);
	}

	delete inicio;

	analizador = a;
	//contadorMovimientos = 0;
	faltaMover = false;

	// ------------------------------------------------------------------------------------------
	// calcula la posicion de reunion de los soldados, a la cual se dirigiran si el bunker en el que estaban es destruido, para
	// liberar la posicion del bunker asi se puede reconstruir rapidamente

	posEncuentro = NULL;
	int cuadrante = a->getCuadrante(reg->getCenter());

	if (bunkerCentral != NULL){
		switch (cuadrante){
			case 1:
				if (a->calcularAnguloGrupo(anguloGrupo) == 0)
					posEncuentro = new Position(bunkerCentral->x() * 32 - 192, bunkerCentral->y() * 32);
				else
					posEncuentro = new Position(bunkerCentral->x() * 32, bunkerCentral->y() * 32 - 192);
				break;
			case 2:
				if (a->calcularAnguloGrupo(anguloGrupo) == 0)
					posEncuentro = new Position(bunkerCentral->x() * 32 + 192, bunkerCentral->y() * 32);
				else
					posEncuentro = new Position(bunkerCentral->x() * 32, bunkerCentral->y() * 32 - 192);
				break;
			case 3:
				if (a->calcularAnguloGrupo(anguloGrupo) == 0)
					posEncuentro = new Position(bunkerCentral->x() * 32 - 192, bunkerCentral->y() * 32);
				else
					posEncuentro = new Position(bunkerCentral->x() * 32, bunkerCentral->y() * 32 + 192);
				break;
			case 4:
				if (a->calcularAnguloGrupo(anguloGrupo) == 0)
					posEncuentro = new Position(bunkerCentral->x() * 32 + 192, bunkerCentral->y() * 32);
				else
					posEncuentro = new Position(bunkerCentral->x() * 32, bunkerCentral->y() * 32 + 192);
				break;
		}
	}
	else
		Broodwar->printf("ERROR 002: No encuentro ubicacion posible para la posicion de encuentro");
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
			listBunkers.push_back(u);
		}
		else if (u->getType().getID() == Utilidades::ID_MISSILE_TURRET){
			listMisileTurrets.push_back(u);
		}
		else if (u->getType().getID() == Utilidades::ID_MARINE){
			listMarines.push_back(u);

			if (posEncuentro != NULL)
				u->move(*posEncuentro);
		}
		else if (u->getType().getID() == Utilidades::ID_TANKSIEGE){

			if (!rellenarPosicionTanque(u)){
				Position *p;
				TilePosition *t;

				t = posicionNuevoTanque();

				if (anguloGrupo == 90)
					p = new Position(t->x() * 32 + 44 * listTanks.size(), t->y() * 32 + 16);
				else
					p = new Position(t->x() * 32 + 16, t->y() * 32 + 44 * listTanks.size() - 16);
				delete t;
				
				std::pair<Position*, Unit*> par = std::make_pair(p, u);
				listTanks.push_back(par);

				u->move(*p);
			}
		}
		else
			Broodwar->printf("No se puede agregar ese tipo de unidad a un grupo de bunkers");
	}
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

	std::list<std::pair<Position*, Unit*>>::iterator It1;
	It1 = listTanks.begin();
	int cont = 0;

	if (listTanks.size() > 0){
		while (It1 != listTanks.end()){
			if ((*It1).second->exists())
				cont++;
			
			It1++;
		}
	}

	return cont;
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

	int angulo1;

	if (!analizador->analisisListo())
		return NULL;

	angulo1 = anguloGrupo;

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
	
	if (reg != NULL){
		int cuadrante;
		TilePosition *t;
		int angulo1;
		int factY = 0;


		cuadrante = analizador->getCuadrante(reg->getCenter());
		angulo1 = anguloGrupo;

		//-- NUEVO CODIGO
		if (bunkerCentral != NULL){
			switch (cuadrante){
				//-- PRIMER CUADRANTE (ARRIBA A LA IZQUIERDA)
				case 1:
					if (angulo1 == 90){

						//-- NUEVO CODIGO
						if (choke->getCenter().y() < reg->getCenter().y())
							factY = 2;
						else
							factY = -2;

						// si el angulo del grupo de bunkers es 90 grados, intenta las 2 posiciones posibles para ubicar las misile turrets
						t = new TilePosition(bunkerCentral->x() + 4, bunkerCentral->y() + factY);
						
						//-- intenta en posicion 1
						if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
							return t;
						else{
							delete t;
							t = new TilePosition(bunkerCentral->x() - 3, bunkerCentral->y() + factY);

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
						//-- el angulo es 0

						/*//-- NUEVO CODIGO
						if (choke->getCenter().x() < reg->getCenter().x())
							factX = 3;
						else
							factX = -2;*/
						
						t = new TilePosition(bunkerCentral->x() -2, bunkerCentral->y() - 3);
						
						//-- intenta en posicion 1
						if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
							return t;
						else{
							delete t;
							t = new TilePosition(bunkerCentral->x() -2, bunkerCentral->y() + 3);

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
						t = new TilePosition(bunkerCentral->x() + 4, bunkerCentral->y() -2);
						
						//-- intenta en posicion 1
						if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
							return t;
						else{
							delete t;
							t = new TilePosition(bunkerCentral->x() - 3, bunkerCentral->y() -2);

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
						// el angulo es 0

						t = new TilePosition(bunkerCentral->x() +3, bunkerCentral->y() - 3);
						
						//-- intenta en posicion 1
						if (!ocupado(*t, Utilidades::ID_MISSILE_TURRET))
							return t;
						else{
							delete t;
							t = new TilePosition(bunkerCentral->x() +3, bunkerCentral->y() + 3);

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

	}
	return NULL;
}



//-- CODIGO PARA UBICAR TANQUES
bool GrupoBunkers::rellenarPosicionTanque(Unit *u){
	std::list<std::pair<Position*, Unit*>>::iterator It1;
	Position *aux;

	It1 = listTanks.begin();
	while (It1 != listTanks.end()){
		if (!(*It1).second->exists()){
			aux = (*It1).first;
			(*It1) = std::make_pair(aux, u);
			return true;
		}

		It1++;
	}
	return false;
}

TilePosition* GrupoBunkers::posicionNuevoTanque(){
	if (reg != NULL){
		int cuadrante, angulo1;
		
		angulo1 = anguloGrupo;
		cuadrante = analizador->getCuadrante(reg->getCenter());

		if (bunkerCentral != NULL){
			// el grupo de bunkers esta ubicado en forma horizontal
			if (angulo1 == 90){
				//if ((cuadrante == 1) || (cuadrante == 2))
					if (choke->getCenter().y() < reg->getCenter().y())
						return new TilePosition(bunkerCentral->x(), bunkerCentral->y() + 3);
					else
						return new TilePosition(bunkerCentral->x(), bunkerCentral->y() - 2);
				//else{
					//if (choke->getCenter().y() < reg->getCenter().y())
						//return new TilePosition(bunkerCentral->x(), bunkerCentral->y() + 3);
					/*else
						return new 
				}*/
			}
			else if (angulo1 == 0){
				if ((cuadrante == 1) || (cuadrante == 3))
					return new TilePosition(bunkerCentral->x() - 2, bunkerCentral->y());
				else
					return new TilePosition(bunkerCentral->x() + 4, bunkerCentral->y());
			}
			else
				return NULL;
		}
		else{
			Broodwar->printf("ERROR: No se encontro posicion - Metodo: posicionNuevoTanque - Clase: GrupoBunkers");
			return NULL;
		}
	}

	return NULL;
}


void GrupoBunkers::ubicarModoSiege(){
	std::list<std::pair<Position*, Unit*>>::iterator It;

	It = listTanks.begin();

	while (It != listTanks.end()){
		if (((*It).second->exists()) && (*It).second->isIdle() && (!(*It).second->isSieged()) && ((*It).first->x() == (*It).second->getPosition().x()) && ((*It).first->x() == (*It).second->getPosition().x()))
			(*It).second->siege();
		else
			(*It).second->move(*((*It).first));

		It++;
	}
}

//-- FIN CODIGO PARA UBICAR TANQUES



void GrupoBunkers::controlDestruidos(){
	std::list<Unit*>::iterator It1;
	It1 = listBunkers.begin();
	int cont = 1;

	// ----------- controla los bunkers -----------
	while (It1 != listBunkers.end()){
		if (!(*It1)->exists()){
			listBunkers.erase(It1);
			It1 = listBunkers.begin(); // tuve que poner esto porque sino se colgaba el while...

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

	/*if (e != NULL)
		posicionPrimerBunker2(reg, choke, false, e);*/

	/*if (choke != NULL)
		Broodwar->drawLineMap(choke->getCenter().x(), choke->getCenter().y(), analizador->regionInicial()->getCenter().x(), analizador->regionInicial()->getCenter().y(), Colors::White);
*/
	//resaltarUnidades();

	if ((Broodwar->getFrameCount() % 24 == 0) && (faltaMover))
		moverSoldadosPosEncuentro();

	if (Broodwar->getFrameCount() % frameLatency == 0){
		if (getCantBunkers() > 1)
			ponerACubierto();

		ubicarModoSiege();
	}

	//dibujarPosiciones();
}


void GrupoBunkers::onUnitDestroy(Unit *u){
	if (u->getType().getID() == Utilidades::ID_BUNKER){
		if (perteneceBunker(u)){
			controlDestruidos();
			//contadorMovimientos++;
		}
	}
	else if ((u->getType().getID() == Utilidades::ID_MISSILE_TURRET) || (u->getType().getID() == Utilidades::ID_TANKSIEGE) || (u->getType().getID() == Utilidades::ID_TANKSIEGE_SIEGEMODE))
		controlDestruidos();
}


bool GrupoBunkers::faltanMarines(){
	return (getCantMarines() < 12);
}

bool GrupoBunkers::faltanTanques(){
	return (getCantTanks() < 3);
}

bool GrupoBunkers::faltanMisileTurrets(){
	return (getCantMisileTurrets() < 2);
}

void GrupoBunkers::moverSoldadosPosEncuentro(){
	bool estanAtacando = false;

	if (posEncuentro != NULL){
		std::list<Unit*>::iterator It1;

		It1 = listMarines.begin();

		while (It1 != listMarines.end()){
			if (((*It1)->exists()) && ((*It1)->isCompleted()) && (!(*It1)->isLoaded()))
				estanAtacando = estanAtacando || (*It1)->isAttacking();

			It1++;
		}

		if (!estanAtacando){
			It1 = listMarines.begin();

			while (It1 != listMarines.end()){
				if (((*It1)->exists()) && ((*It1)->isCompleted()) && (!(*It1)->isLoaded()))
					(*It1)->move(*posEncuentro);

				It1++;
			}
			faltaMover = false;
		}
		else
			faltaMover = true;
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

	std::list<std::pair<Position*, Unit*>>::iterator It2;
	It2 = listTanks.begin();

	while (It2 != listTanks.end()){
		if (((*It2).second->isCompleted()) && (!(*It2).second->isLoaded()))
			Graficos::resaltarUnidad((*It2).second, Colors::White);

		It2++;
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
	//cuadrante = analizador->getOrientacion(c, r);

	angulo = analizador->calcularAngulo(c);
	res = encontrarPosicion(cuadrante, c->getCenter(), angulo);

	return res;
}

TilePosition* GrupoBunkers::posicionPrimerBunker2(Region* r, Chokepoint* c, bool buscarHaciaAdentro, Region* regionEnemiga){
	int cuadrante = 0;
	int angulo = analizador->calcularAngulo(c);
	cuadrante = analizador->getCuadrante(r->getCenter());
	int angulo1;// = analizador->calcularAnguloGrupo(angulo);
	
	if (cantChokes == 2){
		if ((angulo >= 135) && (angulo <= 179))
			angulo1 = 0;
		else if ((angulo <= 45) && (angulo >= 0))
			angulo1 = 0;
		else
			angulo1 = 90;
	}
	else{
		if (abs(r->getCenter().x() - regionEnemiga->getCenter().x()) < abs(r->getCenter().y() - regionEnemiga->getCenter().y()))
			angulo1 = 90;
		else
			angulo1 = 0;
	}
	
	return encontrarPosicion2(cuadrante, c->getCenter(), angulo1, buscarHaciaAdentro);
}


TilePosition* GrupoBunkers::encontrarPosicion2(int cuadrante, Position p, int angulo, bool buscarHaciaAdentro){
	int factorX, factorY;
	//int contX = 0, contY = 0;
	int contX = 8, contY = 4;
	TilePosition *t, *res = NULL;
	bool hayPosicion;

	//TilePosition *turr1, *turr2;

	UnitType *bunker = new UnitType(Utilidades::ID_BUNKER);
	UnitType *missileTurret = new UnitType(Utilidades::ID_MISSILE_TURRET);

	// calcula el cuadrante donde se ubicara el grupo de bunkers
	if (cuadrante == 1){
		if ((choke->getCenter().y() < reg->getCenter().y()) && (angulo == 90)){
			factorX = -1;
			factorY = -1;
		}
		else{
			factorX = 1;
			factorY = 1;
		}
	}
	else if (cuadrante == 2){
		factorX = -1;
		factorY = 1;
	}
	else if (cuadrante == 3){
		factorX = 1;
		factorY = -1;
	}
	else if (cuadrante == 4){
		factorX = -1;
		factorY = -1;
	}

	//Broodwar->printf("El cuadrante es: %d, angulo1: %d", cuadrante, angulo);

	if (buscarHaciaAdentro){
		factorX *= -1;
		factorY *= -1;
	}

	if (angulo == 90){
		// si el angulo del chokepoint es totalmente horizontal, intenta construir 3 bunkers alineados sobre el chokepoint,
		// si no puede se mueve un tile en el eje Y, e intenta nuevamente
		// contY lleva la cuenta de los tiles que se movio sobre el ejeY
		// para cada posicion sobre el eje Y, prueba en 5 posiciones sobre el eje X:
		// desde el centro del choquepoint - 2 buildtiles hasta el centro del choquepoint + 2 buildtiles

		if ((cuadrante == 1) || (cuadrante == 3))
			contX = -2;
		else
			contX = 2;


		while (contY < 20){
			//-- Posicion del bunker central
			t = new TilePosition((p.x() / TILE_SIZE) + contX, (p.y() / TILE_SIZE) + contY * factorY);
			//Broodwar->drawBoxMap(t->x() * TILE_SIZE, t->y() * TILE_SIZE, t->x() * TILE_SIZE + 8, t->y() * TILE_SIZE + 8, Colors::Green, true);
			//Broodwar->printf("horizontal");

			//Broodwar->printf("Intento en x: %d - y: %d", (p.x() / TILE_SIZE) + contX, (p.y() / TILE_SIZE) + contY * factorY);
			if (t->isValid())
				hayPosicion = puedoConstruir2(*t, *bunker);
			else
				hayPosicion = false;

			//-- Posicion del bunker derecho
			if (hayPosicion){
				// verifico si puedo construir un bunker a la derecha
				//res = new TilePosition(t->x(), t->y() + factorY);
				res = new TilePosition(t->x(), t->y());
				delete t;
				t = new TilePosition((p.x() / TILE_SIZE) + 3 + contX, (p.y() / TILE_SIZE) + contY * factorY);

				if (t->isValid())
					hayPosicion = puedoConstruir2(*t, *bunker);
				else
					hayPosicion = false;
			}

			//-- Posicion del misile turret derecho
			if (hayPosicion){
				// verifico si puedo construir un misile turret detras del bunker derecho
				delete t;
				//if ((cuadrante == 1) || (cuadrante == 2))
				if (p.y() > reg->getCenter().y())
					t = new TilePosition((p.x() / TILE_SIZE) + 3 + contX + 1, (p.y() / TILE_SIZE) + contY * factorY - 2);
				else
					t = new TilePosition((p.x() / TILE_SIZE) + 3 + contX + 1, (p.y() / TILE_SIZE) + contY * factorY + 2);

				if (t->isValid())
					hayPosicion = puedoConstruir2(*t, *missileTurret);
				else
					hayPosicion = false;
			}

			//-- Posicion del bunker izquierdo
			if (hayPosicion){
				// verifico si puedo construir un bunker a la izquierda
				delete t;
				t = new TilePosition((p.x() / TILE_SIZE) - 3 + contX, (p.y() / TILE_SIZE) + contY * factorY);

				if (t->isValid())
					hayPosicion = puedoConstruir2(*t, *bunker);
				else
					hayPosicion = false;
			}

			//-- Posicion del misile turret izquierdo
			if (hayPosicion){
				delete t;
				//if ((cuadrante == 1) || (cuadrante == 2))
				if (p.y() > reg->getCenter().y())
					t = new TilePosition((p.x() / TILE_SIZE) - 3 + contX, (p.y() / TILE_SIZE) + contY * factorY - 2);
				else
					t = new TilePosition((p.x() / TILE_SIZE) - 3 + contX, (p.y() / TILE_SIZE) + contY * factorY + 2);
				
				if (t->isValid())
					hayPosicion = puedoConstruir2(*t, *missileTurret);
				else
					hayPosicion = false;
			}

			//-- Resultado final
			if (hayPosicion){
				//-- SETEA EL ANGULO UTILIZADO POR EL GRUPO DE BUNKERS (VARIABLE GLOBAL)
				anguloGrupo = 90;
				delete t;

				//-- RETORNA LA POSICION DEL PRIMER BUNKER A CONSTRUIR
				delete bunker;
				delete missileTurret;
				return res;
			}
			else delete t;

			//-- ACTUALIZA LOS CONTADORES
			// si el cuadrante esta en la mitad izquierda de la pantalla, busca posicion de izquierda a derecha, de lo contrario busca al reves
			if ((cuadrante == 1) || (cuadrante == 3)){
				if (contX == /*2*/4){
					contY++;
					contX = /*-2*/-4;
				}
				else
					contX++;
			}
			else{
				if (contX == /*-2*/-4){
					contY++;
					contX = /*2*/4;
				}
				else
					contX--;
			}

			if (res != NULL){
				delete res;
				res = NULL;
			}
		}
	}
	else if (angulo == 0){
		contY = /*-3*/-10;

		//-- CODIGO GIRATORIO
		// busco el punto mas a la izquierda y mas a la derecha de la region
		int i = 0, d = 0;

		BWTA::Polygon pol=reg->getPolygon();
		for(int j = 0; j < (int)pol.size(); j++){
			if ((pol[j].y() > (p.y() - 3 * TILE_SIZE)) && (pol[j].y() < (p.y() + 3 * TILE_SIZE))){
				if (pol[j].x() < pol[i].x())
					i = j;
				
				if (pol[j].x() > pol[d].x())
					d = j;
			}
		}

		//-- Verifica si hay espacio suficiente entre el borde del mapa y el grupo de bunkers
		//bool condicion1 = ((cuadrante == 1) || (cuadrante == 3)) && ((p.x() / 32) > 12);
		//bool condicion2 = ((cuadrante == 2) || (cuadrante == 4)) && ((p.x() / 32) < (Broodwar->mapWidth() - 11));
		bool condicion1 = ((cuadrante == 1) || (cuadrante == 3)) && (((p.x() - pol[i].x()) / 32) > 12);
		bool condicion2 = ((cuadrante == 2) || (cuadrante == 4)) && ((p.x() / 32) < ((pol[d].x() / 32) - 11));

		// si el bunker central esta separado mas de 11 build tiles del borde esta OK, sino lo intenta ubicar en forma horizontal
		if (!(condicion1 || condicion2)){
			//Broodwar->printf("Intenta girar el grupo");
			return encontrarPosicion2(cuadrante, p, 90, buscarHaciaAdentro);
		}

		//-- FIN CODIGO GIRATORIO

		while (contX < 20){
			//-- Posicion del bunker central
			t = new TilePosition(p.x() / 32 + contX * factorX, p.y() / 32 + contY);
			//Broodwar->drawBoxMap(t->x() * TILE_SIZE, t->y() * TILE_SIZE, t->x() * TILE_SIZE + 8, t->y() * TILE_SIZE + 8, Colors::Green, true);
			//Broodwar->printf("Intento en x: %d - y: %d", (p.x() / TILE_SIZE) + contX, (p.y() / TILE_SIZE) + contY * factorY);
			//Broodwar->printf("VERTICAL");
			hayPosicion = puedoConstruir2(*t, *bunker);

			//-- Posicion del bunker inferior
			if (hayPosicion){
				// verifico si puedo construir un bunker arriba
				res = new TilePosition(t->x(), t->y());
				delete t;
				t = new TilePosition(p.x() / 32 + contX * factorX, p.y() / 32 + 2 + contY);
				hayPosicion = puedoConstruir2(*t, *bunker);
			}

			//-- Posicion del misile turret inferior
			if (hayPosicion){
				delete t;
				//if ((cuadrante == 1) || (cuadrante == 3))
				if (p.x() > reg->getCenter().x())
					t = new TilePosition(p.x() / 32 + contX * factorX - 2, p.y() / 32 + 3 + contY);
				else
					t = new TilePosition(p.x() / 32 + contX * factorX + 3, p.y() / 32 + 3 + contY);
				
				//turr1 = new TilePosition(t->x(), t->y());
				//Broodwar->drawBoxMap(t->x() * TILE_SIZE, t->y() * TILE_SIZE, (t->x() + 2) * TILE_SIZE + 8, (t->y() + 2) * TILE_SIZE + 8, Colors::Red, false);
				hayPosicion = puedoConstruir2(*t, *missileTurret);
			}

			//-- Posicion del bunker superior
			if (hayPosicion){
				// verifico si puedo construir un bunker arriba
				delete t;
				t = new TilePosition(p.x() / 32 + contX * factorX, p.y() / 32 - 2 + contY);
				hayPosicion = puedoConstruir2(*t, *bunker);
			}

			//-- Posicion del misile turret superior
			if (hayPosicion){
				delete t;
				if (p.x() > reg->getCenter().x())
					t = new TilePosition(p.x() / 32 + contX * factorX - 2, p.y() / 32 - 3 + contY);
				else
					t = new TilePosition(p.x() / 32 + contX * factorX + 3, p.y() / 32 - 3 + contY);
				
				//turr2 = new TilePosition(t->x(), t->y());
				//Broodwar->drawBoxMap(t->x() * TILE_SIZE, t->y() * TILE_SIZE, (t->x() + 2) * TILE_SIZE + 8, (t->y() + 2) * TILE_SIZE + 8, Colors::Red, false);
				hayPosicion = puedoConstruir2(*t, *missileTurret);
			}

			if (hayPosicion){
				//-- SETEA EL ANGULO UTILIZADO POR EL GRUPO DE BUNKERS (VARIABLE GLOBAL)
				anguloGrupo = 0;
				delete t;

				//Broodwar->drawBoxMap(turr1->x() * TILE_SIZE, turr1->y() * TILE_SIZE, (turr1->x() + 2) * TILE_SIZE + 8, (turr1->y() + 2) * TILE_SIZE + 8, Colors::Red, false);
				//Broodwar->drawBoxMap(turr2->x() * TILE_SIZE, turr2->y() * TILE_SIZE, (turr2->x() + 2) * TILE_SIZE + 8, (turr2->y() + 2) * TILE_SIZE + 8, Colors::Red, false);

				TilePosition *aux = res;
				if ((cuadrante == 1) || (cuadrante == 3))
					res = new TilePosition(aux->x()/* - 1*/, aux->y());
				else
					res = new TilePosition(aux->x()/* + 1*/, aux->y());

				delete aux;
				//-- RETORNA LA POSICION DEL PRIMER BUNKER A CONSTRUIR
				delete bunker;
				delete missileTurret;
				return res;
			}
			else delete t;

			if (contY == /*3*/10){
				contX++;
				contY = /*-3*/-10;
			}
			else
				contY++;

			if (res != NULL){
				delete res;
				res = NULL;
			}
		}
	}

	delete bunker;
	delete missileTurret;
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

bool GrupoBunkers::puedoConstruir2(TilePosition t, UnitType tipo){
	bool res = true;
	//TilePosition *aux;

	// verifica si la posicion esta dentro del mapa
	if ((t.x() > Broodwar->mapWidth()) || (t.y() > Broodwar->mapHeight()))
		return false;

	//-- Posiciones que representan los bordes de la unidad a construir
	Position *si, *sd, *ii, *id;
	si = new Position(t.x() * TILE_SIZE, t.y() * TILE_SIZE);
	sd = new Position((t.x() + tipo.tileWidth()) * TILE_SIZE, t.y() * TILE_SIZE);
	ii = new Position(t.x() * TILE_SIZE, (t.y() + tipo.tileHeight()) * TILE_SIZE);
	id = new Position((t.x() + tipo.tileWidth()) * TILE_SIZE, (t.y() + tipo.tileHeight()) * TILE_SIZE);

	// verifica si los bordes estan en alguna de las regiones conectadas por el chokepoint a defender
	res = ((choke->getRegions().first->getPolygon().isInside(*si) || choke->getRegions().second->getPolygon().isInside(*si))); 
	res = res && ((choke->getRegions().first->getPolygon().isInside(*sd) || choke->getRegions().second->getPolygon().isInside(*sd)));
	res = res && ((choke->getRegions().first->getPolygon().isInside(*ii) || choke->getRegions().second->getPolygon().isInside(*ii)));
	res = res && ((choke->getRegions().first->getPolygon().isInside(*id) || choke->getRegions().second->getPolygon().isInside(*id)));

	for (int x = 0; x < tipo.tileWidth(); x++){
		for (int y = 0; y < tipo.tileHeight(); y++){
			//aux = new TilePosition(t.x() + x, t.y() + y);
			res = res && Broodwar->isBuildable(t.x() + x, t.y() + y);
			//res = res && Broodwar->isBuildable(*aux);
			//delete aux;
		}
	}

	delete si;
	delete sd;
	delete ii;
	delete id;
	return res;
}

bool GrupoBunkers::ocupado(TilePosition t, int IDTipo){
	bool ocupado = false;

	//-- recorre la lista de unidades en el tile para verificar si hay una unidad de tipo IDTipo
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
	//angulo1 = analizador->calcularAnguloGrupo(angulo);

	// NUEVO
	if ((angulo >= 112/*135*/) && (angulo <= 179))
		angulo1 = 0;
	else if ((angulo <= 67/*45*/) && (angulo >= 0))
		angulo1 = 0;
	else
		angulo1 = 90;

	// FIN NUEVO

	// calcula el cuadrante donde se ubicara el grupo de bunkers
	if (cuadrante == 1){
		//-- NUEVO CODIGO
		if ((choke->getCenter().y() < reg->getCenter().y()) && (angulo1 == 90)){
			factorX = 1;
			factorY = 1;
		}
		else{
		//-- FIN NUEVO CODIGO
			factorX = -1;
			factorY = -1;
		}
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

	//Broodwar->printf("angulo1: %d", angulo1);
	//Broodwar->printf("angulo real: %d", angulo);

	if (angulo1 == 90){
		// si el angulo del chokepoint es totalmente horizontal, intenta construir 3 bunkers alineados sobre el chokepoint,
		// si no puede se mueve un tile en el eje Y, e intenta nuevamente
		// contY lleva la cuenta de los tiles que se movio sobre el ejeY
		// para cada posicion sobre el eje Y, prueba en 5 posiciones sobre el eje X:
		// desde el centro del choquepoint - 2 buildtiles hasta el centro del choquepoint + 2 buildtiles

		if ((cuadrante == 1) || (cuadrante == 3))
			contX = -2;
		else
			contX = 2;

		while (contY < 10){
			t = new TilePosition(p.x() / 32 + contX, p.y() / 32 + contY * factorY);

			if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
				// puedo construir el primer bunker en esa posicion, ahora verifico si puedo construir otro bunker al lado
				res = new TilePosition(t->x(), t->y() + factorY);

				delete t;
				t = new TilePosition(p.x() / 32 + 3 + contX, p.y() / 32 + contY * factorY);

				if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
					// puedo construir el primer bunker en esa posicion, ahora verifico si puedo construir otro bunker al lado
					delete t;
					t = new TilePosition(p.x() / 32 - 3 + contX, p.y() / 32 + contY * factorY);

					if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
						//-- SETEA EL ANGULO UTILIZADO POR EL GRUPO DE BUNKERS (VARIABLE GLOBAL)
						anguloGrupo = 90;

						delete t;

						TilePosition *aux = res;
						if ((cuadrante == 1) || (cuadrante == 2))
							res = new TilePosition(aux->x(), aux->y() - 1);
						else
							res = new TilePosition(aux->x(), aux->y() + 1);

						delete aux;

						//-- RETORNA LA POSICION DEL PRIMER BUNKER A CONSTRUIR
						return res;
					}
					else delete t;
				}
				else delete t;
			}
			else delete t;

			// si el cuadrante esta en la mitad izquierda de la pantalla, busca posicion de izquierda a derecha, de lo contrario busca al reves
			if ((cuadrante == 1) || (cuadrante == 3))
			{
				if (contX == 2){
					contY++;
					contX = -2;
				}
				else
					contX++;
			}
			else
			{
				if (contX == -2){
					contY++;
					contX = 2;
				}
				else
					contX--;
			}


			if (res != NULL){
				delete res;
				res = NULL;
			}
		}
	}
	else if (angulo1 == 0){
		contY = -3/*-2*/;

		//-- CODIGO GIRATORIO

		//-- Verifica si hay espacio suficiente entre el borde del mapa y el grupo de bunkers
		bool condicion1 = ((cuadrante == 1) || (cuadrante == 3)) && ((p.x() / 32) > 12);
		bool condicion2 = ((cuadrante == 2) || (cuadrante == 4)) && ((p.x() / 32) < (Broodwar->mapWidth() - 11));

		// si el bunker central esta separado mas de 11 build tiles del borde esta OK, sino lo intenta ubicar en forma horizontal
		if (!(condicion1 || condicion2)){
			Broodwar->printf("Intenta girar el grupo");
			return encontrarPosicion(cuadrante, p, 90);
		}

		//-- FIN CODIGO GIRATORIO

		while (contX < 10){
			t = new TilePosition(p.x() / 32 + contX * factorX, p.y() / 32 + contY);

			if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
				// puedo construir el primer bunker en esa posicion, ahora verifico si puedo construir otro bunker al lado
				if ((cuadrante == 1) || (cuadrante == 3))
					res = new TilePosition(t->x() - 1, t->y());
				else
					res = new TilePosition(t->x() + 1, t->y());

				delete t;
				t = new TilePosition(p.x() / 32 + contX * factorX, p.y() / 32 + 2 + contY/* + 1*/);

				if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
					// puedo construir el primer bunker en esa posicion, ahora verifico si puedo construir otro bunker al lado

					delete t;
					t = new TilePosition(p.x() / 32 + contX * factorX, p.y() / 32 - 2 + contY/* - 1*/);

					if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
						//-- SETEA EL ANGULO UTILIZADO POR EL GRUPO DE BUNKERS (VARIABLE GLOBAL)
						anguloGrupo = 0;

						delete t;

						TilePosition *aux = res;
						if ((cuadrante == 1) || (cuadrante == 3))
							res = new TilePosition(aux->x() - 1, aux->y());
						else
							res = new TilePosition(aux->x() + 1, aux->y());

						delete aux;
						//-- RETORNA LA POSICION DEL PRIMER BUNKER A CONSTRUIR
						return res;

					}
					else delete t;
				}
				else delete t;
			}
			else delete t;

			if (contY == 3/*2*/){
				contX++;
				contY = -3/*-2*/;
			}
			else
				contY++;

			if (res != NULL){
				delete res;
				res = NULL;
			}
		}
	}

	return res;
}

int GrupoBunkers::getAngulo(){
	//return anguloGrupo;
	return analizador->calcularAngulo(choke);
}


TilePosition* GrupoBunkers::getTileBunkerCentral(){
	return bunkerCentral;
}

Chokepoint* GrupoBunkers::getChoke(){
	return choke;
}

bool GrupoBunkers::perteneceMarine(Unit *u){
	std::list<Unit*>::iterator It;

	It = listMarines.begin();
	while (It != listMarines.end()){
		if (((*It)->exists()) && (u->exists()) && ((*It)->getID() == u->getID()))
			return true;
		It++;
	}

	return false;
}

bool GrupoBunkers::faltanBunkers(){
	return (getCantBunkers() < 3);
}

int GrupoBunkers::cantMaximaTurrets(){
	return 2;
}

Region* GrupoBunkers::getRegion(){
	return reg;
}


void GrupoBunkers::dibujarPosiciones(){

	TilePosition *t111 = NULL;
	TilePosition *t222 = NULL;
	TilePosition *t333 = NULL;
	
	t111 = posicionNuevoBunker();
	
	if (t111 != NULL)
		Graficos::dibujarCuadro(t111, 3, 2);

	t222 = posicionNuevaTorreta();
	if (t222 != NULL)
		Graficos::dibujarCuadro(t222, 2, 2);

	t333 = posicionNuevoTanque();		
	if (t333 != NULL){
		Graficos::dibujarCuadro(t333, 1, 1);
		delete t333;
	}

	if (posEncuentro != NULL)
		Graficos::dibujarCuadro(new TilePosition(posEncuentro->x() / 32, posEncuentro->y() / 32), 1, 1);
}