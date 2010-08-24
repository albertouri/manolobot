#include "compania.h"
#include "Utilidades.h"

std::list<Unit*> lista;
int latencia=0;

bool atacando = false;
Position posicionanteriorDelComandante;

std::list<Unit*> listaDeTanquesAUbicar;

compania::compania(AnalizadorTerreno *at, Color ID)
{
	c = ID;
	analizador = at;
	comandante = NULL;
	cantTransportes = 0;
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

		calcularTransportes();
	}
	else{
		if (u->getType().getID() == Utilidades::ID_MARINE){
			listMarines.push_back(u);
			calcularTransportes();
		}
		else if (u->getType().getID() == Utilidades::ID_MEDIC){
			listMedics.push_back(u);
			calcularTransportes();
		}
		else if (u->getType().getID() == Utilidades::ID_FIREBAT){
			listFirebats.push_back(u);
			calcularTransportes();
		}
		else if (u->getType().getID() == Utilidades::ID_GOLIATH){
			listGoliath.push_back(u);
			calcularTransportes();
		}
		else if (u->getType().getID() == Utilidades::ID_SCIENCE_VESSEL){
			listScienceVessel.push_back(u);
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

int compania::countTanks(){
	return contarUnidades(&listTanks);
}

int compania::countGoliaths(){
	return contarUnidades(&listGoliath);
}

int compania::countScienceVessels(){
	return contarUnidades(&listScienceVessel);
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
				if ((!(*It1)->isSieged()) /* codigo agregado por mi */ && (!comandante->isLoaded())){
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
	Unit* herido;
	std::list<Unit*> listaDeUnidadesAfectadas;
	std::list<Unit*> listaDeUnidadesNotMatrixed;

	listaDeUnidadesAfectadas.clear();
	listaDeUnidadesNotMatrixed.clear();


	// ------------------------ realiza un recuadro a las unidades de la compañia ------------------------



	if ((comandante != NULL) && (comandante->exists())){
		Graficos::resaltarUnidad(comandante, c);
		//Graficos::dibujarCirculo(*(new TilePosition((*It1)->getTilePosition().x() + 1, (*It1)->getTilePosition().y() + 1)), 1, 1);
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
		// si el comandante no esta atacando ningun objetivo, se busca algun nuevo objetivo para atacar
		if ((comandante->getTarget() == NULL) || (!comandante->getTarget()->exists())){
			
//			if(listMarines.size() > 9){
				double minDist = 90;
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
			controlarDistancia(); 
	}
	else{
// de aca 
		if (listGoliath.size() > 0){
			std::list<Unit*>::iterator It1;
			It1 = listGoliath.begin();

			while(It1 != listGoliath.end()){
				if(!(*It1)->exists()) {
					It1 = listGoliath.erase(It1);	
				}
				else {
					if (((*It1)->isLockedDown())||((*It1)->isParasited())||((*It1)->isEnsnared())||((*It1)->isBlind())||((*It1)->isPlagued()))
						listaDeUnidadesAfectadas.push_back(*It1);
					
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
					if (((*It1)->isLockedDown())||((*It1)->isParasited())||((*It1)->isEnsnared())||((*It1)->isBlind())||((*It1)->isPlagued()))
						listaDeUnidadesAfectadas.push_back(*It1);
					if(!(*It1)->isDefenseMatrixed())
						listaDeUnidadesNotMatrixed.push_back(*It1);
					Graficos::resaltarUnidad(*It1, c);
					It1++; 
				}
			}
		}



// hasta aca agregué
		std::list<Unit*>::iterator It1;	
		It1 = listMedics.begin();

		while((It1 != listMedics.end())&&(!listaDeUnidadesAfectadas.empty())){
			if ((*It1)->getEnergy()>50){
				herido = listaDeUnidadesAfectadas.front();
				listaDeUnidadesAfectadas.pop_front();
				if((herido!=NULL)&&(herido->exists())){
					(*It1)->useTech(TechTypes::Restoration, herido);
				}
			}
			It1++;
		}

		It1 = listScienceVessel.begin();

		while((It1 != listScienceVessel.end())&&(!listaDeUnidadesNotMatrixed.empty())){
			if ((*It1)->getEnergy()>100){
				herido = listaDeUnidadesNotMatrixed.front();
				listaDeUnidadesNotMatrixed.pop_front();
				if((herido!=NULL)&&(herido->exists())){
					(*It1)->useTech(TechTypes::Defensive_Matrix, herido);
				}
			}
			It1++;
		}
	}
	
	if ((comandante != NULL)&& (comandante->exists())&&(posicionanteriorDelComandante != comandante->getPosition()) ){
		posicionanteriorDelComandante = comandante->getPosition();
	}
	
	// ----------------------------------------------------------------------------------------
	//-- controla las unidades eliminando las unidades muertas de la lista correspondiente
	if (Broodwar->getFrameCount() % 200 == 0){
		countMarines();
	}
	else if(Broodwar->getFrameCount() % 200 == 10){
		countMedics();
	}
	else if(Broodwar->getFrameCount() % 200 == 20){
		countTanks();
	}
	else if(Broodwar->getFrameCount() % 200 == 30){
		countGoliaths();
	}
	else if(Broodwar->getFrameCount() % 200 == 40){
		countScienceVessels();
	}
	else if(Broodwar->getFrameCount() % 200 == 50){
		countFirebats();
	}
	
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
	std::list<Unit*> listaDeUnidadesAfectadas;
	std::list<Unit*> listaDeMarinesHeridos;
	Unit* herido;
	listaDeUnidadesAfectadas.clear();

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


		//muevo los tanques
		It1 = listTanks.begin();

		while(It1 != listTanks.end()){
			if((*It1)->exists() && ((*It1)->getDistance(comandante->getPosition()) > 160) && ((*It1)->getID() != comandante->getID())){
				if ((*It1)->isSieged()){
					(*It1)->unsiege();
				}

				(*It1)->rightClick(comandante->getPosition());
			}
			else{
				if (((*It1)->getID() != comandante->getID()) /*codigo agregado por mi ->*/ && (!comandante->isLoaded()))
					(*It1)->stop();
					(*It1)->siege();
			}

			if (((*It1)->isLockedDown())||((*It1)->isParasited())||((*It1)->isEnsnared())||((*It1)->isBlind())||((*It1)->isPlagued()))
				listaDeUnidadesAfectadas.push_back(*It1);

			It1++;
		}

		//muevo los Goliaths
		It1 = listGoliath.begin();

		while(It1 != listGoliath.end()){
			if((*It1)->exists() && ((*It1)->getDistance(comandante->getPosition()) > 140) && ((*It1)->getID() != comandante->getID())){
				(*It1)->rightClick(comandante->getPosition());
			}
			else{
				if ((*It1)->isMoving())
					(*It1)->stop();				
			}
			if (((*It1)->isLockedDown())||((*It1)->isParasited())||((*It1)->isEnsnared())||((*It1)->isBlind())||((*It1)->isPlagued()))
				listaDeUnidadesAfectadas.push_back(*It1);
			It1++;
		}


		//muevo las naves de ciencia
		It1 = listScienceVessel.begin();

		while(It1 != listScienceVessel.end()){
			if((*It1)->exists() && ((*It1)->getDistance(comandante->getPosition()) > 200) && ((*It1)->getID() != comandante->getID())){
				(*It1)->rightClick(comandante->getPosition());
			}
			else{
				if ((*It1)->isMoving())
					(*It1)->stop();				
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

					if ((!listaDeUnidadesAfectadas.empty()) && ((*It1)->getEnergy()>50)){
						herido = listaDeUnidadesAfectadas.front();
						listaDeUnidadesAfectadas.pop_front();
						if((herido!=NULL)&&(herido->exists())){
							(*It1)->useTech(TechTypes::Restoration, herido);
						}
					}
					
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


bool compania::listaParaAtacar(){

	if ((listGoliath.size() > 2) && /*(listTanks.size() > 1) &&*/ (listScienceVessel.size() == 1) && (listMedics.size() > 4) && (listMarines.size() >= 12))
		return true;
	else
		return false;
}


int compania::cantidadTransportes(){
	return cantTransportes;
}


void compania::calcularTransportes(){
	cantTransportes = (listMarines.size() + listMedics.size() + listFirebats.size() + 2 * listGoliath.size() /*+ 4 * listTanks.size()*/) / 8;

	// si alguna unidad no alcanza a entrar en el transporte se necesita un transporte mas
	if (((listMarines.size() + listMedics.size() + listFirebats.size() + 2 * listGoliath.size() /*+ 4 * listTanks.size()*/) % 8) > 0)
		cantTransportes++;
}


void compania::abordarTransporte(std::list<Unit*> *transportes){
	std::list<Unit*>::iterator ItTransportes;
	std::list<Unit*>::iterator ItUnidades;

	//Broodwar->printf("Intenta abordar");
	ItTransportes = transportes->begin();
	ItUnidades = listMarines.begin();

	if (!transportes->empty()){

		//-- carga los marines
		while (ItUnidades != listMarines.end()){
			if (((*ItUnidades)->exists()) && (!(*ItUnidades)->isLoaded())){
				ItTransportes = transportes->begin();
				while (ItTransportes != transportes->end()){
					if (((*ItTransportes)->exists()) && ((*ItTransportes)->isIdle()))
						(*ItTransportes)->load(*ItUnidades);

					ItTransportes++;
				}
			}
			ItUnidades++;
		}

		//-- carga los medicos
		ItUnidades = listMedics.begin();
		while (ItUnidades != listMedics.end()){
			if (((*ItUnidades)->exists()) && (!(*ItUnidades)->isLoaded())){
				ItTransportes = transportes->begin();
				while (ItTransportes != transportes->end()){
					if (((*ItTransportes)->exists()) && ((*ItTransportes)->isIdle()))
						(*ItTransportes)->load(*ItUnidades);

					ItTransportes++;
				}
			}

			ItUnidades++;
		}

		//-- carga los firebats
		ItUnidades = listFirebats.begin();
		while (ItUnidades != listFirebats.end()){
			if (((*ItUnidades)->exists()) && (!(*ItUnidades)->isLoaded())){
				ItTransportes = transportes->begin();
				while (ItTransportes != transportes->end()){
					if (((*ItTransportes)->exists()) && ((*ItTransportes)->isIdle()))
						(*ItTransportes)->load(*ItUnidades);

					ItTransportes++;
				}
			}

			ItUnidades++;
		}

		//-- carga los goliaths
		ItUnidades = listGoliath.begin();
		while (ItUnidades != listGoliath.end()){
			if (((*ItUnidades)->exists()) && (!(*ItUnidades)->isLoaded())){
				ItTransportes = transportes->begin();
				while (ItTransportes != transportes->end()){
					if (((*ItTransportes)->exists()) && ((*ItTransportes)->isIdle()))
						(*ItTransportes)->load(*ItUnidades);

					ItTransportes++;
				}
			}

			ItUnidades++;
		}

		/*
		//-- carga los tanques
		ItUnidades = listTanks.begin();
		while (ItUnidades != listTanks.end()){
			if (((*ItUnidades)->exists()) && (!(*ItUnidades)->isLoaded())){
				ItTransportes = transportes->begin();
				while (ItTransportes != transportes->end()){
					if (((*ItTransportes)->exists()) && ((*ItTransportes)->isIdle()))
						(*ItTransportes)->load(*ItUnidades);

					ItTransportes++;
				}
			}

			ItUnidades++;
		}*/
	}
}


bool compania::companiaAbordo(){
	std::list<Unit*>::iterator It;

	It = listMarines.begin();
	while (It != listMarines.end()){
		if (!(*It)->isLoaded())
			return false;
		It++;
	}
	
	It = listFirebats.begin();
	while (It != listFirebats.end()){
		if (!(*It)->isLoaded())
			return false;
		It++;
	}

	It = listMedics.begin();
	while (It != listMedics.end()){
		if (!(*It)->isLoaded())
			return false;
		It++;
	}

	It = listGoliath.begin();
	while (It != listGoliath.end()){
		if (!(*It)->isLoaded())
			return false;
		It++;
	}

	/*It = listTanks.begin();
	while (It != listTanks.end()){
		if (!(*It)->isLoaded())
			return false;
		It++;
	}*/

	return true;
}


Unit* compania::getComandante(){
	return comandante;
}