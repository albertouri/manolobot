#include "GrupoBunkers.h"
#include <list>

//bool a = true;

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
	//angulo = a->calcularAngulo(choke);
	angulo = anguloGrupo;

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
			listBunkers.push_back(u);
		}
		else if (u->getType().getID() == Utilidades::ID_MISSILE_TURRET){
			listMisileTurrets.push_back(u);
		}
		else if (u->getType().getID() == Utilidades::ID_MARINE){
			listMarines.push_back(u);
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


// el bunker atacado es pasado como parametro
/*void GrupoBunkers::estrategia1(Unit *u){

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

				/*u->load(*It2);
				It2++;
			}
		}
	}
}*/


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

	//angulo = analizador->calcularAngulo(choke);
	//angulo1 = analizador->calcularAnguloGrupo(angulo);
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
	int cuadrante;
	TilePosition *t;
	int angulo1;
	int factY = 0;//, factX = 0;

	cuadrante = analizador->getCuadrante(reg->getCenter());
	//cuadrante = analizador->getOrientacion(choke, reg);
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

	resaltarUnidades();

	if (Broodwar->getFrameCount() % frameLatency == 0){

		if (getCantBunkers() > 1)
			ponerACubierto();

		ubicarModoSiege();
	}

	Graficos::dibujarCuadro(new TilePosition(posEncuentro->x() / 32, posEncuentro->y() / 32), 1, 1);
}


void GrupoBunkers::onUnitDestroy(Unit *u){
	if ((u->getType().getID() == Utilidades::ID_BUNKER) || (u->getType().getID() == Utilidades::ID_MISSILE_TURRET) || (u->getType().getID() == Utilidades::ID_TANKSIEGE) || (u->getType().getID() == Utilidades::ID_TANKSIEGE_SIEGEMODE))
		controlDestruidos();
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
	angulo1 = analizador->calcularAnguloGrupo(angulo);
	
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
				res = new TilePosition(t->x() /*- factorX*/, t->y());

				delete t;
				t = new TilePosition(p.x() / 32 + contX * factorX, p.y() / 32 + 2 + contY);

				if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
					// puedo construir el primer bunker en esa posicion, ahora verifico si puedo construir otro bunker al lado

					delete t;
					t = new TilePosition(p.x() / 32 + contX * factorX, p.y() / 32 - 2 + contY);

					if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
						//-- SETEA EL ANGULO UTILIZADO POR EL GRUPO DE BUNKERS (VARIABLE GLOBAL)
						anguloGrupo = 0;

						delete t;
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




