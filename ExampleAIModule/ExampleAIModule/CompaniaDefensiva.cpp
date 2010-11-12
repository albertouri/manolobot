#include "CompaniaDefensiva.h"

// Setea un color especifico para resaltar las unidades de la compañia
CompaniaDefensiva::CompaniaDefensiva(Color ID){
	c = ID;
}

CompaniaDefensiva::~CompaniaDefensiva(void){

}


void CompaniaDefensiva::onFrame(){

	if (Broodwar->getFrameCount() % 45 == 0){
		controlarEliminados();
	}
	else if (Broodwar->getFrameCount() % 45 == 10){
		// Busca objetivos para los fantasmas de la compañia y los ataca
		defenderBaseGhosts();
	}
	else if (Broodwar->getFrameCount() % 45 == 20){
		// Busca objetivos para los medicos de la compañia y los cura
		defenderBaseMedics();
	}

	// Descomentar para realizar un recuadro a las unidades de la compañia defensiva
	//recuadrarUnidades();
}


int CompaniaDefensiva::countMarines(){
	int j = 0;

	if (!listMarines.empty()){

		std::list<Unit*>::iterator It1;
		It1 = listMarines.begin();

		while(It1 != listMarines.end()){
			if (!(*It1)->exists()){
				It1 = listMarines.erase(It1);
				It1 = listMarines.begin();
			}
			else {
				It1++; 
				j++;
			}
		}
	}

	return j;
}


void CompaniaDefensiva::controlarEliminados(){
	std::list<Unit*>::iterator It;

	if (!listMarines.empty()){
		It = listMarines.begin();
		while (It != listMarines.end()){
			if (!(*It)->exists()){
				listMarines.erase(It);
				It = listMarines.begin(); // si se elimina esta linea el juego normalmente se cuelga...
			}
			else
				It++;
		}
	}

	if (!listMedics.empty()){
		It = listMedics.begin();
		while (It != listMedics.end()){
			if (!(*It)->exists()){
				listMedics.erase(It);
				It = listMedics.begin(); // si se elimina esta linea el juego normalmente se cuelga...
			}
			else
				It++;
		}
	}

	if (!listGhosts.empty()){
		It = listGhosts.begin();
		while (It != listGhosts.end()){
			if (!(*It)->exists()){
				listGhosts.erase(It);
				It = listGhosts.begin(); // si se elimina esta linea el juego normalmente se cuelga...
			}
			else
				It++;
		}
	}

}


void CompaniaDefensiva::asignarUnidad(Unit *u){
	if (u->getType().getID() == Utilidades::ID_MARINE)
		listMarines.push_back(u);
	else if (u->getType().getID() == Utilidades::ID_MEDIC)
		listMedics.push_back(u);
	else if (u->getType().getID() == Utilidades::ID_GHOST)
		listGhosts.push_back(u);
}


void CompaniaDefensiva::atacar(Unit *u){
	
	if (!listMarines.empty()){
		std::list<Unit*>::iterator It1;
		It1 = listMarines.begin();

		// Aplica el stim pack a los marines y los manda a atacar a la unidad pasada como parametro
		while(It1 != listMarines.end()){
			if ((*It1)->exists() && u->exists() && u->isVisible()){
				(*It1)->useTech(TechTypes::Stim_Packs);
				(*It1)->attackUnit(u);
			}

			It1++;
		}
	}
}

void CompaniaDefensiva::atacar(Position p){
	if (!listMarines.empty()){
		std::list<Unit*>::iterator It1;
		It1 = listMarines.begin();

		// Aplica el stim pack a los marines y los manda a atacar todas las unidades que encuentren en el camino hacia la posicion recibida como parametro
		while(It1 != listMarines.end()){
			if ((*It1)->exists()){
				(*It1)->useTech(TechTypes::Stim_Packs);
				(*It1)->attackMove(p);
			}

			It1++;
		}
	}
}


bool CompaniaDefensiva::faltanMarines(){
	return (listMarines.size() < 5);
}

bool CompaniaDefensiva::faltanMedics(){
	return (listMedics.size() < 2);
}

bool CompaniaDefensiva::faltanGhosts(){
	return ((listGhosts.size() < 2) && ((Broodwar->enemy()->getRace() == Races::Terran) || (Broodwar->enemy()->getRace() == Races::Protoss)));
}


Unit* CompaniaDefensiva::buscarObjetivosGhost(){
	// Los objetivos para los fantasmas seran unidades mecanicas, ya que las mismas pueden ser locked down
	Unit *objetivo = NULL;

	if (!Broodwar->enemy()->getUnits().empty()){
		std::set<Unit*>::const_iterator ItObj;
		
		Region *inicial = BWTA::getStartLocation(Broodwar->self())->getRegion();

		ItObj = Broodwar->enemy()->getUnits().begin();
		while (ItObj != Broodwar->enemy()->getUnits().end()){
			// Busca una unidad mecanica que este dentro de la region defendida por el fantasma o una unidad mecanica que este atacando alguna unidad nuestra que este dentro de la region defendida por la compañia defensiva
			if ((*ItObj)->exists() && ((*ItObj)->isVisible()) && (*ItObj)->getType().isMechanical() && (!(*ItObj)->isLockedDown()) && (inicial->getPolygon().isInside((*ItObj)->getPosition()) || ((*ItObj)->getOrderTarget() != NULL && inicial->getPolygon().isInside((*ItObj)->getOrderTarget()->getPosition())) || (((*ItObj)->getTarget() != NULL) && inicial->getPolygon().isInside((*ItObj)->getTarget()->getPosition())))){
				if (objetivo == NULL){
					objetivo = (*ItObj);
					return objetivo;
				}
				else{
					// Busca unidad enemiga de mayor tamaño que cumpla las condiciones para ser atacada por los fantasmas
					if ((objetivo->getType().size() == UnitSizeTypes::Small) && (((*ItObj)->getType().size() == UnitSizeTypes::Medium) || ((*ItObj)->getType().size() == UnitSizeTypes::Large)))
						objetivo = (*ItObj);
					else if ((objetivo->getType().size() == UnitSizeTypes::Medium) && ((*ItObj)->getType().size() == UnitSizeTypes::Large))
						objetivo = (*ItObj);
				}
			}

			ItObj++;
		}
	}

	return objetivo;
}

Unit* CompaniaDefensiva::buscarObjetivosMedics(){
	// Los medicos de la compañia defensiva se encargan de aplicar restauracion a todas las unidades aliadas que encuentre dentro de la region defendida por la compañia defensiva
	Unit *objetivo = NULL;

	if (!Broodwar->self()->getUnits().empty()){
		std::set<Unit*>::const_iterator ItObj;
		
		Region *inicial = BWTA::getStartLocation(Broodwar->self())->getRegion();

		ItObj = Broodwar->self()->getUnits().begin();
		while (ItObj != Broodwar->self()->getUnits().end()){
			// Busca alguna unidad aliada que requiera restauracion
			if ((*ItObj)->exists() && ((*ItObj)->isLockedDown() || (*ItObj)->isBlind() || (*ItObj)->isIrradiated() || (*ItObj)->isPlagued() || (*ItObj)->isEnsnared() || (*ItObj)->isParasited()) && inicial->getPolygon().isInside((*ItObj)->getPosition())){
				if (objetivo == NULL){
					objetivo = (*ItObj);
					return objetivo;
				}
			}

			ItObj++;
		}
	}

	return objetivo;
}

Unit* CompaniaDefensiva::buscarObjetivosMarines(){
	// Los marines atacan a cualquier unidad enemiga que haya entrado a la region defendida por la compañia defensiva
	Unit *objetivo = NULL;

	if (!Broodwar->enemy()->getUnits().empty()){
		std::set<Unit*>::const_iterator ItObj;
		
		Region *inicial = BWTA::getStartLocation(Broodwar->self())->getRegion();

		ItObj = Broodwar->enemy()->getUnits().begin();
		while (ItObj != Broodwar->enemy()->getUnits().end()){
			// Busca la primer unidad enemiga que este dentro de la region defendida por la compañia defensiva
			if ((*ItObj)->exists() && ((*ItObj)->isVisible()) && ((*ItObj)->isDetected()) && (inicial->getPolygon().isInside((*ItObj)->getPosition()))){
				if (objetivo == NULL){
					objetivo = (*ItObj);
					return objetivo;
				}
				else{
					if ((objetivo->getType().size() == UnitSizeTypes::Large) && (((*ItObj)->getType().size() == UnitSizeTypes::Medium) || ((*ItObj)->getType().size() == UnitSizeTypes::Small)))
						objetivo = (*ItObj);
					else if ((objetivo->getType().size() == UnitSizeTypes::Medium) && ((*ItObj)->getType().size() == UnitSizeTypes::Small))
						objetivo = (*ItObj);
				}
			}

			ItObj++;
		}
	}

	return objetivo;
}


void CompaniaDefensiva::defenderBaseGhosts(){

	if (!listGhosts.empty()){
		Unit* objetivo;
		std::list<Unit*>::iterator It;

		// Busca un objetivo para los fantasmas
		objetivo = buscarObjetivosGhost();

		if ((objetivo != NULL) && (objetivo->exists())){
			It = listGhosts.begin();
			bool encontre = false;

			// Busca un fantasma en la compañia defensiva que tenga suficiente energia para ejecutar un lockdown sobre el objetivo
			while ((It != listGhosts.end()) && (!encontre)){
				if ((*It)->exists() && ((*It)->getEnergy() >= 100)){
					(*It)->useTech(TechTypes::Lockdown, objetivo);
					encontre = true;
				}
				It++;
			}
		}
	}
}


void CompaniaDefensiva::defenderBaseMedics(){

	if (!listMedics.empty()){
		// Busca un objetivo para los medicos
		Unit *objetivo = buscarObjetivosMedics();
		std::list<Unit*>::iterator It;
		bool encontre = false;
		
		if ((objetivo != NULL) && (objetivo->exists())){
			It = listMedics.begin();

			// Busca un medico que tenga suficiente energia para realizar un restoration sobre el objetivo
			while (It != listMedics.end() && (!encontre)){
				if ((*It)->exists() && ((*It)->getEnergy() >= 50)){
					(*It)->useTech(TechTypes::Restoration, objetivo);
					encontre = true;
				}
				It++;
			}
		}
	}
} 

void CompaniaDefensiva::defenderBaseMarines(){
	
	if (!listMarines.empty()){
		std::list<Unit*>::iterator It = listMarines.begin();
		bool atacando = false;

		while (It != listMarines.end()){
			if ((*It)->exists())
				atacando = atacando || (*It)->isAttacking();

			It++;
		}

		// Si los marines no estan atacando, busca un objetivo y los manda a atacar
		if (!atacando){
			Unit* objetivo = buscarObjetivosMarines();
			
			if (objetivo != NULL){
				It = listMarines.begin();

				while (It != listMarines.end()){
					if ((*It)->exists())
						(*It)->attackUnit(objetivo);

					It++;
				}
			}
		}
	}
}

void CompaniaDefensiva::recuadrarUnidades(){
	// ------------------------ realiza un recuadro a las unidades de la compañia ------------------------
	if (!listMarines.empty()){
		std::list<Unit*>::iterator It1;
		It1 = listMarines.begin();

		while(It1 != listMarines.end()){
			if((*It1)->exists()){
				Unit *u;
				
				if (!(*It1)->isLoaded()){
					// dibuja una linea conectando a la unidad con su objetivo actual
					if ((*It1)->getTarget() != NULL){
						u = (*It1)->getTarget();
						//Broodwar->drawLine(CoordinateType::Map, ((*It1)->getTilePosition().x() + (*It1)->getType().tileWidth()) * 32, ((*It1)->getTilePosition().y() + (*It1)->getType().tileHeight()) * 32, u->getPosition().x(), u->getPosition().y(), Colors::Red);
					}
					else if ((*It1)->getOrderTarget() != NULL){
						u = (*It1)->getOrderTarget();
						//Broodwar->drawLine(CoordinateType::Map, ((*It1)->getPosition().x() + (*It1)->getType().tileWidth()) * 32, ((*It1)->getTilePosition().y() + (*It1)->getType().tileHeight()) * 32, u->getPosition().x(), u->getPosition().y(), Colors::Red);
					}

					//Graficos::resaltarUnidad(*It1, c);
				}
			}

			It1++;			
		}
	}

	if (!listMedics.empty()){
		std::list<Unit*>::iterator It1;
		It1 = listMedics.begin();

		while(It1 != listMedics.end()){
			if((*It1)->exists()){
				Unit *u;
				
				if (!(*It1)->isLoaded()){
					// dibuja una linea conectando a la unidad con su objetivo actual
					if ((*It1)->getTarget() != NULL){
						u = (*It1)->getTarget();
						//Broodwar->drawLine(CoordinateType::Map, ((*It1)->getTilePosition().x() + (*It1)->getType().tileWidth()) * 32, ((*It1)->getTilePosition().y() + (*It1)->getType().tileHeight()) * 32, u->getPosition().x(), u->getPosition().y(), Colors::Red);
					}
					else if ((*It1)->getOrderTarget() != NULL){
						u = (*It1)->getOrderTarget();
						//Broodwar->drawLine(CoordinateType::Map, ((*It1)->getPosition().x() + (*It1)->getType().tileWidth()) * 32, ((*It1)->getTilePosition().y() + (*It1)->getType().tileHeight()) * 32, u->getPosition().x(), u->getPosition().y(), Colors::Red);
					}

					//Graficos::resaltarUnidad(*It1, c);
				}

			}
			It1++;
		}
	}

	if (!listGhosts.empty()){

		std::list<Unit*>::iterator It1;
		It1 = listGhosts.begin();

		while(It1 != listGhosts.end()){
			if((*It1)->exists()){
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
			}
			It1++;
		}
	}


}