#include "Graficos.h"

/*Graficos::Graficos(void)
{
}

Graficos::~Graficos(void)
{
}*/

void Graficos::dibujarCuadro(TilePosition* p, int tilesAncho, int tilesAlto){
	Broodwar->drawBox(CoordinateType::Map, p->x() * 32, p->y() * 32, p->x() * 32 + tilesAncho * 32, p->y() * 32 + tilesAlto * 32, Colors::Orange, false);
}

void Graficos::dibujarCirculo(TilePosition p, int tilesAncho, int tilesAlto){
	if (tilesAncho > tilesAlto)
		Broodwar->drawCircle(CoordinateType::Map, p.x() * 32, p.y() * 32, tilesAncho * 32 / 2, Colors::White, false);
	else
		Broodwar->drawCircle(CoordinateType::Map, p.x() * 32, p.y() * 32, tilesAlto * 32 / 2, Colors::White, false);
}


void Graficos::resaltarUnidad(Unit *u, Color c){

	if (u != NULL){
		int izq = u->getType().dimensionLeft() * 2;
		int der = u->getType().dimensionRight() * 2;
		int arr = u->getType().dimensionUp() * 2;
		int aba = u->getType().dimensionDown() * 2;

		Broodwar->drawBox(CoordinateType::Map, u->getPosition().x() - izq, u->getPosition().y() - arr, u->getPosition().x() + der, u->getPosition().y() + aba, c, false);
	}
}

void Graficos::resaltarUnidad(Unit *u){
	resaltarUnidad(u, Colors::White);
}

void Graficos::dibujarTerreno(bool show_visibility_data, bool analyzed){

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

	//drawStats();

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

}


void Graficos::drawStats()
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
