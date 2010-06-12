#include <BWAPI.h>
#include "compania.h"
#include <list>
#include "Utilidades.h"

std::list<Unit*> lista;
Unit *unidad1, *unidad2, *unidad3;
int i=1;

compania::compania(void)
{
}

compania::~compania(void)
{
}

void compania::asignarUnidad(Unit *U){
	if(i == 1) unidad1 = U;/*
	else if(i==2) unidad2 = U;
	else if(i==3) unidad3 = U;*/

	i++;
	
	lista.push_front(U);

	//conteoUnidades();
	if ((unidad1!=NULL)&&(unidad1->exists())) unidad1->rightClick(*(new TilePosition(1,1)));
	
	if (i>1) ponerACubierto(U);

}

void compania::conteoUnidades(){
	int j=0;
	std::list<Unit*>::iterator It1;
    It1 = lista.begin();

	while(It1 != lista.end()){
		if(!(*It1)->exists()) It1 = lista.erase(It1);	
		else {It1++; j++;}
	}
}

void compania::ponerACubierto(Unit* U){
	Unit* bunker;
	
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
		if (((*i)->getType().getID()==125)&&((*i)->getLoadedUnits().size()<4)){
			bunker = (*i);
			break;
		}
	}
	
	if (bunker!=NULL) {bunker->load(U); };
}

void compania::aplicarStim(){

	if (lista.size() > 0){
		std::list<Unit*>::iterator It1;
		It1 = lista.begin();

		while (It1 != lista.end()){
			if (((*It1)->getType().getID() == Utilidades::ID_MARINE) || ((*It1)->getType().getID() == Utilidades::ID_FIREBAT)){
				
				// si no esta dentro de un contenedor, se aplica el stim pack a la unidad
				if (!(*It1)->isLoaded())
					(*It1)->useTech(*(new TechType(TechTypes::Stim_Packs)));
			}
		}
	}
	
}