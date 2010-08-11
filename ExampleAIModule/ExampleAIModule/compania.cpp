#include "compania.h"
#include "Utilidades.h"

std::list<Unit*> lista;
int latencia=0;

bool atacando = false;
Position posicionanteriorDelComandante;

std::list<Unit*> listaDeTanquesAUbicar;

compania::compania(Color ID)
{
	c = ID;
	comandante = NULL;
}

compania::~compania(void)
{
}

void compania::asignarUnidad(Unit *u){

	if (u->getType().getID() == Utilidades::ID_TANKSIEGE){
		if ((comandante!=NULL)&&(comandante->exists())){
			u->rightClick(comandante->getPosition());
		}
		else{
			Unit* bunker;
			for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
				if (((*i)->getType().getID()==Utilidades::ID_BUNKER)){
					bunker = (*i);
					break;
				}
			}
			if ((bunker != NULL)&&(bunker->exists())) u->rightClick(bunker->getPosition());
		}
		listTanks.push_front(u);
//		listaDeTanquesAUbicar.push_front(u);
	}
	else{
		if (u->getType().getID() == Utilidades::ID_MARINE){
			//listMarines.push_front(u);
			listMarines.push_back(u);
			
			//ponerACubierto(u);
		}
		else if (u->getType().getID() == Utilidades::ID_MEDIC)
			//listMedics.push_front(u);
			listMedics.push_back(u);
		else if (u->getType().getID() == Utilidades::ID_FIREBAT)
			//listFirebats.push_front(u);
			listFirebats.push_back(u);
		else if (u->getType().getID() == Utilidades::ID_GOLIATH){
			listGoliath.push_back(u);
		}
		if ((comandante != NULL)&&(comandante->exists()))
			u->rightClick(comandante);
	}
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
			if ((*It1)->exists() && (!(*It1)->isLoaded()) && (!(*It1)->isStimmed())){
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

	if (listMarines.size() > 0){
		std::list<Unit*>::iterator It1;
		It1 = listMarines.begin();

		while(It1 != listMarines.end()){
			if(!(*It1)->exists()) It1 = listMarines.erase(It1);	
			else {
				(*It1)->attackUnit(u);
				It1++;
			}
		}
		aplicarStim(listMarines);
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
			else { 
			//	(*It1)->follow(comandante);
			//	(*It1)->rightClick(u->getPosition());
				It1++;
			}
		}
	}

	if (listGoliath.size() > 0){

		std::list<Unit*>::iterator It1;
		It1 = listGoliath.begin();

		while(It1 != listGoliath.end()){
			if(!(*It1)->exists()) It1 = listGoliath.erase(It1);	
			else {
				(*It1)->attackUnit(u);
				It1++;
			}
		}
	}

	if (listTanks.size() > 0){
		std::list<Unit*>::iterator It1;
		It1 = listTanks.begin();

		while(It1 != listTanks.end()){
			if((*It1)->exists() && ((*It1)->getDistance(u->getPosition()) > (*It1)->getType().seekRange())){
				if ((*It1)->isSieged()){
					(*It1)->unsiege();
				}
				(*It1)->attackUnit(u);
			}
			else{
				if (!(*It1)->isSieged()){
						(*It1)->siege();
				}
				(*It1)->attackUnit(u);
			}
			It1++;
		}
	}
	
}


Unit* compania::buscarDañado(std::list<Unit*> lista){
	if (lista.size() > 0){
		std::list<Unit*>::iterator It1;
		It1 = lista.begin();

		while(It1 != lista.end()){
			if ((*It1)->exists() && (!(*It1)->isLoaded()) && ((*It1)->getType().maxHitPoints() > (*It1)->getHitPoints()) && (!(*It1)->isBeingHealed())){
				return 	(*It1);
			}

			It1++;
		}
	}
	return NULL;
}


void compania::onFrame(){

	// ------------------------ realiza un recuadro a las unidades de la compañia ------------------------

	if (listMarines.size() > 0){
		//Broodwar->printf("Entra a 1");

		std::list<Unit*>::iterator It1;
		It1 = listMarines.begin();

		while(It1 != listMarines.end()){
			if(!(*It1)->exists()){
				It1 = listMarines.erase(It1);	
			}
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

					if ((comandante != NULL) && (*It1)->getID() == comandante->getID()){
						Graficos::dibujarCirculo(*(new TilePosition((*It1)->getTilePosition().x() + 1, (*It1)->getTilePosition().y() + 1)), 1, 1);
					}
				}

				It1++;
			}
		}
	}

	if (listFirebats.size() > 0){
		//Broodwar->printf("Entra a 2");

		std::list<Unit*>::iterator It1;
		It1 = listFirebats.begin();

		while(It1 != listFirebats.end()){
			if(!(*It1)->exists()){
				It1 = listFirebats.erase(It1);	
			}
			else {
				if (!(*It1)->isLoaded()) Graficos::resaltarUnidad(*It1, c);
				It1++; 
			}
		}
	}

	if (listGoliath.size() > 0){
		std::list<Unit*>::iterator It1;
		It1 = listGoliath.begin();

		while(It1 != listGoliath.end()){
			if(!(*It1)->exists()) {
				It1 = listGoliath.erase(It1);	
			}
			else {
				Graficos::resaltarUnidad(*It1, c);
				It1++; 
			}
		}
	}



	if (listTanks.size() > 0){
		std::list<Unit*>::iterator It1;
		It1 = listTanks.begin();

		while(It1 != listTanks.end()){
			if(!(*It1)->exists()) {
				It1 = listTanks.erase(It1);	
			}
			else {
				Graficos::resaltarUnidad(*It1, c);
				It1++; 
			}
		}
	}


	// ------------------------ verifica si el comandante esta seteado ------------------------

	// si no hay comandante, o murio, se asigna uno nuevo
	if ((comandante == NULL) || (!comandante->exists())){
		
		actualizarEstado(&listMarines);
		
		if (listMarines.size() > 0){
			comandante = *(listMarines.begin());
			posicionanteriorDelComandante = comandante->getPosition();
		}
		else{
			actualizarEstado(&listFirebats);

			if (listFirebats.size() > 0){
				comandante = *(listFirebats.begin());
				posicionanteriorDelComandante = comandante->getPosition();
			}
			else
				comandante = NULL;
		}
	}

	// ------------------------  Ordenes de ataque ------------------------

	if ((comandante != NULL)&& (comandante->exists())){
		//Broodwar->printf("Entra a 5");
		// si el comandante no esta atacando ningun objetivo, se busca algun nuevo objetivo para atacar
		if ((comandante->getTarget() == NULL) || (!comandante->getTarget()->exists())){
			
//			if(listMarines.size() > 9){
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

				if ((masCercana != NULL)&& (masCercana->exists())){
					atacar(masCercana);
					atacando = true;
				}
				else{
					atacando = false;
				}
//			}
		}

	}

	if (!atacando){
		if ((comandante!= NULL)&&(comandante->exists()))
			controlarDistancia(); // hace que los soldados sigan al comandante		
		/*
			if (posicionanteriorDelComandante != comandante->getPosition()){
				controlarDistancia(); // hace que los soldados sigan al comandante		
			}
			else{
				if (listMedics.size() > 0){
				std::list<Unit*>::iterator It1;
				It1 = listMedics.begin();

				while(It1 != listMedics.end()){
					if(!(*It1)->exists()) It1 = listMedics.erase(It1);	
					else {
						if (!(*It1)->isLoaded()) {
							Graficos::resaltarUnidad(*It1, c);
							// manda a los medicos a curar a los soldados de su unidad
							Unit *u;
							if ((*It1)->isIdle() || (*It1)->isMoving()){
								// recorre la lista de marines y firebats buscando alguna unidad dañada
								u = buscarDañado(listMarines);

								if ((u != NULL)&& (u->exists())){
									(*It1)->rightClick(u);
								}
							}
						}

						It1++;
					}
				}
			}
		}
*/

	}

	// ------------------------ Ubica los tanques en modo asedio ------------------------
/*
	if (listaDeTanquesAUbicar.size() > 0){
		//Broodwar->printf("Entra a 6");
		if (latencia>100){			
			
			//revisar si hay que ubicar algun tanque en modo asedio
			std::list<Unit*>::iterator It1;
			It1 = listaDeTanquesAUbicar.begin();

			while(It1 != listaDeTanquesAUbicar.end()){
				if(!(*It1)->exists()) It1 = listaDeTanquesAUbicar.erase(It1);	
				else {
					if (!(*It1)->isMoving()) (*It1)->siege();
					It1++;
				}
			}
			latencia = 0;
			
		} else {latencia++;}
	}
	*/
	
	if ((comandante != NULL)&& (comandante->exists())&&(posicionanteriorDelComandante != comandante->getPosition()) ){
		posicionanteriorDelComandante = comandante->getPosition();
	}
	
	// ----------------------------------------------------------------------------------------

}



void compania::moverCompania(Position pos){

	if ((listMarines.size() > 0) && (comandante!=NULL) && (comandante->exists())){
		//comandante->attackMove(pos);
		comandante->rightClick(pos);

		std::list<Unit*>::iterator It1;
		It1 = listMarines.begin();

		while(It1 != listMarines.end()){
			/*if(!(*It1)->exists()) It1 = listMarines.erase(It1);	
			else {
				if ((*It1)->getDistance(comandante->getPosition())>2) (*It1)->rightClick(comandante->getPosition());
				It1++; 
			}*/

			if((*It1)->exists() && ((*It1)->getDistance(comandante->getPosition()) > 2))
				(*It1)->rightClick(comandante->getPosition());
				It1++;
		}
	}
	else{
		Broodwar->printf("el comandante no está, el comandante se fue, el comandante se escapa de mi vida");
	}
}



void compania::controlarDistancia(){
	
	std::list<Unit*> listaDeMarinesHeridos;
	Unit* herido;

	if ((listMarines.size() > 0) && (comandante!=NULL) && (comandante->exists())){
		std::list<Unit*>::iterator It1;
		It1 = listMarines.begin();

		while(It1 != listMarines.end()){
			if((*It1)->exists() && ((*It1)->getDistance(comandante->getPosition()) > 80) && ((*It1)->getID() != comandante->getID())) {
				(*It1)->rightClick(comandante->getPosition());

			}
			else{
				if ((*It1)->getID() != comandante->getID())
					(*It1)->stop();
				
				if (((*It1)->getType().maxHitPoints() > (*It1)->getHitPoints()) && (!(*It1)->isBeingHealed())){
					listaDeMarinesHeridos.push_front(*It1);
				}
			}
			It1++;
		}


		It1 = listMedics.begin();

		while(It1 != listMedics.end()){
			if((*It1)->exists() && ((*It1)->getDistance(comandante->getPosition()) > 110) && ((*It1)->getID() != comandante->getID())){
				(*It1)->rightClick(comandante->getPosition());
			}
			else{
				if ((*It1)->getID() != comandante->getID()){
					if ((*It1)->isMoving())
						(*It1)->stop();
					if(!listaDeMarinesHeridos.empty()) {
						herido = listaDeMarinesHeridos.front();
						listaDeMarinesHeridos.pop_front();
						if((herido!=NULL)&&(herido->exists())){
							(*It1)->rightClick(herido);
						}
					}
				}
			}
			It1++;
		}

		It1 = listFirebats.begin();

		while(It1 != listFirebats.end()){
			if((*It1)->exists() && ((*It1)->getDistance(comandante->getPosition()) > 70) && ((*It1)->getID() != comandante->getID())){
				(*It1)->rightClick(comandante->getPosition());
			}
			else{
				if ((*It1)->getID() != comandante->getID())
					(*It1)->stop();
			}
			It1++;
		}

		//muevo los tanques
		It1 = listTanks.begin();

		while(It1 != listTanks.end()){
			if((*It1)->exists() && ((*It1)->getDistance(comandante->getPosition()) > 140) && ((*It1)->getID() != comandante->getID())){
				if ((*It1)->isSieged()){
					(*It1)->unsiege();
				}

				(*It1)->rightClick(comandante->getPosition());
			}
			else{
				if ((*It1)->getID() != comandante->getID())
					(*It1)->stop();
					(*It1)->siege();
			}
			It1++;
		}
	}
	/*else{
		Broodwar->printf("el comandante no está, el comandante se fue, el comandante se escapa de mi vida");
	}*/
}

bool compania::pertenece(Unit *u){

	if (u->getType().getID() == Utilidades::ID_MARINE){
		std::list<Unit*>::iterator It1;
		It1 = listMarines.begin();

		while(It1 != listMarines.end()){
			if((*It1)->getID() == u->getID())
				return true;
			It1++;
		}
	}
	else if (u->getType().getID() == Utilidades::ID_MEDIC){
		std::list<Unit*>::iterator It1;
		It1 = listMedics.begin();

		while(It1 != listMedics.end()){
			if((*It1)->getID() == u->getID())
				return true;
			It1++;
		}
	}
	else if (u->getType().getID() == Utilidades::ID_FIREBAT){
		std::list<Unit*>::iterator It1;
		It1 = listFirebats.begin();

		while(It1 != listFirebats.end()){
			if((*It1)->getID() == u->getID())
				return true;
			It1++;
		}
	}

	return false;
}