#include "GrupoBunkers.h"

GrupoBunkers::GrupoBunkers(void)
{
}

GrupoBunkers::~GrupoBunkers(void)
{
}

void GrupoBunkers::agregarBunker(Unit* u){
	if ((u != NULL) && (u->getType().getID() == Utilidades::ID_BUNKER)){
		bunkers.push_front(u);
	}
}

Unit* GrupoBunkers::getUltimoBunkerCreado(){
	if (bunkers.size() > 0)
		return bunkers.front();
	else 
		return NULL;
}

int GrupoBunkers::getCantBunkers(){
	return bunkers.size();
}