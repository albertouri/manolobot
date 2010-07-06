#include "AnalizadorTerreno.h"
#include "Graficos.h"
#include <math.h>

#define PI 3.14159265

Position *uno, *dos;


AnalizadorTerreno::AnalizadorTerreno(void)
{
	//read map information into BWTA so terrain analysis can be done in another thread
	BWTA::readMap();
	analyzed=false;
	analysis_just_finished=false;
	show_visibility_data=false;

	// Analiza el terreno del mapa
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
}


AnalizadorTerreno::~AnalizadorTerreno(void)
{


}


void AnalizadorTerreno::dibujarResultados(void){

	Graficos::dibujarTerreno(show_visibility_data, analyzed);

	if (analysis_just_finished)
	{
		Broodwar->printf("Finished analyzing map.");
		analysis_just_finished=false;
	}
}


// Devuelve la posicion correspondiente al centro del chokepoint que es necesario defender. Por ahora retorna el 
// chokepoint a defender a partir de la region donde se inicia el juego
// Deberia pasarse como parametro una unidad para saber la region donde se quieren ubicar las defensas
Position * AnalizadorTerreno::obtenerCentroChokepoint(){
	
	//get the chokepoints linked to our home region
	std::set<BWTA::Chokepoint*> chokepoints= home->getChokepoints();
	double min_length=10000;
	//double min_length = 0;
	BWTA::Chokepoint* choke=NULL;

	//iterate through all chokepoints and look for the one with the smallest gap (least width)
	for(std::set<BWTA::Chokepoint*>::iterator c=chokepoints.begin();c!=chokepoints.end();c++){
		double length=(*c)->getWidth();

		if (length < min_length || choke==NULL){

			std::pair<BWTA::Region*, BWTA::Region*> p;

			p = (*c)->getRegions();

			if ((p.first->getCenter().x() == home->getCenter().x()) && (p.first->getCenter().y() == home->getCenter().y())){
				// el primer elemento del par es la region que contiene al centro de comando

				if (p.second->getChokepoints().size() == 1){
					// la region cuyo limite con la region donde esta ubicado el centro de comando, es el chokepoint en 
					// cuestion tiene un solo chokepoint, es decir que no se puede acceder por tierra a esa region sin 
					// pasar por la region que contiene el centro de comando, por lo tanto no es necesario defenderla 
					// inicialmente					
				}
				else{
					choke=*c;
				}
			}
			else{
				// el segundo elemento del par es la region que contiene al centro de comando

				if (p.first->getChokepoints().size() == 1){
					// la region cuyo limite con la region donde esta ubicado el centro de comando, es el chokepoint en 
					// cuestion tiene un solo chokepoint, es decir que no se puede acceder por tierra a esa region sin 
					// pasar por la region que contiene el centro de comando, por lo tanto no es necesario defenderla 
					// inicialmente					
				}
				else{
					choke=*c;
				}
			}
		}
	}

	if (choke == NULL)
		return NULL;
	else
		return( new Position(choke->getCenter().x(), choke->getCenter().y()));
}


// Devuelve la posicion correspondiente al centro del chokepoint que es necesario defender. Por ahora retorna el 
// chokepoint a defender a partir de la region donde se inicia el juego
// Deberia pasarse como parametro una unidad para saber la region donde se quieren ubicar las defensas
Chokepoint* AnalizadorTerreno::obtenerChokepoint(){
	
	//get the chokepoints linked to our home region
	std::set<BWTA::Chokepoint*> chokepoints= home->getChokepoints();

	double min_length=10000;
	//double min_length = 0;
	BWTA::Chokepoint* choke=NULL;

	//iterate through all chokepoints and look for the one with the smallest gap (least width)
	for(std::set<BWTA::Chokepoint*>::iterator c=chokepoints.begin();c!=chokepoints.end();c++){
		double length=(*c)->getWidth();

		if (length < min_length || choke==NULL){

			std::pair<BWTA::Region*, BWTA::Region*> p;

			p = (*c)->getRegions();

			if ((p.first->getCenter().x() == home->getCenter().x()) && (p.first->getCenter().y() == home->getCenter().y())){
				// el primer elemento del par es la region que contiene al centro de comando

				if (p.second->getChokepoints().size() == 1){
					// la region cuyo limite con la region donde esta ubicado el centro de comando, es el chokepoint en 
					// cuestion tiene un solo chokepoint, es decir que no se puede acceder por tierra a esa region sin 
					// pasar por la region que contiene el centro de comando, por lo tanto no es necesario defenderla 
					// inicialmente					
				}
				else{
					choke=*c;
				}
			}
			else{
				// el segundo elemento del par es la region que contiene al centro de comando

				if (p.first->getChokepoints().size() == 1){
					// la region cuyo limite con la region donde esta ubicado el centro de comando, es el chokepoint en 
					// cuestion tiene un solo chokepoint, es decir que no se puede acceder por tierra a esa region sin 
					// pasar por la region que contiene el centro de comando, por lo tanto no es necesario defenderla 
					// inicialmente					
				}
				else{
					choke=*c;
				}
			}
		}
	}

	if (choke == NULL)
		return NULL;
	else
		return choke;
}



DWORD WINAPI AnalyzeThread()
{
	BWTA::analyze();
	analyzed = true;
	analysis_just_finished = true;

	//self start location only available if the map has base locations
	if (BWTA::getStartLocation(BWAPI::Broodwar->self())!=NULL){
		home = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
	}

	//enemy start location only available if Complete Map Information is enabled.
	if (BWTA::getStartLocation(BWAPI::Broodwar->enemy())!=NULL){
		enemy_base = BWTA::getStartLocation(BWAPI::Broodwar->enemy())->getRegion();
	}

	return 0;
}


// Retorna true si el analisis del terreno finalizo (es necesario que el analisis haya finalizado para obtener chokepoints
// y demas). Retorna false en caso contrario
bool AnalizadorTerreno::analisisListo(void){
	return analyzed;
}


Region* AnalizadorTerreno::regionInicial(){
	return home;
}



TilePosition* AnalizadorTerreno::calcularPrimerTile(Region* r, Chokepoint* c, int nroBunker){
	Position *p1 = NULL, *p2 = NULL;
	
	int cuadrante = 0;
	int angulo = 0;

	// distancias desde el centro del chokepoint hasta el lugar para ubicar el bunker
	int distanciaX = /*96*/ /*160*/ 288;
	int distanciaY = /*96*/ 160;
	//Position *res = NULL;
	TilePosition *res = NULL;

	bool encontre = false;
	int cont = 0;


	if (Broodwar->self()->getStartLocation().x() <= (Broodwar->mapWidth() / 2)){
		if (Broodwar->self()->getStartLocation().y() <= (Broodwar->mapHeight() / 2))
			cuadrante = 1;
		else
			cuadrante = 3;
	}
	else{
		if (Broodwar->self()->getStartLocation().y() <= (Broodwar->mapHeight() / 2))
			cuadrante = 2;
		else
			cuadrante = 4;
	}


	// Inicializo los puntos que representan a los bordes del chokepoint
	// p1 siempre sera el borde mas a la izquierda del chokepoint
	// en caso de que tengan la misma coordenada X (el chokepoint es vertical), p1 sera el punto que tenga la menor coordenada Y
	if (c->getSides().first.x() != c->getSides().second.x()){
		// la inclinacion del chokepoint no es completamente vertical |, es decir el chokepoint esta inclinado	
		if (c->getSides().first.x() < c->getSides().second.x()){
			p1 = new Position(c->getSides().first.x(), c->getSides().first.y());
			p2 = new Position(c->getSides().second.x(), c->getSides().second.y());
		}
		else{
			p2 = new Position(c->getSides().first.x(), c->getSides().first.y());
			p1 = new Position(c->getSides().second.x(), c->getSides().second.y());
		}
	}
	else{
		// la inclinacion del chokepoint es vertical |
		if (c->getSides().first.y() < c->getSides().second.y()){
			p1 = new Position(c->getSides().first.x(), c->getSides().first.y());
			p2 = new Position(c->getSides().second.x(), c->getSides().second.y());
		}
		else{
			p2 = new Position(c->getSides().first.x(), c->getSides().first.y());
			p1 = new Position(c->getSides().second.x(), c->getSides().second.y());
		}
	}

	angulo = calcularAngulo(p1, p2);
	//Broodwar->printf("cuadrante: %d - angulo: %d", cuadrante, angulo);
	
	/*if (cuadrante == 1){
		if ((angulo > 0) && (angulo < 23))
			// como el angulo del chokepoint es pequeño no inclino la ubicacion del bunker, solo la desplazo en el eje X
			res = new Position(c->getCenter().x() - distanciaX, c->getCenter().y());
		else if ((angulo >= 23) && (angulo < 67))
			res = new Position(c->getCenter().x() - distanciaX, c->getCenter().y() + distanciaY);
		else if ((angulo >= 67) && (angulo < 112))
			res = new Position(c->getCenter().x() - 32, c->getCenter().y() - distanciaY);
		else if ((angulo >= 112) && (angulo < 157))
			res = new Position(c->getCenter().x() - distanciaX, c->getCenter().y() - distanciaY);
		else
			res = new Position(c->getCenter().x() - distanciaX, c->getCenter().y());
	}
	else if (cuadrante == 2){
		
		if ((angulo > 0) && (angulo < 23))
			res = new Position(c->getCenter().x() + distanciaX, c->getCenter().y());
		else if ((angulo >= 23) && (angulo < 67))
			res = new Position(c->getCenter().x() + distanciaX, c->getCenter().y() - distanciaY);
		else if ((angulo >= 67) && (angulo < 112))
			res = new Position(c->getCenter().x(), c->getCenter().y() - distanciaY);
		else if ((angulo >= 112) && (angulo < 157))
			res = new Position(c->getCenter().x() + distanciaX, c->getCenter().y() + distanciaY);
		else
			res = new Position(c->getCenter().x() + distanciaX, c->getCenter().y());
	}
	else if (cuadrante == 3){

		if ((angulo > 0) && (angulo < 23))
			res = new Position(c->getCenter().x() - distanciaX, c->getCenter().y());
		else if ((angulo >= 23) && (angulo < 67))
			res = new Position(c->getCenter().x() - distanciaX, c->getCenter().y() + distanciaY);
		else if ((angulo >= 67) && (angulo < 112))
			res = new Position(c->getCenter().x() - 32, c->getCenter().y() + distanciaY);
		else if ((angulo >= 112) && (angulo < 157))
			res = new Position(c->getCenter().x() - distanciaX, c->getCenter().y() - distanciaY);
		else
			res = new Position(c->getCenter().x() - distanciaX, c->getCenter().y());
	}
	else{
		if ((angulo > 0) && (angulo < 23))
			res = new Position(c->getCenter().x() + distanciaX, c->getCenter().y());
		else if ((angulo >= 23) && (angulo < 67))
			res = new Position(c->getCenter().x() + distanciaX, c->getCenter().y() - distanciaY);
		else if ((angulo >= 67) && (angulo < 112))
			res = new Position(c->getCenter().x(), c->getCenter().y() + distanciaY);
		else if ((angulo >= 112) && (angulo < 157))
			res = new Position(c->getCenter().x() + distanciaX, c->getCenter().y() + distanciaY);
		else
			res = new Position(c->getCenter().x() + distanciaX, c->getCenter().y());
	}*/

	res = encontrarPosicion(cuadrante, c->getCenter(), angulo, nroBunker);
		
	
	delete p1;
	delete p2;

	//return (new TilePosition(res->x() / 32, res->y() / 32));
	return res;
}

bool AnalizadorTerreno::puedoConstruir(TilePosition t, UnitType tipo){
	bool res = true;

	for (int x = 0; x < tipo.tileWidth(); x++){
		for (int y = 0; y < tipo.tileHeight(); y++)
			res = res && Broodwar->isBuildable(t.x() + x, t.y() + y);
	}

	return res;
}

TilePosition* AnalizadorTerreno::encontrarPosicion(int cuadrante, Position p, int angulo, int nroBunker){
	int factorX, factorY;

	int contX = -1, contY = 0;
	bool encontre = false;
	TilePosition *t, *res = NULL;

	int angulo1 = 0;
	
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

	if ((angulo > 135) && (angulo <= 179))
		angulo1 = 0;
	else if ((angulo < 45) && (angulo >= 0))
		angulo1 = 0;
	else
		angulo1 = 90;


	if (angulo1 == 90){
		while (contY < 10){
			t = new TilePosition(p.x() / 32 + contX, p.y() / 32 + contY * factorY);

			if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
				// puedo construir el primer bunker en esa posicion, ahora verifico si puedo construir otro bunker al lado
				if (nroBunker == 1)
					res = new TilePosition(t->x(), t->y() + 1 * factorY);

				delete t;
				t = new TilePosition(p.x() / 32 + 3 + contX, p.y() / 32 + contY * factorY);

				if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
					// puedo construir el primer bunker en esa posicion, ahora verifico si puedo construir otro bunker al lado
					if (nroBunker == 2)
						res = new TilePosition(t->x(), t->y() + 1 * factorY);

					delete t;
					t = new TilePosition(p.x() / 32 - 3 + contX, p.y() / 32 + contY * factorY);

					if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
						if (nroBunker == 3)
							res = new TilePosition(t->x(), t->y() + 1 * factorY);						
						
						delete t;

						return res;
					}
					else delete t;
				}
				else delete t;
			}
			else delete t;

			if (contX == 1){
				contY++;
				contX = -1;
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

		while (contX < 10){
			t = new TilePosition(p.x() / 32 + contX * factorX, p.y() / 32 + contY);

			if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
				// puedo construir el primer bunker en esa posicion, ahora verifico si puedo construir otro bunker al lado
				if (nroBunker == 1)
					res = new TilePosition(t->x() + 1 * factorX, t->y());

				delete t;
				t = new TilePosition(p.x() / 32 + contX * factorX, p.y() / 32 + 2 + contY);

				if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
					// puedo construir el primer bunker en esa posicion, ahora verifico si puedo construir otro bunker al lado
					if (nroBunker == 2)
						res = new TilePosition(t->x() + 1 * factorX, t->y());

					delete t;
					t = new TilePosition(p.x() / 32 + contX * factorX, p.y() / 32 - 2 + contY);

					if (puedoConstruir(*t, *(new UnitType(Utilidades::ID_BARRACK)))){
						if (nroBunker == 3)
							res = new TilePosition(t->x() + 1 * factorX, t->y());	
						
						delete t;

						return res;
					}
					else delete t;
				}
				else delete t;
			}
			else delete t;

			if (contY == 1){
				contX++;
				contY = -1;
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


// retorna el angulo que forma una recta que une los dos puntos con respecto a la vertical
int AnalizadorTerreno::calcularAngulo(Position *p1, Position *p2){
	double angulo;
	double division;

	int restoAngulo; // si el punto p2 tiene menor coordenada Y que el punto p1, se debe sumar 90 al angulo resultante del calculo
	double segmentoB;

	if (p1->y() >= p2->y()){
		restoAngulo = 90;
		segmentoB = p1->y() - p2->y();
	}
	else{
		restoAngulo = 0;
		segmentoB = p2->y() - p1->y();
	}

	division = segmentoB / p1->getDistance(*p2);
	angulo = asin(division) * 180.0 / PI;

	/*Broodwar->printf("PUNTO1: x = %d - y = %d", p1->x(), p1->y());
	Broodwar->printf("PUNTO2: x = %d - y = %d", p2->x(), p2->y());
	Broodwar->printf("distancia: %lf", p1->getDistance(*p2));
	Broodwar->printf("division: %lf", division);

	Broodwar->printf("el angulo es: %lf", angulo);*/

	return ((int) (angulo + restoAngulo));
}