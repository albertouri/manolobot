#include "CompaniaDefensiva.h"

CompaniaDefensiva::CompaniaDefensiva(Color ID){
	c = ID;
}

CompaniaDefensiva::~CompaniaDefensiva(void){

}

void CompaniaDefensiva::onFrame(){

	// ------------------------ realiza un recuadro a las unidades de la compañia ------------------------

	if (listMarines.size() > 0){

		std::list<Unit*>::iterator It1;
		It1 = listMarines.begin();

		while(It1 != listMarines.end()){
			if(!(*It1)->exists()) It1 = listMarines.erase(It1);	
			else {
				Unit *u;
				
				if (!(*It1)->isLoaded()){
					// dibuja una linea conectando a la unidad con su objetivo actual
					if ((*It1)->getTarget() != NULL){
						u = (*It1)->getTarget();
						Broodwar->drawLine(CoordinateType::Map, ((*It1)->getTilePosition().x() + (*It1)->getType().tileWidth()) * 32, ((*It1)->getTilePosition().y() + (*It1)->getType().tileHeight()) * 32, u->getPosition().x(), u->getPosition().y(), Colors::Red);
					}
					else if ((*It1)->getOrderTarget() != NULL){
						u = (*It1)->getOrderTarget();
						Broodwar->drawLine(CoordinateType::Map, ((*It1)->getPosition().x() + (*It1)->getType().tileWidth()) * 32, ((*It1)->getTilePosition().y() + (*It1)->getType().tileHeight()) * 32, u->getPosition().x(), u->getPosition().y(), Colors::Red);
					}

					Graficos::resaltarUnidad(*It1, c);
				}

				It1++;
			}
		}
	}
}

int CompaniaDefensiva::countMarines(){
	int j = 0;

	if (listMarines.size() > 0){

		std::list<Unit*>::iterator It1;
		It1 = listMarines.begin();

		while(It1 != listMarines.end()){
			if (!(*It1)->exists()) It1 = listMarines.erase(It1);	
			else {It1++; j++;}
		}
	}

	return j;
}


void CompaniaDefensiva::asignarUnidad(Unit *u){
	
	if (u->getType().getID() == Utilidades::ID_MARINE){
		listMarines.push_back(u);
	}
			
}


void CompaniaDefensiva::atacar(Position p){
	std::list<Unit*>::iterator It1;
	It1 = listMarines.begin();

	while(It1 != listMarines.end()){
		if ((*It1)->exists()){
			(*It1)->useTech(TechTypes::Stim_Packs);
			(*It1)->attackMove(p);
		}

		It1++;
	}
}

