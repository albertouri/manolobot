#include "compania.h"
#include "Utilidades.h"
#include <math.h>

std::list<Unit*> lista;
int latencia=0;

int latenciaMovimientoTropas = 0;

bool atacando = false;
bool moviendoARegionContigua = false;


std::list<Unit*> listaDeTanquesAUbicar;

Unit* herido;
std::list<Unit*> listaDeUnidadesAfectadas;
std::list<Unit*> listaDeUnidadesNotMatrixed;


std::set<TilePosition> setPosicionesEdificiosEnemigos;

Region* regionActual = NULL;
Region* puntoDeRetirada = NULL;

compania::compania(AnalizadorTerreno *at, Color ID)
{
	c = ID;
	analizador = at;
	comandante = NULL;
	cantTransportes = 0;
	listRefuerzos.clear();
	esperar = false;

	setPosicionesEdificiosEnemigos.clear();
}

compania::~compania(void)
{
}

void compania::asignarUnidad(Unit *u){
	asignarARefuerzos(u);
	//asignarAPelotones(u);
}

void compania::asignarAPelotones(Unit *u){

	if ((u->getType().getID() == Utilidades::ID_TANKSIEGE) || (u->getType().getID() == Utilidades::ID_TANKSIEGE_SIEGEMODE)){
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

		//if ((comandante != NULL)&&(comandante->exists()))

	}

	if (regionActual != NULL)
		u->rightClick(regionActual->getCenter());

}

void compania::asignarARefuerzos(Unit *U){
	listRefuerzos.push_back(U);
	if ((listRefuerzos.size()>=5)||((puntoDeRetirada!=NULL)&&(analizador->analisisListo())&&(puntoDeRetirada == analizador->regionInicial()))){
		for(std::list<Unit*>::const_iterator i=listRefuerzos.begin();i!=listRefuerzos.end();i++){
			if ((*i)->exists())
				//Broodwar->printf("asigne un %d", (*i)->getType().getID());
				asignarAPelotones(*i);
		}
		listRefuerzos.clear();
	}
	else{
		if (puntoDeRetirada != NULL)
			U->rightClick(puntoDeRetirada->getCenter());
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
/*
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
*/
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


	listaDeUnidadesAfectadas.clear();
	listaDeUnidadesNotMatrixed.clear();


	//------------------------------controla las regiones iniciales----------------------------------------


	if(regionActual == NULL){
		if(analizador->analisisListo()){	
			regionActual = analizador->regionInicial();
			puntoDeRetirada = analizador->regionInicial();
		}
	}
	else{
		if((setPosicionesEdificiosEnemigos.size()>0) && (regionActual== analizador->regionInicial()) && (Broodwar->getFrameCount()%30 == 15)){
			Broodwar->printf("cambie a la region de al lado");
			TilePosition* centroRegionActual = new TilePosition(analizador->regionInicial()->getCenter());
			std::vector<BWAPI::TilePosition> vectorPosiciones = BWTA::getShortestPath(*centroRegionActual, setPosicionesEdificiosEnemigos);
			std::vector<BWAPI::TilePosition>::iterator It1;
			It1 = vectorPosiciones.begin();

			while((It1 != vectorPosiciones.end())&& (regionActual == analizador->regionInicial())){
				if (BWTA::getRegion(*It1)!=analizador->regionInicial()){
					regionActual = BWTA::getRegion(*It1);
				}
				It1++;
			}
		}
		
	}




	//----------------------------------------------------------------------------------------------------



	// ------------------------ realiza un recuadro a las unidades de la compañia ------------------------



	if ((comandante != NULL) && (comandante->exists())){
		Graficos::resaltarUnidad(comandante, c);
	//	Broodwar->printf("distancia %f  ----- rango %d", comandante->getDistance(analizador->regionInicial()->getCenter())/32 , comandante->getType().sightRange()/32);
	//	Broodwar->drawCircleMap(comandante->getPosition().x(),comandante->getPosition().y(), comandante->getType().sightRange(), Colors::White, false);
	}

	// ------------------------ verifica si el comandante esta seteado ------------------------
	setComandantes();
	

	// ------------------------  Ordenes de ataque ------------------------

	if ((comandante != NULL)&& (comandante->exists())&&(Broodwar->getFrameCount()%3 == 2)){
		// si el comandante no esta atacando ningun objetivo, se busca algun nuevo objetivo para atacar
		if ((comandante->getTarget() == NULL) || (!comandante->getTarget()->exists())){
			
//			if(listMarines.size() > 9){
			double minDist = comandante->getType().seekRange();
			Unit *masCercana = NULL;
				
			for(std::set<Unit*>::const_iterator i=Broodwar->enemy()->getUnits().begin();i!=Broodwar->enemy()->getUnits().end();i++){
				if ((*i)->exists()){
					if (comandante->getDistance((*i)->getPosition())/32 < minDist){
						minDist = comandante->getDistance((*i)->getPosition())/32 ;
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
				if(comandante->isSieged()) comandante->unsiege();
			}
//			}
		}

	}

	if (!atacando){
		if ((comandante!= NULL)&&(comandante->exists())){
			controlarDistancia(); 

		//codigo para decidir si marcha a combate
			if((esperar==false)&&(latenciaMovimientoTropas>50)&&(listaParaAtacar()) && (setPosicionesEdificiosEnemigos.size()>0) ){
				if (analizador->analisisListo()){
					std::vector<BWAPI::TilePosition> vectorPosiciones = BWTA::getShortestPath(comandante->getTilePosition(), setPosicionesEdificiosEnemigos);
					std::vector<BWAPI::TilePosition>::iterator It1;
					It1 = vectorPosiciones.begin();

					while(It1 != vectorPosiciones.end()){
						if (BWTA::getRegion(*It1)!=regionActual){
							puntoDeRetirada = regionActual;
							regionActual = BWTA::getRegion(*It1);
							break;
						}
						It1++;
						
					}

					if(BWTA::getRegion(comandante->getTilePosition()) != regionActual){
						comandante->rightClick(regionActual->getCenter());
					}

					latenciaMovimientoTropas = 0;
				}
			}
			else{
				latenciaMovimientoTropas++;
				if (regionActual!= NULL)
					if ((BWTA::getRegion(comandante->getTilePosition()) != regionActual)&&(Broodwar->getFrameCount()%50 == 27)){
						//Broodwar->printf("me ejecuto vieja");
						comandante->rightClick(regionActual->getCenter());
					}
					Graficos::dibujarCuadro(new TilePosition(regionActual->getCenter()), 2,2);
			}
		}
	}
	else{

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
	/*else{
		Broodwar->printf("el comandante no está, el comandante se fue, el comandante se escapa de mi vida");
	}*/
}



void compania::controlarDistancia(){
	std::list<Unit*> listaDeUnidadesAfectadas;
	std::list<Unit*> listaDeMarinesHeridos;
	Unit* herido;
	listaDeUnidadesAfectadas.clear();


	if ((comandante!= NULL) && (comandante->exists())){
		int	distanciaMarines = 80; 
		int	distanciaTanques = 160;
		int distanciaMedicos = 110;
		int distanciaGoliaths = 140;
		int distanciaNave = 200;
		std::list<Unit*>::iterator It1;
		int modulo12 = Broodwar->getFrameCount()%12;

		if (comandante->getType().getID() == Utilidades::ID_TANKSIEGE){
			distanciaMarines = 140;
			distanciaTanques = 80;
			distanciaMedicos = 160;
			distanciaGoliaths = 110; 
			distanciaNave = 200;
		}

		if ((listMarines.size() > 0) && (modulo12==0)){
			//std::list<Unit*>::iterator It1;
			It1 = listMarines.begin();

			while(It1 != listMarines.end()){
				if((*It1)->exists() && ((*It1)->getDistance(comandante->getPosition()) > distanciaMarines) && ((*It1)->getID() != comandante->getID())) {
					(*It1)->rightClick(comandante->getPosition());

				}
				else{
					if ((*It1)->getID() != comandante->getID())
						//(*It1)->stop();
						(*It1)->holdPosition();
					
					if (((*It1)->getType().maxHitPoints() > (*It1)->getHitPoints()) && (!(*It1)->isBeingHealed())){
						listaDeMarinesHeridos.push_front(*It1);
					}
				}
				It1++;
			}
		}

			//muevo los tanques
			if ((listTanks.size() > 0) && (modulo12==3)){
				//std::list<Unit*>::iterator It1;
				It1 = listTanks.begin();

				while(It1 != listTanks.end()){
					if((*It1)->exists() && ((*It1)->getDistance(comandante->getPosition()) > distanciaTanques) && ((*It1)->getID() != comandante->getID())){
						if ((*It1)->isSieged()){
							(*It1)->unsiege();
						}

						(*It1)->rightClick(comandante->getPosition());
					}
					else{
						if (((*It1)->getID() != comandante->getID()) /*codigo agregado por mi ->*/ && (!comandante->isLoaded())){
							(*It1)->holdPosition();
							if (atacando)
								(*It1)->siege();
							else if((*It1)->isSieged())
								(*It1)->unsiege();
						}
					}

					if (((*It1)->isLockedDown())||((*It1)->isParasited())||((*It1)->isEnsnared())||((*It1)->isBlind())||((*It1)->isPlagued()))
						listaDeUnidadesAfectadas.push_back(*It1);

					It1++;
				}
			}

			//muevo los Goliaths
			if ((listGoliath.size() > 0) && (modulo12==6)){
				It1 = listGoliath.begin();
				while(It1 != listGoliath.end()){
					if((*It1)->exists() && ((*It1)->getDistance(comandante->getPosition()) > distanciaGoliaths) && ((*It1)->getID() != comandante->getID())){
						(*It1)->rightClick(comandante->getPosition());
					}
					else{
						if ((*It1)->isMoving())
							(*It1)->holdPosition();				
					}
					if (((*It1)->isLockedDown())||((*It1)->isParasited())||((*It1)->isEnsnared())||((*It1)->isBlind())||((*It1)->isPlagued()))
						listaDeUnidadesAfectadas.push_back(*It1);
					It1++;
				}

			}
			
			//muevo las naves de ciencia
			if ((listScienceVessel.size() > 0) && (modulo12==9)){
				It1 = listScienceVessel.begin();

				while(It1 != listScienceVessel.end()){
					if((*It1)->exists() && ((*It1)->getDistance(comandante->getPosition()) > distanciaNave) && ((*It1)->getID() != comandante->getID())&& (Broodwar->getFrameCount()%12==9)){
						(*It1)->rightClick(comandante->getPosition());
					}
					else{
						if ((*It1)->isMoving())
							(*It1)->stop();				
					}
					It1++;
				}
			}

			if ((listMedics.size() > 0) && ((modulo12==0)||((modulo12==6)))){
				It1 = listMedics.begin();

				while(It1 != listMedics.end()){
					if((*It1)->exists() && ((*It1)->getDistance(comandante->getPosition()) > distanciaMedicos) && ((*It1)->getID() != comandante->getID())){
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
			}

			if ((listFirebats.size() > 0) && (modulo12==11)){
				It1 = listFirebats.begin();

				while(It1 != listFirebats.end()){
					if((*It1)->exists() && ((*It1)->getDistance(comandante->getPosition()) > distanciaMarines) && ((*It1)->getID() != comandante->getID())){
						(*It1)->rightClick(comandante->getPosition());
					}
					else{
						if ((*It1)->getID() != comandante->getID())
							(*It1)->stop();
					}
					It1++;
				}
			}
	
	}

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

	if((comandante!=NULL)&&(comandante->exists())){
		if ((listGoliath.size() > 2) && (listTanks.size() > 3) && (listScienceVessel.size() == 1) && (listMedics.size() > 4) && (listMarines.size() >= 10)){
			/*std::list<Unit*>::iterator It1;

			It1 = listMarines.begin();
			while(It1 != listMarines.end()){
				if((*It1)->exists()){
					if(floor((*It1)->getDistance(comandante))> comandante->getType().sightRange())
						return false;
				}
					
				It1++;
			}

			It1 = listTanks.begin();
			while(It1 != listTanks.end()){
				if((*It1)->exists()){
					if(floor((*It1)->getDistance(comandante))> comandante->getType().sightRange())
						return false;
				}
					
				It1++;
			}*/
			return true;
		}
		else
			return false;

	}
	else{
		return false;
	}

	
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





void compania::onEnemyBuildingShow(Unit* enemyB){
	setPosicionesEdificiosEnemigos.insert(enemyB->getTilePosition());
}


void compania::onEnemyBuildingDestroy(Unit* enemyB){
	setPosicionesEdificiosEnemigos.erase(enemyB->getTilePosition());
}


Unit* compania::getComandante(){
	return comandante;
}

void compania::setComandantes(void){
	Unit * comandanteanterior = comandante;
	
	// si no hay comandante, o murio, se asigna uno nuevo
	if ((comandante == NULL) || (!comandante->exists()) || (comandante->getType().getID()!=Utilidades::ID_TANKSIEGE)){
		
		actualizarEstado(&listTanks);
		if (listTanks.size() > 0){
			if((BWTA::getRegion((*listTanks.begin())->getTilePosition()) == regionActual)/*||(atacando== false)*/){
				comandante = *(listTanks.begin());
			}
		}
		//verifico que se haya seteado el comandante
		if ((comandante == NULL) || (!comandante->exists())){

			actualizarEstado(&listMarines);
			if (listMarines.size() > 0){
				if((BWTA::getRegion((*listMarines.begin())->getTilePosition()) == regionActual)||(atacando== false)){
					comandante = *(listMarines.begin());
				}
			}
			//verifico que se haya seteado el comandante
			if ((comandante == NULL) || (!comandante->exists())){
				retirada();
			}
			else{ //Hubo cambio de comandante
				if (comandanteanterior != comandante)
					if (comandante->getTarget()!=NULL)
						comandante->stop();
			}
		}
		else{ //Hubo cambio de comandante
			if (comandanteanterior != comandante)
				if (comandante->getTarget()!=NULL)
					comandante->stop();
		}




	}




	
}

void compania::retirada(){
	Region* ultimaBase = analizador->regionInicial();

	if(Broodwar->self()->completedUnitCount(Utilidades::ID_COMMANDCENTER)>=2){
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
			if (((*i)->getType().getID()== Utilidades::ID_COMMANDCENTER)&& (BWTA::getRegion((*i)->getPosition())!= analizador->regionInicial())){
				ultimaBase = BWTA::getRegion((*i)->getPosition());
				break;
			}	
		}
	}

	if (analizador->analisisListo() && (puntoDeRetirada!=ultimaBase)){
		TilePosition* posicionInicial = new TilePosition(ultimaBase->getCenter());
		TilePosition* posicionRetirada = new TilePosition(puntoDeRetirada->getCenter());
		std::vector<BWAPI::TilePosition> vectorPosiciones = BWTA::getShortestPath(*posicionRetirada, *posicionInicial);
		std::vector<BWAPI::TilePosition>::iterator It1;
		It1 = vectorPosiciones.begin();
		while(It1 != vectorPosiciones.end()){
			if (BWTA::getRegion(*It1)!=puntoDeRetirada){
				regionActual = puntoDeRetirada;
				puntoDeRetirada = BWTA::getRegion(*It1);
				break;
			}
			It1++;
		}
		
		if(listMarines.size()>0){
			for(std::list<Unit*>::const_iterator i=listMarines.begin();i!=listMarines.end();i++){
				if ((*i)->exists())
					(*i)->rightClick(regionActual->getCenter());
			}
		}

		if(listMedics.size()>0){
			for(std::list<Unit*>::const_iterator i=listMedics.begin();i!=listMedics.end();i++){
				if ((*i)->exists())
					(*i)->rightClick(regionActual->getCenter());
			}
		}
		
		if(listTanks.size()>0){
			for(std::list<Unit*>::const_iterator i=listTanks.begin();i!=listTanks.end();i++){
				if ((*i)->exists())
					if ((*i)->isSieged())
						(*i)->unsiege();
					(*i)->rightClick(regionActual->getCenter());
			}
		}

		if(listGoliath.size()>0){
			for(std::list<Unit*>::const_iterator i=listGoliath.begin();i!=listGoliath.end();i++){
				if ((*i)->exists())
					(*i)->rightClick(regionActual->getCenter());
			}
		}

		if(listScienceVessel.size()>0){
			for(std::list<Unit*>::const_iterator i=listScienceVessel.begin();i!=listScienceVessel.end();i++){
				if ((*i)->exists())
					(*i)->rightClick(regionActual->getCenter());
			}
		}


		if(listRefuerzos.size()>0){
			for(std::list<Unit*>::const_iterator i=listRefuerzos.begin();i!=listRefuerzos.end();i++){
				if ((*i)->exists())
					asignarAPelotones(*i);
			}
			listRefuerzos.clear();
		}


	}
}

void compania::setComportanmientoEsperando(){
	esperar = true;
}
	
void compania::setComportanmientoNormal(){
	esperar = false;
}

bool compania::faltanMarines(){
	return (listMarines.size()< 6);
}


bool compania::faltanMedicos(){
	return (listMedics.size()<4);
}

bool compania::faltanGoliaths(){
	return (listGoliath.size()<2);
}

bool compania::faltanTanks(){
	return (listTanks.size()<3);
}