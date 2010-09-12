#include "Scout.h"

using namespace BWAPI;

Unit *explorer; // puntero a la unidad que es el scout

TilePosition *posiciones[30]; // arreglo de las posiciones a explorar

int cantPosiciones = 0; // cantidad de posiciones a explorar
int modo = 0; // modo de visualizacion de la grilla
int cont = 0;

int ID; // almacena el ID de la unidad utilizada como scout


// El constructor inicializa el arreglo con las posiciones a explorar
Scout::Scout(BWAPI::Unit *unidad)
{
	explorer = unidad;
	ID = explorer->getID();

	// Crea un arreglo de posiciones a explorar
	int maxX, maxY;
	int termX, termY;
	int posX, posY;
	int nPosX, nPosY;

	//int nPosX, nPosY;

	maxX = Broodwar->mapWidth();
	maxY = Broodwar->mapHeight();

	termX = maxX / 6;
	termY = maxY / 6;

	// posicion de referencia sobre la cual se calculan las posiciones a explorar
	posX = Broodwar->self()->getStartLocation().x();
	posY = Broodwar->self()->getStartLocation().y();

	// a la derecha
	if (posX + termX < maxX){
		posiciones[cantPosiciones] = new TilePosition(posX + termX, posY);
		cantPosiciones++;

		// arriba a la derecha
		if (posY - termY >= 0){
			posiciones[cantPosiciones] = new TilePosition(posX + termX, posY - termY);
			cantPosiciones++;
		}
	}

	// arriba
	if (posY - termY >= 0){
		posiciones[cantPosiciones] = new TilePosition(posX, posY - termY);
		cantPosiciones++;

		// arriba a la izq
		if (posX - termX >= 0){
			posiciones[cantPosiciones] = new TilePosition(posX - termX, posY - termY);
			cantPosiciones++;
		}
	}

	// izquierda
	if (posX - termX >= 0){
		posiciones[cantPosiciones] = new TilePosition(posX - termX, posY);
		cantPosiciones++;

		// abajo a la izq
		if (posY - termY < maxY){
			posiciones[cantPosiciones] = new TilePosition(posX - termX, posY + termY);
			cantPosiciones++;
		}
	}

	// abajo
	if (posY + termY < maxY){
		posiciones[cantPosiciones] = new TilePosition(posX, posY + termY);
		cantPosiciones++;

		// abajo a la derecha
		if (posX + termX < maxX){
			posiciones[cantPosiciones] = new TilePosition(posX + termX, posY + termY);
			cantPosiciones++;
		}
	}

	posiciones[cantPosiciones] = new TilePosition(*posiciones[0]);
	cantPosiciones++;
	// Hasta aca deberia haber 4 posiciones a explorar

	
	
	for(std::set<TilePosition>::iterator p=Broodwar->getStartLocations().begin(); p != Broodwar->getStartLocations().end(); p++){
		nPosX = (*p).x();
		nPosY = (*p).y();

		if ((nPosY != posY) || (nPosX != posX)){
			
			// a la derecha
			if (nPosX + termX < maxX){
				posiciones[cantPosiciones] = new TilePosition(nPosX + termX, nPosY);
				cantPosiciones++;

				// arriba a la derecha
				if (nPosY - termY >= 0){
					posiciones[cantPosiciones] = new TilePosition(nPosX + termX, nPosY - termY);
					cantPosiciones++;
				}
			}

			// arriba
			if (nPosY - termY >= 0){
				posiciones[cantPosiciones] = new TilePosition(nPosX, nPosY - termY);
				cantPosiciones++;

				// arriba a la izq
				if (posX - termX >= 0){
					posiciones[cantPosiciones] = new TilePosition(nPosX - termX, nPosY - termY);
					cantPosiciones++;
				}
			}

			// izquierda
			if (nPosX - termX >= 0){
				posiciones[cantPosiciones] = new TilePosition(nPosX - termX, nPosY);
				cantPosiciones++;

				// abajo a la izq
				if (nPosY - termY < maxY){
					posiciones[cantPosiciones] = new TilePosition(nPosX - termX, nPosY + termY);
					cantPosiciones++;
				}
			}

			// abajo
			if (nPosY + termY < maxY){
				posiciones[cantPosiciones] = new TilePosition(nPosX, nPosY + termY);
				cantPosiciones++;

				// abajo a la derecha
				if (nPosX + termX < maxX){
					posiciones[cantPosiciones] = new TilePosition(nPosX + termX, nPosY + termY);
					cantPosiciones++;
				}
			}
		}
	}



	//Broodwar->printf("Hay %d posiciones a explorar", cantPosiciones);

	explorer->rightClick(*posiciones[cont]);
	cont++;

	//-- NUEVO
	//regActual = NULL;
	grafo = NULL;
	tiempoMax = 0;
}



Scout::~Scout(void)
{
}


// Nuevo constructor
Scout::Scout(Unit *unidad, Grafo *g){
	explorer = unidad;
	ID = explorer->getID();

	//-- calcula las posiciones a explorar para la primera exploracion

	// Crea un arreglo de posiciones a explorar
	int maxX, maxY;
	int termX, termY;
	int posX, posY;
	int nPosX, nPosY;

	//int nPosX, nPosY;

	maxX = Broodwar->mapWidth();
	maxY = Broodwar->mapHeight();

	termX = maxX / 6;
	termY = maxY / 6;

	// posicion de referencia sobre la cual se calculan las posiciones a explorar
	posX = Broodwar->self()->getStartLocation().x();
	posY = Broodwar->self()->getStartLocation().y();

	// a la derecha
	if (posX + termX < maxX){
		posiciones[cantPosiciones] = new TilePosition(posX + termX, posY);
		cantPosiciones++;

		// arriba a la derecha
		if (posY - termY >= 0){
			posiciones[cantPosiciones] = new TilePosition(posX + termX, posY - termY);
			cantPosiciones++;
		}
	}

	// arriba
	if (posY - termY >= 0){
		posiciones[cantPosiciones] = new TilePosition(posX, posY - termY);
		cantPosiciones++;

		// arriba a la izq
		if (posX - termX >= 0){
			posiciones[cantPosiciones] = new TilePosition(posX - termX, posY - termY);
			cantPosiciones++;
		}
	}

	// izquierda
	if (posX - termX >= 0){
		posiciones[cantPosiciones] = new TilePosition(posX - termX, posY);
		cantPosiciones++;

		// abajo a la izq
		if (posY - termY < maxY){
			posiciones[cantPosiciones] = new TilePosition(posX - termX, posY + termY);
			cantPosiciones++;
		}
	}

	// abajo
	if (posY + termY < maxY){
		posiciones[cantPosiciones] = new TilePosition(posX, posY + termY);
		cantPosiciones++;

		// abajo a la derecha
		if (posX + termX < maxX){
			posiciones[cantPosiciones] = new TilePosition(posX + termX, posY + termY);
			cantPosiciones++;
		}
	}

	posiciones[cantPosiciones] = new TilePosition(*posiciones[0]);
	cantPosiciones++;
	// Hasta aca deberia haber 4 posiciones a explorar

	
	
	for(std::set<TilePosition>::iterator p=Broodwar->getStartLocations().begin(); p != Broodwar->getStartLocations().end(); p++){
		nPosX = (*p).x();
		nPosY = (*p).y();

		if ((nPosY != posY) || (nPosX != posX)){
			
			// a la derecha
			if (nPosX + termX < maxX){
				posiciones[cantPosiciones] = new TilePosition(nPosX + termX, nPosY);
				cantPosiciones++;

				// arriba a la derecha
				if (nPosY - termY >= 0){
					posiciones[cantPosiciones] = new TilePosition(nPosX + termX, nPosY - termY);
					cantPosiciones++;
				}
			}

			// arriba
			if (nPosY - termY >= 0){
				posiciones[cantPosiciones] = new TilePosition(nPosX, nPosY - termY);
				cantPosiciones++;

				// arriba a la izq
				if (posX - termX >= 0){
					posiciones[cantPosiciones] = new TilePosition(nPosX - termX, nPosY - termY);
					cantPosiciones++;
				}
			}

			// izquierda
			if (nPosX - termX >= 0){
				posiciones[cantPosiciones] = new TilePosition(nPosX - termX, nPosY);
				cantPosiciones++;

				// abajo a la izq
				if (nPosY - termY < maxY){
					posiciones[cantPosiciones] = new TilePosition(nPosX - termX, nPosY + termY);
					cantPosiciones++;
				}
			}

			// abajo
			if (nPosY + termY < maxY){
				posiciones[cantPosiciones] = new TilePosition(nPosX, nPosY + termY);
				cantPosiciones++;

				// abajo a la derecha
				if (nPosX + termX < maxX){
					posiciones[cantPosiciones] = new TilePosition(nPosX + termX, nPosY + termY);
					cantPosiciones++;
				}
			}
		}
	}



	//Broodwar->printf("Hay %d posiciones a explorar", cantPosiciones);

	explorer->rightClick(*posiciones[cont]);
	cont++;

	//-- 


	//regActual = g->primerNodoNiveles();
	posActual = g->primerNodoNiveles();
	grafo = g;
	tiempoMax = 0;
	primeraExploracion = true;
}

/*void Scout::asignarNuevoScout(Unit* nuevoScout){
	explorer = nuevoScout;
}*/


bool Scout::exists(){
	return explorer->exists();
}

void Scout::explorar(void){

	/*for(std::set<Unit*>::iterator i=Broodwar->getSelectedUnits().begin();i!=Broodwar->getSelectedUnits().end();i++){
		dibujarGrilla();
	}*/

	//grafo->dibujarPuntosVisitar();

	if (primeraExploracion){
		if ((explorer != NULL) && (explorer->exists())){
			if (cont < cantPosiciones){
				if (explorer->isIdle()){
					//Broodwar->printf("Me muevo...");
					explorer->rightClick(*posiciones[cont]);
					cont++;
				}
			}
		}
	}
	else{
		//dibujarPosiciones();
		tiempoMax++;

		if (Broodwar->getFrameCount() % 23 == 0){
			if ((/*regActual*/posActual != NULL) && (grafo != NULL)){
				if ((explorer != NULL) && (explorer->exists()) && /*(explorer->getPosition().getDistance(regActual->getCenter())*/(explorer->getPosition().getApproxDistance(*posActual) < 30)){
					//regActual = grafo->siguienteNodoNiveles();
					posActual = grafo->siguienteNodoNiveles();
					tiempoMax = 0;
				}
				else{
					if (tiempoMax > 700){
						tiempoMax = 0;
						//regActual = grafo->siguienteNodoNiveles();
						posActual = grafo->siguienteNodoNiveles();
					}
					else{
						//explorer->move(regActual->getCenter());
						explorer->move(*posActual);
						//tiempoMax++;
					}
				}
			}
		}
	}
}


void Scout::cambiarModo(int m){
	if ((m >= 0) && (m <= 1)){
		modo = m;
	}
}


void Scout::dibujarPosiciones(void){

	Broodwar->drawBox(CoordinateType::Map, Broodwar->self()->getStartLocation().x() * 32, Broodwar->self()->getStartLocation().y() * 32, Broodwar->self()->getStartLocation().x() * 32 + 32, Broodwar->self()->getStartLocation().y() * 32 + 32, Colors::Yellow, true);

	for (int z = 0; z < cantPosiciones; z++){
		Broodwar->drawBox(CoordinateType::Map, (*posiciones[z]).x() * 32, (*posiciones[z]).y() * 32, (*posiciones[z]).x() * 32 + 32, (*posiciones[z]).y() * 32 + 32, Colors::Red, true);
	}

}


void Scout::dibujarGrilla(void){

	int y, ejex, ejey, maxX, maxY;
	int dim = 8;
	int fac = 32;
	int f2 = 8;
	

	ejex = explorer->getTilePosition().x() + 1;
	//ejey = pos->y() + 1;
	ejey = explorer->getTilePosition().y() + 1;
	maxX = Broodwar->mapWidth() * 4;
	maxY = Broodwar->mapHeight() * 4;


	if (modo == 0){
		for (int k = (explorer->getType().sightRange() / 8 + 4); k > 1; k--){
			y = k;

			for (int x = 0; x <= k; x++){
			
				if (((ejex * 4 + x) < maxX) && ((ejey * 4 + y) < maxY)){
					if (!Broodwar->isWalkable(ejex * 4 + x, ejey  * 4 + y)) {
						Broodwar->drawBox(CoordinateType::Map, ejex * fac + x * f2, ejey * fac + y * f2, ejex * fac + x * f2 + dim, ejey * fac + y * f2 + dim, Colors::Black, false);
					}
					else Broodwar->drawBox(CoordinateType::Map, ejex * fac + x * f2, ejey * fac + y * f2, ejex * fac + x * f2 + dim, ejey * fac + y * f2 + dim, Colors::Grey, false);
				}

				if (((ejex * 4 + x) < maxX) && ((ejey * 4 - y) >= 0)){
					if (!Broodwar->isWalkable(ejex * 4 + x, ejey * 4 - y)) {
						Broodwar->drawBox(CoordinateType::Map, ejex * fac + x * f2, ejey * fac - y * f2, ejex * fac + x * f2 + dim, ejey * fac - y * f2 + dim, Colors::Black, false);
					}
					else Broodwar->drawBox(CoordinateType::Map, ejex * fac + x * f2, ejey * fac - y * f2, ejex * fac + x * f2 + dim, ejey * fac - y * f2 + dim, Colors::Grey, false);
				}

				if (((ejex * 4 - x) >= 0) && ((ejey * 4 + y) < maxY)){
					if (!Broodwar->isWalkable(ejex * 4 - x, ejey * 4 + y)) {
						Broodwar->drawBox(CoordinateType::Map, ejex * fac - x * f2, ejey * fac + y * f2, ejex * fac - x * f2 + dim, ejey * fac + y * f2 + dim, Colors::Black, false);
					}
					else{
						Broodwar->drawBox(CoordinateType::Map, ejex * fac - x * f2, ejey * fac + y * f2, ejex * fac - x * f2 + dim, ejey * fac + y * f2 + dim, Colors::Grey, false);
					}
				}

				if (((ejex * 4 - x) >= 0) && ((ejey * 4 - y) >= 0)){
					if (!Broodwar->isWalkable(ejex * 4 - x, ejey * 4 - y)) {
						Broodwar->drawBox(CoordinateType::Map, ejex * fac - x * f2, ejey * fac - y * f2, ejex * fac - x * f2 + dim, ejey * fac - y * f2 + dim, Colors::Black, false);
					}
					else Broodwar->drawBox(CoordinateType::Map, ejex * fac - x * f2, ejey * fac - y * f2, ejex * fac - x * f2 + dim, ejey * fac - y * f2 + dim, Colors::Grey, false);
				}

				y--;
			}
		}
	}
	else if (modo == 1){

		//for (int k = 36; k > 1; k--){
		for (int k = (explorer->getType().sightRange() / 8 + 4); k > 1; k--){
			y = k;

			for (int x = 0; x <= k; x++){
			
				if (((ejex * 4 + x) < maxX) && ((ejey * 4 + y) < maxY)){
					if (Broodwar->getGroundHeight(ejex * 4 + x, ejey * 4 + y) == 2) {			
						Broodwar->drawBox(CoordinateType::Map, ejex * fac + x * f2, ejey * fac + y * f2, ejex * fac + x * f2 + dim, ejey * fac + y * f2 + dim, Colors::Blue, false);
					}
					else Broodwar->drawBox(CoordinateType::Map, ejex * fac + x * f2, ejey * fac + y * f2, ejex * fac + x * f2 + dim, ejey * fac + y * f2 + dim, Colors::Grey, false);
				}

				if (((ejex * 4 + x) < maxX) && ((ejey * 4 - y) >= 0)){
					if (Broodwar->getGroundHeight(ejex * 4 + x, ejey * 4 - y) == 2) {
						Broodwar->drawBox(CoordinateType::Map, ejex * fac + x * f2, ejey * fac - y * f2, ejex * fac + x * f2 + dim, ejey * fac - y * f2 + dim, Colors::Blue, false);
					}
					else Broodwar->drawBox(CoordinateType::Map, ejex * fac + x * f2, ejey * fac - y * f2, ejex * fac + x * f2 + dim, ejey * fac - y * f2 + dim, Colors::Grey, false);
				}

				if (((ejex * 4 - x) >= 0) && ((ejey * 4 + y) < maxY)){
					if (Broodwar->getGroundHeight(ejex * 4 - x, ejey * 4 + y) == 2) {
						Broodwar->drawBox(CoordinateType::Map, ejex * fac - x * f2, ejey * fac + y * f2, ejex * fac - x * f2 + dim, ejey * fac + y * f2 + dim, Colors::Blue, false);
					}
					else{
						Broodwar->drawBox(CoordinateType::Map, ejex * fac - x * f2, ejey * fac + y * f2, ejex * fac - x * f2 + dim, ejey * fac + y * f2 + dim, Colors::Grey, false);
					}
				}

				if (((ejex * 4 - x) >= 0) && ((ejey * 4 - y) >= 0)){
					if (Broodwar->getGroundHeight(ejex * 4 - x, ejey * 4 - y) == 2) {
						Broodwar->drawBox(CoordinateType::Map, ejex * fac - x * f2, ejey * fac - y * f2, ejex * fac - x * f2 + dim, ejey * fac - y * f2 + dim, Colors::Blue, false);
					}
					else Broodwar->drawBox(CoordinateType::Map, ejex * fac - x * f2, ejey * fac - y * f2, ejex * fac - x * f2 + dim, ejey * fac - y * f2 + dim, Colors::Grey, false);
				}

				y--;
			}
		}

	}
}


void Scout::setExplorador(Unit *unidad){
	explorer = unidad;
	ID = explorer->getID();
	tiempoMax = 0;

	if (grafo != NULL)
		//regActual = grafo->primerNodoNiveles();
		posActual = grafo->primerNodoNiveles();
	else
		//regActual = NULL;
		posActual = NULL;

	//if (regActual != NULL){
	if (posActual != NULL){
		primeraExploracion = false;
	}
	else{
		// parte nueva, si falla borrarlo
		primeraExploracion = true;
		cont = 0;
	}
}


Unit* Scout::getExplorador(){
	return explorer;
}