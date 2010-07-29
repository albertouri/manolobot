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






// retorna el angulo que forma una recta que une los dos puntos con respecto a la vertical
//int AnalizadorTerreno::calcularAngulo(Position *p1, Position *p2){
int AnalizadorTerreno::calcularAngulo(Chokepoint *c){
	double angulo;
	double division;
	Position *p1, *p2;

	int restoAngulo; // si el punto p2 tiene menor coordenada Y que el punto p1, se debe sumar 90 al angulo resultante del calculo
	double segmentoB;

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
			p1 = new Position(c->getSides().second.x(), c->getSides().second.y());
			p2 = new Position(c->getSides().first.x(), c->getSides().first.y());
		}
	}
	else{
		// la inclinacion del chokepoint es vertical |
		if (c->getSides().first.y() < c->getSides().second.y()){
			p1 = new Position(c->getSides().first.x(), c->getSides().first.y());
			p2 = new Position(c->getSides().second.x(), c->getSides().second.y());
		}
		else{
			p1 = new Position(c->getSides().second.x(), c->getSides().second.y());
			p2 = new Position(c->getSides().first.x(), c->getSides().first.y());
		}
	}

	if (p1->y() >= p2->y()){
		restoAngulo = 90;
		segmentoB = p1->y() - p2->y();
	}
	else{
		restoAngulo = 0;
		segmentoB = p2->x() - p1->x();
	}

	division = segmentoB / p1->getDistance(*p2);
	angulo = asin(division) * 180.0 / PI;

	//Broodwar->printf("el angulo es: %lf", angulo);

	delete p1;
	delete p2;

	return ((int) (angulo + restoAngulo));
}


int AnalizadorTerreno::getOrientacion(Chokepoint *c, Region *r){
	if (r->getCenter().y() < c->getCenter().y()){
		if (r->getCenter().x() < c->getCenter().x())
			return 1;
		else
			return 2;
	}
	else{
		if (r->getCenter().x() < c->getCenter().x())
			return 3;
		else
			return 4;
	}
}

int AnalizadorTerreno::getCuadrante(Position p){
	
	if (p.x() <= (Broodwar->mapWidth() * 32 / 2)){
		if (p.y() <= (Broodwar->mapHeight() * 32 / 2))
			return 1;
		else
			return 3;
	}
	else{
		if (p.y() <= (Broodwar->mapHeight() * 32 / 2))
			return 2;
		else
			return 4;
	}
}

int AnalizadorTerreno::calcularAnguloGrupo(int angulo){
	int angulo1;

	if ((angulo > 112) && (angulo <= 179))
		angulo1 = 0;
	else if ((angulo < 67) && (angulo >= 0))
		angulo1 = 0;
	else
		angulo1 = 90;

	return angulo1;
}