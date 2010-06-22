#include "compania.h"
#include "Utilidades.h"

std::list<Unit*> lista;
Color *c;
Unit *comandante = NULL;


compania::compania(Color ID)
{
	c = new Color(ID);
}

compania::~compania(void)
{
	delete c;
}

void compania::asignarUnidad(Unit *u){
	
	if (u->getType().getID() == Utilidades::ID_MARINE)
		listMarines.push_front(u);
	else if (u->getType().getID() == Utilidades::ID_MEDIC)
		listMedics.push_front(u);
	else if (u->getType().getID() == Utilidades::ID_FIREBAT)
		listFirebats.push_front(u);


	//lista.push_front(u);

	//conteoUnidades();
	//if ((unidad1!=NULL)&&(unidad1->exists())) unidad1->rightClick(*(new TilePosition(1,1)));
	
	ponerACubierto(u);
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
			if (!(*It1)->exists()) It1 = lista->erase(It1);	
			else {It1++; j++;}
		}
	}

	return j;
}

void compania::actualizarEstado(std::list<Unit*> *lista){
	if (lista->size() > 0){
		std::list<Unit*>::iterator It1;
		It1 = lista->begin();

		while(It1 != lista->end()){
			if (!(*It1)->exists()) It1 = lista->erase(It1);	
			else It1++;
		}
	}
}

void compania::atacar(Unit *u){
	//Broodwar->printf("Iniciando movimiento de ataque");

	if (listMarines.size() > 0){

		std::list<Unit*>::iterator It1;
		It1 = listMarines.begin();

		while(It1 != listMarines.end()){
			if(!(*It1)->exists()) It1 = listMarines.erase(It1);	
			else {
				aplicarStim(listMarines);
				(*It1)->attackUnit(u);
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
				//(*It1)->attackMove(u->getPosition());
				(*It1)->follow(comandante);
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
				Unit *u;
				
				if ((!(*It1)->isLoaded()) && ((*It1)->getTarget() != NULL)){
					u = (*It1)->getTarget();
					Broodwar->drawLine(CoordinateType::Map, ((*It1)->getTilePosition().x() + (*It1)->getType().tileWidth()) * 32, ((*It1)->getTilePosition().y() + (*It1)->getType().tileHeight()) * 32, u->getPosition().x(), u->getPosition().y(), Colors::Red);
				}
				else if ((!(*It1)->isLoaded()) && ((*It1)->getOrderTarget() != NULL)){
					u = (*It1)->getOrderTarget();
					Broodwar->drawLine(CoordinateType::Map, ((*It1)->getPosition().x() + (*It1)->getType().tileWidth()) * 32, ((*It1)->getTilePosition().y() + (*It1)->getType().tileHeight()) * 32, u->getPosition().x(), u->getPosition().y(), Colors::Red);
				}

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
				if (!(*It1)->isLoaded()) Graficos::resaltarUnidad(*It1, *c);
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
				if (!(*It1)->isLoaded()) Graficos::resaltarUnidad(*It1, *c);
				It1++;
			}
		}
	}

	if ((comandante == NULL) || (!comandante->exists())){
		actualizarEstado(&listMarines);
		
		if (listMarines.size() > 0)
			comandante = *(listMarines.begin());
		else{
			actualizarEstado(&listFirebats);

			if (listFirebats.size() > 0)
				comandante = *(listFirebats.begin());
			else
				comandante = NULL;
		}
	}

	if (comandante != NULL){
		// si no esta atacando ningun objetivo, se busca algun nuevo objetivo para atacar
		if ((comandante->getTarget() == NULL) || (!comandante->getTarget()->exists())){
			double minDist = 10000;
			Unit *masCercana = NULL;

			for(std::set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++){
				if ((*i)->exists()){
					if (comandante->getPosition().getApproxDistance((*i)->getPosition()) < minDist){
						minDist = comandante->getPosition().getApproxDistance((*i)->getPosition());
						masCercana = *i;
					}
				}
			}

			if (masCercana != NULL)
				atacar(masCercana);
		}
	}
}