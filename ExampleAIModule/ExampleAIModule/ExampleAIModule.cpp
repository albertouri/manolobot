#include "ExampleAIModule.h"
#include "ManoloBot.h"
#include <stdlib.h>

#include "Utilidades.h"

using namespace BWAPI;

ManoloBot *agente;

Unit *sambueza = NULL;


void ExampleAIModule::onStart()
{
  Broodwar->sendText("Hello world!");
  Broodwar->printf("The map is %s, a %d player map",Broodwar->mapName().c_str(),Broodwar->getStartLocations().size());
  
  // Enable some cheat flags
  Broodwar->enableFlag(Flag::UserInput);
  // Uncomment to enable complete map information
  //Broodwar->enableFlag(Flag::CompleteMapInformation);

  /*//read map information into BWTA so terrain analysis can be done in another thread
  BWTA::readMap();
  analyzed=false;
  analysis_just_finished=false;
  show_visibility_data=false;*/

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

	Broodwar->setLocalSpeed(8);
	agente = new ManoloBot();
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
	agente->checkGoals();	

	/*if (sambueza != NULL){
		if (sambueza->isCompleted()){
			Broodwar->printf("BARRACA LISTA");
		}
	}*/
}

void ExampleAIModule::onUnitCreate(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay()){
		Broodwar->sendText("A %s [%x] has been created at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
		if (unit->getType().isBuilding()){
			agente->edificioConstruido(unit->getType().getID());
		}
		else if(unit->getType().getID()==0) agente->unidadConstruida(unit);


		if (unit->getType().getID() == Utilidades::ID_BARRACK){
			sambueza = unit;
		}
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
	if (!Broodwar->isReplay()){}
    //Broodwar->sendText("A %s [%x] has been morphed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
  else
  {
    /*if we are in a replay, then we will print out the build order
    (just of the buildings, not the units).*/
    if (unit->getType().isBuilding() && unit->getPlayer()->isNeutral()==false)
    {
      /*int seconds=Broodwar->getFrameCount()/24;
      int minutes=seconds/60;
      seconds%=60;
      Broodwar->sendText("%.2d:%.2d: %s morphs a %s",minutes,seconds,unit->getPlayer()->getName().c_str(),unit->getType().getName().c_str());*/
    }
  }
}
void ExampleAIModule::onUnitShow(BWAPI::Unit* unit)
{
  //if (!Broodwar->isReplay())
    //Broodwar->sendText("A %s [%x] has been spotted at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}
void ExampleAIModule::onUnitHide(BWAPI::Unit* unit)
{
  //if (!Broodwar->isReplay())
    //Broodwar->sendText("A %s [%x] was last seen at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
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
    /*if (analyzed == false)
    {
      Broodwar->printf("Analyzing map... this may take a minute");
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
    }*/
    return false;
  } else
  {
    Broodwar->printf("You typed '%s'!",text.c_str());
  }
  return true;
}

void ExampleAIModule::onSaveGame(std::string gameName)
{
  Broodwar->sendText("User just save game with file name %s",gameName.c_str());
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