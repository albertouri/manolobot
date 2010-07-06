#include "GrupoBunkers.h"
#include <list>
std::list<int> posicionesLibres;

/*GrupoBunkers::GrupoBunkers(void)
{
}*/
GrupoBunkers::GrupoBunkers(AnalizadorTerreno *a)
{
	analizador = a;
	choke = a->obtenerChokepoint();
	reg = a->regionInicial();

	//primerBunker = posicionPrimerBunker();
}

GrupoBunkers::~GrupoBunkers(void)
{
	delete choke;
	delete reg;
}

void GrupoBunkers::agregarUnidad(Unit* u){
	if (u != NULL){
		if (u->getType().getID() == Utilidades::ID_BUNKER)
			bunkers.push_front(u);
		else if (u->getType().getID() == Utilidades::ID_MISSILE_TURRET)
			misileTurrets.push_front(u);
		else
			Broodwar->printf("No se puede agregar ese tipo de unidad a un grupo de bunkers");
	}
}

Unit* GrupoBunkers::getUltimoBunkerCreado(){
	if (bunkers.size() > 0)
		return bunkers.front();
	else 
		return NULL;
}

Unit* GrupoBunkers::getPrimerBunkerCreado(){
	if (bunkers.size() > 0)
		return bunkers.back();
	else
		return NULL;
}


int GrupoBunkers::getCantBunkers(){

	std::list<Unit*>::iterator It1;
	It1 = bunkers.begin();
	int cont = 0;

	if (bunkers.size() > 0){
		while (It1 != bunkers.end()){
			if ((*It1)->exists())
				cont++;
			
			It1++;
		}
	}

	return cont;
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

/*TilePosition* GrupoBunkers::posicionNuevoBunker(){
	if (!analizador->analisisListo())
		return NULL;

	if (getCantBunkers() == 0){
		t = analizador->calcularPrimerTile(reg, choke);
	}
	else{
		int angulo;
		Position *p1, *p2;

		// Inicializo los puntos que representan a los bordes del chokepoint
		// p1 siempre sera el borde mas a la izquierda del chokepoint
		// en caso de que tengan la misma coordenada X (el chokepoint es vertical), p1 sera el punto que tenga la menor coordenada Y
		if (choke->getSides().first.x() != choke->getSides().second.x()){
			// la inclinacion del chokepoint no es completamente vertical |, es decir el chokepoint esta inclinado	
			if (choke->getSides().first.x() < choke->getSides().second.x()){
				p1 = new Position(choke->getSides().first.x(), choke->getSides().first.y());
				p2 = new Position(choke->getSides().second.x(), choke->getSides().second.y());
			}
			else{
				p2 = new Position(choke->getSides().first.x(), choke->getSides().first.y());
				p1 = new Position(choke->getSides().second.x(), choke->getSides().second.y());
			}
		}
		else{
			// la inclinacion del chokepoint es vertical |
			if (choke->getSides().first.y() < choke->getSides().second.y()){
				p1 = new Position(choke->getSides().first.x(), choke->getSides().first.y());
				p2 = new Position(choke->getSides().second.x(), choke->getSides().second.y());
			}
			else{
				p2 = new Position(choke->getSides().first.x(), choke->getSides().first.y());
				p1 = new Position(choke->getSides().second.x(), choke->getSides().second.y());
			}
		}

		angulo = analizador->calcularAngulo(p1, p2);

		Unit *bunker;
		TilePosition *t = NULL;

		// Dependiendo el angulo determina una posicion
		// TODO: mejorar la heuristica
		bunker = getPrimerBunkerCreado();

		if ((angulo >= 67) && (angulo < 112)){
			if (getCantBunkers() % 2 == 0)
				t = new TilePosition(bunker->getTilePosition().x() + 3, bunker->getTilePosition().y());
			else
				t = new TilePosition(bunker->getTilePosition().x() - 3, bunker->getTilePosition().y());
		}
		else if ((angulo >= 112) && (angulo < 157)){
			if (getCantBunkers() % 2 == 0)
				t = new TilePosition(bunker->getTilePosition().x() + 1, bunker->getTilePosition().y() - 2);
			else
				t = new TilePosition(bunker->getTilePosition().x() - 1, bunker->getTilePosition().y() + 2);
		}
		else if ((angulo < 67) && (angulo > 22)){
			if (getCantBunkers() % 2 == 0)
				t = new TilePosition(bunker->getTilePosition().x() - 1, bunker->getTilePosition().y() - 2);
			else
				t = new TilePosition(bunker->getTilePosition().x() + 1, bunker->getTilePosition().y() + 2);			
		}
		else if ((angulo <= 22) || (angulo >= 157)){
			if (getCantBunkers() % 2 == 0)
				t = new TilePosition(bunker->getTilePosition().x(), bunker->getTilePosition().y() - 2);
			else
				t = new TilePosition(bunker->getTilePosition().x(), bunker->getTilePosition().y() + 2);			
		}

		return t;
	}
}*/


TilePosition* GrupoBunkers::posicionPrimerBunker(){
	if (analizador->analisisListo())
		return analizador->calcularPrimerTile(reg, choke, 1);
	else
		return NULL;
}


TilePosition* GrupoBunkers::posicionNuevoBunker(){
	if (!analizador->analisisListo())
		return NULL;
	
	return analizador->calcularPrimerTile(reg, choke, getCantBunkers() + 1);

}


TilePosition* GrupoBunkers::ubicarGrupo(){

}


TilePosition* GrupoBunkers::posicionNuevaTorreta(){
	
}



void GrupoBunkers::controlDestruidos(){
	std::list<Unit*>::iterator It1;
	It1 = bunkers.begin();
	int cont = 1;

	while (It1 != bunkers.end()){
		if (!(*It1)->exists()){
			bunkers.erase(It1);
			It1 = bunkers.begin(); // tuve que poner esto porque sino se colgaba el while...

			//posicionesLibres.push_front(cont);
			//posicionesLibres.unique(); // elimina posiciones repetidas
		}
		else
			It1++;

		cont++;
	}
}


void GrupoBunkers::onFrame(){
	if (Broodwar->getFrameCount() % frameLatency == 0){

		// Calcula la posicion del primer bunker a construir
		if (primerBunker = NULL)
			posicionPrimerBunker();

		if (getCantBunkers() != bunkers.size()) // es decir hay algun bunker que ya no existe en la lista, se debe actualizar
			controlDestruidos();
	}
}