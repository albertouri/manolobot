#include "GrupoBunkers.h"

/*GrupoBunkers::GrupoBunkers(void)
{
}*/
GrupoBunkers::GrupoBunkers(AnalizadorTerreno *a)
{
	analizador = a;
	choke = a->obtenerChokepoint();
	reg = a->regionInicial();
}

GrupoBunkers::~GrupoBunkers(void)
{
	delete choke;
	delete reg;
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


// el bunker atacado es pasado como parametro
void GrupoBunkers::estrategia1(Unit *u){

	if (perteneceBunker(u)){

		if (u->getLoadedUnits().size() > 0){
			std::list<Unit*>::iterator It1;
			std::list<Unit*>::iterator It2;
			std::list<Unit*> temp;
			
			It1 = u->getLoadedUnits().begin();

			//u->unloadAll();

			// si el bunker atacado pertenece al grupo actual, expulsa a los soldados, les aplica el stim pack y los vuelve a ingresar al bunker
			while (It1 != u->getLoadedUnits().end()){
				//if (((*It1)->getType().getID() == Utilidades::ID_MARINE) || ((*It1)->getType().getID() == Utilidades::ID_FIREBAT)){
					
					//if ((*It1)->exists() && ((!(*It1)->isStimmed()) || ((*It1)->getStimTimer() == 0))){
						// agrega al soldado a una lista temporal
						temp.push_front(*It1);
						//u->unload(*It1);
					//}
				//}
				It1++;
			}
		
			u->unloadAll();

			// ahora vuelve a cargar los soldados en el bunker
			It2 = temp.begin();

			while (It2 != temp.end()){
				// si no esta dentro de un contenedor, se aplica el stim pack a la unidad
				/*if (!(*It1)->isLoaded())
					(*It1)->useTech(*(new TechType(TechTypes::Stim_Packs)));*/

				u->load(*It2);
				It2++;
			}

		}
	}
}


bool GrupoBunkers::perteneceBunker(Unit *u){

	if (u != NULL){
		std::list<Unit*>::iterator It1;
		It1 = bunkers.begin();

		while (It1 != bunkers.end()){
			if ((*It1)->getID() == u->getID())
				return true;
			It1++;
		}
	}

	return false;
}

TilePosition* GrupoBunkers::posicionNuevoBunker(){
	//if (getCantBunkers() == 0){
		return analizador->calcularPrimerTile(reg, choke);
	/*}
	else{

	}*/
}