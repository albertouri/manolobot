#include "GrupoAntiaereo.h"
#include "Graficos.h"

TilePosition *tArr = NULL, *tAba = NULL, *tDer = NULL, *tIzq = NULL;
Region *regionDefender = NULL;

GrupoAntiaereo::GrupoAntiaereo(Region *r)
{
	cantidadNecesariaTurrets = 0;

	Chokepoint *defendido = NULL;
	std::set<Chokepoint*>::const_iterator It = r->getChokepoints().begin();

	// recorre la lista de chokepoints de la region y encuentra el que debe estar defendido por un grupo de bunkers
	while (It != r->getChokepoints().end()){

		// revisa si el iterador actualmente apunta a una region que no es la inicial
		if ((*It)->getRegions().first != r){
			if ((*It)->getRegions().first->getChokepoints().size() > 1){
				defendido = (*It);
				break;
			}
		}
		else{
			if ((*It)->getRegions().second->getChokepoints().size() > 1){
				defendido = (*It);
				break;
			}
		}

		It++;
	}

	// una vez ubicado el chokepoint que debe estar defendido por un grupo de bunkers, calcula las posiciones donde se deberian ubicar misile turrets
	std::vector<Position>::const_iterator It2;
	Position masIzq, masDer, masArr, masAba;
	Position acotadaIzq, acotadaDer, acotadaArr, acotadaAba;

	acotadaIzq = r->getCenter();
	acotadaDer = r->getCenter();
	acotadaArr = r->getCenter();
	acotadaAba = r->getCenter();

	masIzq = r->getCenter();
	masDer = r->getCenter();
	masArr = r->getCenter();
	masAba = r->getCenter();

	// valor maximo del cual pueden estar alejados los puntos que encuentre con respecto al centro de la region
	int rango = 3;

	It2 = r->getPolygon().begin();
	while (It2 != r->getPolygon().end()){
		// posiciones mas a la izquierda, mas a la derecha, etc...
		//Position *masIzq = NULL, *masDer = NULL, *masArr = NULL, *masAba = NULL;
		
		if ((*It2).x() < masIzq.x())
			masIzq = (*It2);

		if ((*It2).x() > masDer.x())
			masDer = (*It2);

		if ((*It2).y() < masArr.y())
			masArr = (*It2);

		if ((*It2).y() > masAba.y())
			masAba = (*It2);

		//-- Calcula las posiciones acotadas
		if (((*It2).x() < acotadaIzq.x()) && ((*It2).y() > r->getCenter().y() - TILE_SIZE * rango) &&  ((*It2).y() < r->getCenter().y() + TILE_SIZE * rango))
			acotadaIzq = (*It2);

		if (((*It2).x() > acotadaDer.x()) && ((*It2).y() > r->getCenter().y() - TILE_SIZE * rango) &&  ((*It2).y() < r->getCenter().y() + TILE_SIZE * rango))
			acotadaDer = (*It2);

		if (((*It2).y() < acotadaArr.y()) && ((*It2).x() > r->getCenter().x() - TILE_SIZE * rango) &&  ((*It2).x() < r->getCenter().x() + TILE_SIZE * rango))
			acotadaArr = (*It2);

		if (((*It2).y() > acotadaAba.y()) && ((*It2).x() > r->getCenter().x() - TILE_SIZE * rango) &&  ((*It2).x() < r->getCenter().x() + TILE_SIZE * rango))
			acotadaAba = (*It2);

		It2++;
	}

	// calculo las posiciones donde se debera ubicar el misile turret central
	if (!r->getPolygon().empty()){
		if (masIzq.x() > 128){

			// si el chokepoint no esta a la izquierda, y esta desplazado mas sobre el eje Y que sobre el eje X con respecto al centro de la region, defiende a la izquierda
			if ((defendido != NULL) && ((defendido->getCenter().x() > r->getCenter().x()) || (abs(defendido->getCenter().x() - r->getCenter().x()) < abs(defendido->getCenter().y() - r->getCenter().y())))){
				//-- Encuentra la posicion exacta a defender
				tIzq = encontrarPosicion(IZQUIERDA, acotadaIzq);
				cantidadNecesariaTurrets += 3;
			}
		}

		if (masDer.x() < (Broodwar->mapWidth() * TILE_SIZE - 128)){
			if ((defendido != NULL) && ((defendido->getCenter().x() < r->getCenter().x()) || (abs(defendido->getCenter().x() - r->getCenter().x()) < abs(defendido->getCenter().y() - r->getCenter().y())))){
				tDer = encontrarPosicion(DERECHA, acotadaDer);
				cantidadNecesariaTurrets += 3;
			}
		}

		if (masArr.y() > 128){
			if ((defendido != NULL) && ((defendido->getCenter().y() > r->getCenter().y()) || (abs(defendido->getCenter().x() - r->getCenter().x()) > abs(defendido->getCenter().y() - r->getCenter().y())))){
				tArr = encontrarPosicion(ARRIBA, acotadaArr);
				cantidadNecesariaTurrets += 3;
			}
		}

		if (masAba.y() < (Broodwar->mapHeight() * TILE_SIZE - 128)){
			if ((defendido != NULL) && ((defendido->getCenter().y() < r->getCenter().y()) || (abs(defendido->getCenter().x() - r->getCenter().x()) > abs(defendido->getCenter().y() - r->getCenter().y())))){
				tAba = encontrarPosicion(ABAJO, acotadaAba);
				cantidadNecesariaTurrets += 3;
			}
		}
	}

	regionDefender = r;
}


GrupoAntiaereo::~GrupoAntiaereo(void)
{
}


void GrupoAntiaereo::onFrame(){
	if (tIzq != NULL)
		Graficos::dibujarCuadro(tIzq, 2, 2, Colors::Cyan, false);

	if (tDer != NULL)
		Graficos::dibujarCuadro(tDer, 2, 2, Colors::Cyan, false);

	if (tArr != NULL)
		Graficos::dibujarCuadro(tArr, 2, 2, Colors::Cyan, false);

	if (tAba != NULL)
		Graficos::dibujarCuadro(tAba, 2, 2, Colors::Cyan, false);

	Broodwar->drawBox(CoordinateType::Map, regionDefender->getCenter().x(), regionDefender->getCenter().y(), regionDefender->getCenter().x() + 10, regionDefender->getCenter().y() + 10, Colors::Red, true);
	
}


bool GrupoAntiaereo::puedoConstruir(TilePosition t, UnitType tipo){
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


TilePosition *GrupoAntiaereo::encontrarPosicion(int orientacion, Position posCentral){

	if (orientacion == IZQUIERDA){

		TilePosition *aux;
		UnitType *tipo = new UnitType(Utilidades::ID_MISSILE_TURRET);
		int offsetX = 0;
		
		while (offsetX < 10){
			//Broodwar->printf("Busca");
			aux = new TilePosition(posCentral.x() / TILE_SIZE + offsetX, posCentral.y() / TILE_SIZE);

			if (puedoConstruir(*aux, *tipo)){
				delete aux;
				aux = new TilePosition(posCentral.x() / TILE_SIZE + offsetX, posCentral.y() / TILE_SIZE - 2);

				if (puedoConstruir(*aux, *tipo)){
					delete aux;					
					aux = new TilePosition(posCentral.x() / TILE_SIZE + offsetX, posCentral.y() / TILE_SIZE + 2);

					if (puedoConstruir(*aux, *tipo)){
						delete aux;
						delete tipo;

						//-- ENCONTRO UNA POSICION POSIBLE
						return (new TilePosition(posCentral.x() / TILE_SIZE + offsetX + 2, posCentral.y() / TILE_SIZE));
					}
					else
						delete aux; // no tengo lugar para el tercer misile turret
				}
				else
					delete aux; // no tengo lugar para el segundo misile turret
			}
			else
				delete aux; // no tengo lugar para el primer misile turret
			
			offsetX++;
		}

		//-- NO ENCONTRO POSICION POSIBLE
		delete tipo;
		return NULL;

	}
	else if (orientacion == DERECHA){

		TilePosition *aux;
		UnitType *tipo = new UnitType(Utilidades::ID_MISSILE_TURRET);
		int offsetX = 0;
		
		while (offsetX < 10){
			//Broodwar->printf("Busca");
			aux = new TilePosition(posCentral.x() / TILE_SIZE - offsetX, posCentral.y() / TILE_SIZE);

			if (puedoConstruir(*aux, *tipo)){
				delete aux;
				aux = new TilePosition(posCentral.x() / TILE_SIZE - offsetX, posCentral.y() / TILE_SIZE - 2);

				if (puedoConstruir(*aux, *tipo)){
					delete aux;					
					aux = new TilePosition(posCentral.x() / TILE_SIZE - offsetX, posCentral.y() / TILE_SIZE + 2);

					if (puedoConstruir(*aux, *tipo)){
						delete aux;
						delete tipo;

						//-- ENCONTRO UNA POSICION POSIBLE
						return (new TilePosition(posCentral.x() / TILE_SIZE - offsetX -2, posCentral.y() / TILE_SIZE));
					}
					else
						delete aux; // no tengo lugar para el tercer misile turret
				}
				else
					delete aux; // no tengo lugar para el segundo misile turret
			}
			else
				delete aux; // no tengo lugar para el primer misile turret
			
			offsetX++;
		}

		//-- NO ENCONTRO POSICION POSIBLE
		delete tipo;
		return NULL;

	}
	else if (orientacion == ARRIBA){

		TilePosition *aux;
		UnitType *tipo = new UnitType(Utilidades::ID_MISSILE_TURRET);
		int offsetY = 0;
		
		while (offsetY < 10){
			//Broodwar->printf("Busca");
			aux = new TilePosition(posCentral.x() / TILE_SIZE, posCentral.y() / TILE_SIZE + offsetY);

			if (puedoConstruir(*aux, *tipo)){
				delete aux;
				aux = new TilePosition(posCentral.x() / TILE_SIZE + 2, posCentral.y() / TILE_SIZE + offsetY);

				if (puedoConstruir(*aux, *tipo)){
					delete aux;					
					aux = new TilePosition(posCentral.x() / TILE_SIZE - 2, posCentral.y() / TILE_SIZE + offsetY);

					if (puedoConstruir(*aux, *tipo)){
						delete aux;
						delete tipo;

						//-- ENCONTRO UNA POSICION POSIBLE
						return (new TilePosition(posCentral.x() / TILE_SIZE, posCentral.y() / TILE_SIZE + offsetY +2));
					}
					else
						delete aux; // no tengo lugar para el tercer misile turret
				}
				else
					delete aux; // no tengo lugar para el segundo misile turret
			}
			else
				delete aux; // no tengo lugar para el primer misile turret
			
			offsetY++;
		}

		//-- NO ENCONTRO POSICION POSIBLE
		delete tipo;
		return NULL;

	}
	else if (orientacion == ABAJO){

		TilePosition *aux;
		UnitType *tipo = new UnitType(Utilidades::ID_MISSILE_TURRET);
		int offsetY = 0;
		
		while (offsetY < 10){
			//Broodwar->printf("Busca");
			aux = new TilePosition(posCentral.x() / TILE_SIZE, posCentral.y() / TILE_SIZE - offsetY);

			if (puedoConstruir(*aux, *tipo)){
				delete aux;
				aux = new TilePosition(posCentral.x() / TILE_SIZE + 2, posCentral.y() / TILE_SIZE - offsetY);

				if (puedoConstruir(*aux, *tipo)){
					delete aux;					
					aux = new TilePosition(posCentral.x() / TILE_SIZE - 2, posCentral.y() / TILE_SIZE - offsetY);

					if (puedoConstruir(*aux, *tipo)){
						delete aux;
						delete tipo;

						//-- ENCONTRO UNA POSICION POSIBLE
						return (new TilePosition(posCentral.x() / TILE_SIZE, posCentral.y() / TILE_SIZE - offsetY -2));
					}
					else
						delete aux; // no tengo lugar para el tercer misile turret
				}
				else
					delete aux; // no tengo lugar para el segundo misile turret
			}
			else
				delete aux; // no tengo lugar para el primer misile turret
			
			offsetY++;
		}

		//-- NO ENCONTRO POSICION POSIBLE
		delete tipo;
		return NULL;

	}
	else{
		Broodwar->printf("Llamada erronea a metodo encontrarPosicion de la clase GrupoAntiaereo (parametro fuera de rango correcto)");
		return NULL;
	}


}


TilePosition* GrupoAntiaereo::getPosicionMisileTurret(){

	// recorre los elementos de la lista para ver si alguna de las unidades ya no existe mas, si ese es el caso, reutiliza el elemento de la lista actualizando el puntero de la unidad
	if (!listMisileTurrets.empty()){
		std::list<std::pair<TilePosition*, Unit*>>::iterator It;

		It = listMisileTurrets.begin();

		while (It != listMisileTurrets.end()){
			if (!(*It).second->exists()){
				return (new TilePosition((*It).first->x(), (*It).first->y()));
			}
			
			It++;
		}
	}

	// busca posiciones en sentido horario
	int cantElemSaltear = listMisileTurrets.size();

	if (tIzq != NULL){
		if (cantElemSaltear >= 3)
			cantElemSaltear -= 3;
		else{
			if (cantElemSaltear == 0)
				return (new TilePosition(tIzq->x() - 1, tIzq->y()));
			else if (cantElemSaltear == 1)
				return (new TilePosition(tIzq->x(), tIzq->y() - 2));
			else if (cantElemSaltear == 2)
				return (new TilePosition(tIzq->x(), tIzq->y() + 2));
		}

	}

	if (tArr != NULL){
		if (cantElemSaltear >= 3)
			cantElemSaltear -= 3;
		else{
			if (cantElemSaltear == 0)
				return (new TilePosition(tArr->x(), tArr->y() - 1));
			else if (cantElemSaltear == 1)
				return (new TilePosition(tArr->x() - 2, tArr->y()));
			else if (cantElemSaltear == 2)
				return (new TilePosition(tArr->x() + 2, tArr->y()));
		}

	}

	if (tDer != NULL){
		if (cantElemSaltear >= 3)
			cantElemSaltear -= 3;
		else{
			if (cantElemSaltear == 0)
				return (new TilePosition(tDer->x() + 1, tDer->y()));
			else if (cantElemSaltear == 1)
				return (new TilePosition(tDer->x(), tDer->y() - 2));
			else if (cantElemSaltear == 2)
				return (new TilePosition(tDer->x(), tDer->y() + 2));
		}
	}

	if (tAba != NULL){
		if (cantElemSaltear >= 3)
			cantElemSaltear -= 3;
		else{
			if (cantElemSaltear == 0)
				return (new TilePosition(tAba->x(), tAba->y() + 1));
			else if (cantElemSaltear == 1)
				return (new TilePosition(tAba->x() - 2, tAba->y()));
			else if (cantElemSaltear == 2)
				return (new TilePosition(tAba->x() + 2, tAba->y()));
		}
	}


	Broodwar->printf("Error en getPosicionMisileTurret, clase GrupoAntiaereo, no se encontro posicion");
	return NULL;

}


int GrupoAntiaereo::getCantMisileTurrets(){
	int cont = 0;
	std::list<std::pair<TilePosition*, Unit*>>::iterator It;

	It = listMisileTurrets.begin();

	while (It != listMisileTurrets.end()){
		if ((*It).second->exists())
			cont++;
		
		It++;
	}

	return cont;
}



bool GrupoAntiaereo::faltanMisileTurrets(){
	if (listMisileTurrets.size() < cantidadNecesariaTurrets)
		return true;
	else
		return (getCantMisileTurrets() < cantidadNecesariaTurrets);
}


void GrupoAntiaereo::agregarUnidad(Unit *u){
	bool agregue = false;
	std::list<std::pair<TilePosition*, Unit*>>::iterator It;

	It = listMisileTurrets.begin();

	while (It != listMisileTurrets.end()){
		if (((*It).first->x() == u->getTilePosition().x()) && ((*It).first->y() == u->getTilePosition().y()) && (!(*It).second->exists())){
			TilePosition *aux = (*It).first;

			(*It) = std::make_pair(aux, u);
			agregue = true;
			break;
		}

		It++;
	}

	if (!agregue){
		std::pair<TilePosition*, Unit*> par = std::make_pair(new TilePosition(u->getTilePosition().x(), u->getTilePosition().y()), u);
		listMisileTurrets.push_back(par);
	}
}

int GrupoAntiaereo::cantMaximaTurrets(){
	return cantidadNecesariaTurrets;
}


void onUnitShow(Unit *u){
	/*if (objetivoActual == NULL)
		objetivoActual = u;
	else{
		if (u->getType().isFlyer())
			if ((u->getType().size() == UnitSizeTypes::Large) && (u->getHitPoints() < objetivoActual->getHitPoints()))


	}*/

}