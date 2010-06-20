#include "compania.h"
//#include <list>
#include "Utilidades.h"

std::list<Unit*> lista;
Unit *unidad1, *unidad2, *unidad3;
int i=1;
Color *c;



compania::compania(BWAPI::Color ID)
{
	c = new Color(ID);
}

compania::~compania(void)
{
	delete c;
}

void compania::asignarUnidad(Unit *u){
	/*if(i == 1) unidad1 = U;/*
	else if(i==2) unidad2 = U;
	else if(i==3) unidad3 = U;*/

	//i++;
	
	if (u->getType().getID() == Utilidades::ID_MARINE)
		listMarines.push_front(u);
	else if (u->getType().getID() == Utilidades::ID_MEDIC)
		listMedics.push_front(u);
	else if (u->getType().getID() == Utilidades::ID_FIREBAT)
		listFirebats.push_front(u);


	//lista.push_front(u);

	//conteoUnidades();
	//if ((unidad1!=NULL)&&(unidad1->exists())) unidad1->rightClick(*(new TilePosition(1,1)));
	
	//if (i>1) ponerACubierto(u);
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
	
	if (bunker!=NULL) {bunker->load(U); }
}

void compania::aplicarStim(std::list<Unit*> lista){

	if (lista.size() > 0){
		std::list<Unit*>::iterator It1;
		It1 = lista.begin();

		while (It1 != lista.end()){
			// si existe y no esta dentro de un contenedor, se aplica el stim pack a la unidad
			if ((*It1)->exists() && (!(*It1)->isLoaded())){
				(*It1)->useTech(*(new TechType(TechTypes::Stim_Packs)));
			}
			It1++;
		}
	}
}


int compania::countMarines(){
	return contarUnidades(&listMarines);
}

int compania::countMedics(){
	return contarUnidades(&listMedics);
}

int compania::countFirebats(){
	return contarUnidades(&listFirebats);
}

int compania::contarUnidades(std::list<Unit*> *lista){
	int j = 0;

	if (lista->size() > 0){

		std::list<Unit*>::iterator It1;
		It1 = lista->begin();

		while(It1 != lista->end()){
			if(!(*It1)->exists()) It1 = lista->erase(It1);	
			else {It1++; j++;}
		}
	}

	return j;
}

void compania::atacar(Unit *u){
	Broodwar->printf("Iniciando movimiento de ataque");

	if (listMarines.size() > 0){

		std::list<Unit*>::iterator It1;
		It1 = listMarines.begin();

		while(It1 != listMarines.end()){
			if(!(*It1)->exists()) It1 = listMarines.erase(It1);	
			else {
				//(*It1)->attackUnit(u);}
				aplicarStim(listMarines);
				(*It1)->attackMove(u->getPosition());
				It1++;
			}
		}
	}

	if (listFirebats.size() > 0){

		std::list<Unit*>::iterator It1;
		It1 = listFirebats.begin();

		while(It1 != listFirebats.end()){
			if(!(*It1)->exists()) It1 = listFirebats.erase(It1);	
			else {
				aplicarStim(listFirebats);
				(*It1)->attackUnit(u);
				It1++;
			}
		}
	}

	if (listMedics.size() > 0){

		std::list<Unit*>::iterator It1;
		It1 = listMedics.begin();

		while(It1 != listMedics.end()){
			if(!(*It1)->exists()) It1 = listMedics.erase(It1);	
			else { //(*It1)->rightClick(u->getPosition());}
				(*It1)->attackMove(u->getPosition());
				It1++;
			}
		}
	}
	
}

void compania::onFrame(){

	if (listMarines.size() > 0){

		std::list<Unit*>::iterator It1;
		It1 = listMarines.begin();

		while(It1 != listMarines.end()){
			if(!(*It1)->exists()) It1 = listMarines.erase(It1);	
			else {
				Graficos::resaltarUnidad(*It1, *c);
				It1++;
			}
		}
	}

	if (listFirebats.size() > 0){

		std::list<Unit*>::iterator It1;
		It1 = listFirebats.begin();

		while(It1 != listFirebats.end()){
			if(!(*It1)->exists()) It1 = listFirebats.erase(It1);	
			else {
				Graficos::resaltarUnidad(*It1, *c);
				It1++; 
			}
		}
	}

	if (listMedics.size() > 0){

		std::list<Unit*>::iterator It1;
		It1 = listMedics.begin();

		while(It1 != listMedics.end()){
			if(!(*It1)->exists()) It1 = listMedics.erase(It1);	
			else {
				Graficos::resaltarUnidad(*It1, *c);
				It1++;
			}
		}
	}
}