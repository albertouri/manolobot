#include "AnalizadorTerreno.h"
#include "Graficos.h"

#include <BWAPI.h>
#include <BWTA.h>

using namespace BWAPI;

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

			/*if (Broodwar->isWalkable((*c)->getCenter().x() /8, (*c)->getCenter().y() /8)){
				Broodwar->printf("es walkable y voy para alla");
			}*/

			/*TilePosition *p1, *p2;

			p1 = new TilePosition((*c)->getCenter().x()/32 - 8, (*c)->getCenter().y()/32  - 8);
			p2 = new TilePosition((*c)->getCenter().x()/32 + 8, (*c)->getCenter().y()/32  + 8);

			if (BWTA::isConnected(*p1, *p2)){*/

			/*int x, y;
			x = (*c)->getCenter().x()/ 32;
			y = (*c)->getCenter().y() /32;

			if (BWTA::isConnected(x-32, y-32, x+32, y+32)){
				min_length=length;
				choke=*c;

				uno = new Position(choke->getCenter().x() - 32, choke->getCenter().y() - 32);
				dos = new Position(choke->getCenter().x() + 32, choke->getCenter().y() + 32);
				estocolmo = true;
			}*/
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

