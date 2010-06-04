#include "AnalizadorTerreno.h"

#include <BWAPI.h>
#include <BWTA.h>

using namespace BWAPI;

bool estocolmo = false;
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


void AnalizadorTerreno::dibujar(void){

	if (estocolmo){
		Broodwar->drawBox(CoordinateType::Map, uno->x(), uno->y(), uno->x() + 8, uno->y() + 8, Colors::Red, true);
		Broodwar->drawBox(CoordinateType::Map, dos->x(), dos->y(), dos->x() + 8, dos->y() + 8, Colors::Red, true);
	}

	if (show_visibility_data)
	{    
		for(int x=0;x<Broodwar->mapWidth();x++)
		{
			for(int y=0;y<Broodwar->mapHeight();y++)
			{
				if (Broodwar->isExplored(x,y))
				{
					if (Broodwar->isVisible(x,y))
						Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Green);
					else
						Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Blue);
				}
				else
					Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Red);
			}
		}
	}

	if (Broodwar->isReplay())
		return;

	drawStats();

	if (analyzed)
	{
		//we will iterate through all the base locations, and draw their outlines.
		for(std::set<BWTA::BaseLocation*>::const_iterator i=BWTA::getBaseLocations().begin();i!=BWTA::getBaseLocations().end();i++)
		{
			TilePosition p=(*i)->getTilePosition();
			Position c=(*i)->getPosition();

			//draw outline of center location
			Broodwar->drawBox(CoordinateType::Map,p.x()*32,p.y()*32,p.x()*32+4*32,p.y()*32+3*32,Colors::Blue,false);

			//draw a circle at each mineral patch
			for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getStaticMinerals().begin();j!=(*i)->getStaticMinerals().end();j++)
			{
				Position q=(*j)->getInitialPosition();
				Broodwar->drawCircle(CoordinateType::Map,q.x(),q.y(),30,Colors::Cyan,false);
			}

			//draw the outlines of vespene geysers
			for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getGeysers().begin();j!=(*i)->getGeysers().end();j++)
			{
				TilePosition q=(*j)->getInitialTilePosition();
				Broodwar->drawBox(CoordinateType::Map,q.x()*32,q.y()*32,q.x()*32+4*32,q.y()*32+2*32,Colors::Orange,false);
			}

			//if this is an island expansion, draw a yellow circle around the base location
			if ((*i)->isIsland())
			{
				Broodwar->drawCircle(CoordinateType::Map,c.x(),c.y(),80,Colors::Yellow,false);
			}
		}

		//we will iterate through all the regions and draw the polygon outline of it in green.
		for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++)
		{
			BWTA::Polygon p=(*r)->getPolygon();
			for(int j=0;j<(int)p.size();j++)
			{
				Position point1=p[j];
				Position point2=p[(j+1) % p.size()];
				Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Green);
			}
		}

		//we will visualize the chokepoints with red lines
		for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++)
		{
			for(std::set<BWTA::Chokepoint*>::const_iterator c=(*r)->getChokepoints().begin();c!=(*r)->getChokepoints().end();c++)
			{
				Position point1=(*c)->getSides().first;
				Position point2=(*c)->getSides().second;
				Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Red);
			}
		}
	}

	if (analysis_just_finished)
	{
		Broodwar->printf("Finished analyzing map.");
		analysis_just_finished=false;
	}

}


// Devuelve la posicion correspondiente al centro del chokepoint que es necesario defender. Por ahora retorna el 
// chokepoint a defender a partir de la region donde se inicia el juego
// Deberia pasarse como parametro una unidad para saber la region donde se quieren ubicar las defensas
BWAPI::Position * AnalizadorTerreno::obtenerCentroChokepoint(){
	
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

	Broodwar->printf("hay %d chokepoints", chokepoints.size());

	if (choke == NULL)
		return NULL;
	else
		return( new Position(choke->getCenter().x(), choke->getCenter().y()));
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


void AnalizadorTerreno::drawStats()
{
	std::set<Unit*> myUnits = Broodwar->self()->getUnits();
	Broodwar->drawTextScreen(5,0,"I have %d units:",myUnits.size());
	std::map<UnitType, int> unitTypeCounts;
  
	for(std::set<Unit*>::iterator i=myUnits.begin();i!=myUnits.end();i++){
		if (unitTypeCounts.find((*i)->getType())==unitTypeCounts.end()){
			unitTypeCounts.insert(std::make_pair((*i)->getType(),0));
		}

		unitTypeCounts.find((*i)->getType())->second++;
	}

	int line=1;
	
	for(std::map<UnitType,int>::iterator i=unitTypeCounts.begin();i!=unitTypeCounts.end();i++){
		Broodwar->drawTextScreen(5,16*line,"- %d %ss",(*i).second, (*i).first.getName().c_str());
		line++;
	}

}


// Retorna true si el analisis del terreno finalizo (es necesario que el analisis haya finalizado para obtener chokepoints
// y demas). Retorna false en caso contrario
bool AnalizadorTerreno::analisisListo(void){
	return analyzed;
}

