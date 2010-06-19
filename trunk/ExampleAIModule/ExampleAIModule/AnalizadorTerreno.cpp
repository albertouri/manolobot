#include "AnalizadorTerreno.h"
#include "Graficos.h"

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

			//BWTA::Region *r1, *r2;
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

	//Broodwar->printf("hay %d chokepoints", chokepoints.size());

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

			//BWTA::Region *r1, *r2;
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



TilePosition* AnalizadorTerreno::calcularPrimerTile(Region* r, Chokepoint* c){
	Position *p1 = NULL, *p2 = NULL;
	
	// la variable ubicacionCentro tiene 4 valores posibles:
	// 1- el centro de la region esta ubicado arriba y a la izquierda del centro del chokepoint
	// 2- el centro de la region esta ubicado arriba y a la derecha del centro del chokepoint
	// 3- el centro de la region esta ubicado abajo y a la izquierda del centro del chokepoint
	// 4- el centro de la region esta ubicado abajo y a la derecha del centro del chokepoint
	int ubicacionCentro = 0; 

	int distanciaX = 128;
	int distanciaY = 96;
	Position *res = NULL;

	bool encontre = false;
	int cont = 0;

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

		// si la posicion es totalmente horizontal, no me deberia desplazar sobre el eje X
		if (c->getSides().first.y() == c->getSides().second.y()){
			distanciaX = 0;
			distanciaY *= -1;
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

		// no me desplazo sobre el eje Y
		distanciaY = 0;
		distanciaX *= -1;
	}

	// ubico la posicion del chokepoint respecto al centro de la region a defender
	if (r->getCenter().x() <= c->getCenter().x()){
		if (r->getCenter().y() <= c->getCenter().y())
			ubicacionCentro = 1;
		else
			ubicacionCentro = 3;
	}	
	else{
		if (r->getCenter().y() <= c->getCenter().y())
			ubicacionCentro = 2;
		else
			ubicacionCentro = 4;
	}

	while (!encontre && (cont < 4)){

		if (res != NULL) delete res;
	
		// posiblemente esto necesite una mejora...
		if (ubicacionCentro == 1)
			res = new Position(c->getCenter().x() - distanciaX, c->getCenter().y() - distanciaY);
		else if (ubicacionCentro == 2)
			res = new Position(c->getCenter().x() + distanciaX, c->getCenter().y() - distanciaY);
		else if (ubicacionCentro == 3)
			res = new Position(c->getCenter().x() - distanciaX, c->getCenter().y() + distanciaY);
		else if (ubicacionCentro == 4)
			res = new Position(c->getCenter().x() + distanciaX, c->getCenter().y() + distanciaY);

		if ((c->getRegions().first->getPolygon().isInside(*res)) || (c->getRegions().second->getPolygon().isInside(*res)))
			encontre = true;

		cont++;

		/*if (ubicacionCentro = 1)
			ubicacionCentro = (ubicacionCentro - 2) % 4;
		else*/
			ubicacionCentro = (ubicacionCentro + 1) % 4;
	}


	delete p1;
	delete p2;

	return (new TilePosition(res->x() / 32, res->y() / 32));
}