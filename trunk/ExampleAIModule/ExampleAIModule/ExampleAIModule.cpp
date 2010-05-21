#include "ExampleAIModule.h"
#include "ManoloBot.h"
#include <stdlib.h>

using namespace BWAPI;


/*int cantBarracas = 0; // lleva la cuenta de la cantidad de barracas construidas
int cantSCV = 4;
Position *centroComando;	// mantiene la posicion del centro de comando
Unit *trabajador, *trabajador2; // puntero a la unidad que actualmente esta construyendo algo
*/
ManoloBot *agente;


void ExampleAIModule::onStart()
{
  Broodwar->sendText("Hello world!");
  Broodwar->printf("The map is %s, a %d player map",Broodwar->mapName().c_str(),Broodwar->getStartLocations().size());
  
  // Enable some cheat flags
  Broodwar->enableFlag(Flag::UserInput);
  // Uncomment to enable complete map information
  //Broodwar->enableFlag(Flag::CompleteMapInformation);

  //read map information into BWTA so terrain analysis can be done in another thread
  BWTA::readMap();
  analyzed=false;
  analysis_just_finished=false;
  show_visibility_data=false;

  if (Broodwar->isReplay())
  {
    Broodwar->printf("The following players are in this replay:");

    for(std::set<Player*>::iterator p=Broodwar->getPlayers().begin();p!=Broodwar->getPlayers().end();p++)
    {
      if (!(*p)->getUnits().empty() && !(*p)->isNeutral())
      {
        Broodwar->printf("%s, playing as a %s",(*p)->getName().c_str(),(*p)->getRace().getName().c_str());
      }
    }
  }
  else
  {
    Broodwar->printf("The match up is %s v %s",
      Broodwar->self()->getRace().getName().c_str(),
      Broodwar->enemy()->getRace().getName().c_str());

	//---------------------------------------------

	// Obtiene la posicion del centro de comando Terran
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
		if ( (*i)->getType().getID() == 106){
	//		centroComando = new Position((*i)->getPosition().x(), (*i)->getPosition().y());
		}
	}

	Broodwar->setLocalSpeed(8);
	agente = new ManoloBot();

	//---------------------------------------------

    //send each worker to the mineral field that is closest to it
/*    for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
    {
      if ((*i)->getType().isWorker())
      {
        Unit* closestMineral=NULL;

        for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++)
        {
          if (closestMineral==NULL || (*i)->getDistance(*m)<(*i)->getDistance(closestMineral))
            closestMineral=*m;
        }
        if (closestMineral!=NULL)
          (*i)->rightClick(closestMineral);
      }
      else if ((*i)->getType().isResourceDepot())
      {
        //if this is a center, tell it to build the appropiate type of worker
        if ((*i)->getType().getRace()!=Races::Zerg)
        {
          (*i)->train(*Broodwar->self()->getRace().getWorker());
        }
        else //if we are Zerg, we need to select a larva and morph it into a drone
        {
          std::set<Unit*> myLarva=(*i)->getLarva();
          if (myLarva.size()>0)
          {
            Unit* larva=*myLarva.begin();
            larva->morph(UnitTypes::Zerg_Drone);
          }
        }
      }
    }*/
  }
}


void ExampleAIModule::onEnd(bool isWinner)
{
  if (isWinner)
  {
    //log win to file
  }
}


void ExampleAIModule::onFrame()
{
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
	if (analyzed && Broodwar->getFrameCount()%30==0)
	{
		Broodwar->printf("Entro a la opcion 1");

		//order one of our workers to guard our chokepoint.
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
			if ((*i)->getType().isWorker())
			{
				//get the chokepoints linked to our home region
				std::set<BWTA::Chokepoint*> chokepoints= home->getChokepoints();
				double min_length=10000;
				BWTA::Chokepoint* choke=NULL;

				//iterate through all chokepoints and look for the one with the smallest gap (least width)
				for(std::set<BWTA::Chokepoint*>::iterator c=chokepoints.begin();c!=chokepoints.end();c++)
				{
					double length=(*c)->getWidth();

					if (length<min_length || choke==NULL)
					{
						min_length=length;
						choke=*c;
					}
				}

				//order the worker to move to the center of the gap
				(*i)->rightClick(choke->getCenter());
				break;
			}
		}
	}

	if (analyzed)
	{
		Broodwar->printf("Entro a la opcion 2");

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
		Broodwar->printf("Entro a la opcion 3");

		Broodwar->printf("Finished analyzing map.");
		analysis_just_finished=false;
	}

	//-------------------------------------------------------------------------

	agente->checkGoals();
	
	//-----
	//cantBarracas = 0;
	
	// Busca la cantidad de barracas construidas actualmente (solo para Terran)
	/*for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		if ((*i)->getType().getID() == 111)
		{
			cantBarracas++;
		}
	}
/***************
	  if (trabajador2 == NULL)
	  Broodwar->printf("A manolo lo sacan de la lista");
	  else 	  if (trabajador2->exists()) Broodwar->printf("MANOLO LIVES!");
				else Broodwar->printf("MANOLO LIVES! pero est� muertecito");
	
	if ((Broodwar->self()->minerals() > 150)){
		
		// Si no hay suficientes barracas construye una mas
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
			if ((*i)->getType().isWorker())
			{
				//(*i)->getType().is
				//Position *p = new Position( (*i)->getPosition().x() + rand() % 100, (*i)->getPosition().y() + rand() % 100);
				//Position *p = new Position( (*centroComando).x() + rand() % 100, (*centroComando).y() + rand() & 100);

				Position *p = new Position( (*centroComando).x() + (rand() % 100), (*centroComando).y() + (rand() % 100));


				//if (trabajador == NULL) Broodwar->printf("A por ellos! ");
				//				else Broodwar->printf("soy gallego pero no nulo");

				if (trabajador == NULL){
					// 111 es el codigo de identificacion de la construccion de una barraca terran
					if ( Broodwar->canBuildHere((*i), (*p), *(new UnitType(111)) )){		
						
						if (Broodwar->isExplored(p->x(), p->y())){
							(*i)->build((*p), *(new UnitType(111)) );

							//Broodwar->isBuildable
							if ((*i)->isConstructing()){
								cantBarracas++;
								trabajador = (*i);

								Broodwar->printf("Enhorabuena manolo, has construido una barraca!");
								//trabajador->cancelConstruction();

								//break;
							}
							else{
								Broodwar->printf("Fuiste manolo");
							}
							break;
						
						}
						else{
							trabajador = (*i);
							(*i)->rightClick(*p);
							break;
						}

					}
				}
				else{
					if (trabajador->isIdle()){

						if (cantBarracas==1){
							Unit* closestMineral=NULL;

							for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++)
							{
								if (closestMineral==NULL || trabajador->getDistance(*m)<trabajador->getDistance(closestMineral))
									closestMineral=*m;
							}

							if (closestMineral!=NULL) trabajador->rightClick(closestMineral);
							}
						else {
							trabajador->build(trabajador->getPosition(), *(new UnitType(111)) );
							Broodwar->printf("EAEA manolo");
							cantBarracas++;
						}
						
					}
					
				}
			}
		}
	}
*/
	//(*i)->train(*Broodwar->self()->getRace().getWorker());

	//-------------------------------------------------------------------------

}

void ExampleAIModule::onUnitCreate(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay())
		Broodwar->sendText("A %s [%x] has been created at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
		if (unit->getType().isBuilding()){
			agente->edificioConstruido(unit->getType().getID());
			Broodwar->printf("al� Extramadura al� al�");
		}
	else
	{
		/*if we are in a replay, then we will print out the build order
		(just of the buildings, not the units).*/
		if (unit->getType().isBuilding() && unit->getPlayer()->isNeutral()==false)
		{
			int seconds=Broodwar->getFrameCount()/24;
			int minutes=seconds/60;
			seconds%=60;
			Broodwar->sendText("%.2d:%.2d: %s creates a %s",minutes,seconds,unit->getPlayer()->getName().c_str(),unit->getType().getName().c_str());
		}
	}

	//-------------------------------------------------------------------

}


void ExampleAIModule::onUnitDestroy(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay())
    Broodwar->sendText("A %s [%x] has been destroyed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
	

}

void ExampleAIModule::onUnitMorph(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay())
    Broodwar->sendText("A %s [%x] has been morphed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
  else
  {
    /*if we are in a replay, then we will print out the build order
    (just of the buildings, not the units).*/
    if (unit->getType().isBuilding() && unit->getPlayer()->isNeutral()==false)
    {
      int seconds=Broodwar->getFrameCount()/24;
      int minutes=seconds/60;
      seconds%=60;
      Broodwar->sendText("%.2d:%.2d: %s morphs a %s",minutes,seconds,unit->getPlayer()->getName().c_str(),unit->getType().getName().c_str());
    }
  }
}
void ExampleAIModule::onUnitShow(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay())
    Broodwar->sendText("A %s [%x] has been spotted at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}
void ExampleAIModule::onUnitHide(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay())
    Broodwar->sendText("A %s [%x] was last seen at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}
void ExampleAIModule::onUnitRenegade(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay())
    Broodwar->sendText("A %s [%x] is now owned by %s",unit->getType().getName().c_str(),unit,unit->getPlayer()->getName().c_str());
}
void ExampleAIModule::onPlayerLeft(BWAPI::Player* player)
{
  Broodwar->sendText("%s left the game.",player->getName().c_str());
}
void ExampleAIModule::onNukeDetect(BWAPI::Position target)
{
  if (target!=Positions::Unknown)
    Broodwar->printf("Nuclear Launch Detected at (%d,%d)",target.x(),target.y());
  else
    Broodwar->printf("Nuclear Launch Detected");
}

bool ExampleAIModule::onSendText(std::string text)
{
  if (text=="/show players")
  {
    showPlayers();
    return false;
  } else if (text=="/show forces")
  {
    showForces();
    return false;
  } else if (text=="/show visibility")
  {
    show_visibility_data=true;
  } else if (text=="/analyze")
  {
    if (analyzed == false)
    {
      Broodwar->printf("Analyzing map... this may take a minute");
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
    }
    return false;
  } else
  {
    Broodwar->printf("You typed '%s'!",text.c_str());
  }
  return true;
}

DWORD WINAPI AnalyzeThread()
{
  BWTA::analyze();
  analyzed   = true;
  analysis_just_finished = true;

  //self start location only available if the map has base locations
  if (BWTA::getStartLocation(BWAPI::Broodwar->self())!=NULL)
  {
    home       = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
  }
  //enemy start location only available if Complete Map Information is enabled.
  if (BWTA::getStartLocation(BWAPI::Broodwar->enemy())!=NULL)
  {
    enemy_base = BWTA::getStartLocation(BWAPI::Broodwar->enemy())->getRegion();
  }
  return 0;
}

void ExampleAIModule::drawStats()
{
  std::set<Unit*> myUnits = Broodwar->self()->getUnits();
  Broodwar->drawTextScreen(5,0,"I have %d units:",myUnits.size());
  std::map<UnitType, int> unitTypeCounts;
  for(std::set<Unit*>::iterator i=myUnits.begin();i!=myUnits.end();i++)
  {
    if (unitTypeCounts.find((*i)->getType())==unitTypeCounts.end())
    {
      unitTypeCounts.insert(std::make_pair((*i)->getType(),0));
    }
    unitTypeCounts.find((*i)->getType())->second++;
  }
  int line=1;
  for(std::map<UnitType,int>::iterator i=unitTypeCounts.begin();i!=unitTypeCounts.end();i++)
  {
    Broodwar->drawTextScreen(5,16*line,"- %d %ss",(*i).second, (*i).first.getName().c_str());
    line++;
  }
}

void ExampleAIModule::showPlayers()
{
  std::set<Player*> players=Broodwar->getPlayers();
  for(std::set<Player*>::iterator i=players.begin();i!=players.end();i++)
  {
    Broodwar->printf("Player [%d]: %s is in force: %s",(*i)->getID(),(*i)->getName().c_str(), (*i)->getForce()->getName().c_str());
  }
}
void ExampleAIModule::showForces()
{
  std::set<Force*> forces=Broodwar->getForces();
  for(std::set<Force*>::iterator i=forces.begin();i!=forces.end();i++)
  {
    std::set<Player*> players=(*i)->getPlayers();
    Broodwar->printf("Force %s has the following players:",(*i)->getName().c_str());
    for(std::set<Player*>::iterator j=players.begin();j!=players.end();j++)
    {
      Broodwar->printf("  - Player [%d]: %s",(*j)->getID(),(*j)->getName().c_str());
    }
  }
}