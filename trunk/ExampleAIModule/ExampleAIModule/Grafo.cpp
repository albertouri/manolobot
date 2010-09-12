#include "Grafo.h"

Grafo::Grafo(int cantNodos)
{
	// creo el arreglo dinamico con la cantidad de nodos que tendra el grafo
	listaAdy = new Nodo[cantNodos];
	
	std::set<Region*>::const_iterator It;
	It = BWTA::getRegions().begin();

	std::set<Chokepoint*>::const_iterator It2;
	Nodo *actual;

	int cont = 0;

	while (It != BWTA::getRegions().end()){
		listaAdy[cont].setRegion((*It));
		actual = &listaAdy[cont];

		// agrega los adyacentes a la region actual al grafo
		It2 = (*It)->getChokepoints().begin();
		while (It2 != (*It)->getChokepoints().end()){
			// si la primer region del chokepoint no es la region actual la agrega al grafo
			if ((*It2)->getRegions().first != (*It))
				actual->setSiguiente(new Nodo((*It2)->getRegions().first));
			else
				actual->setSiguiente(new Nodo((*It2)->getRegions().second));

			actual = actual->getSiguiente();
			It2++;
		}

		cont++;
		It++;
	}

	//Broodwar->printf("Grafo creado con exito");

	// crea la lista para visitar las regiones con un recorrido por niveles
	crearListaNiveles();

	//Broodwar->printf("El mapa tiene %d regiones", BWTA::getRegions().size());
	//Broodwar->printf("La lista niveles tiene %d elementos", niveles.size());


}

Grafo::~Grafo(void)
{
}


int Grafo::indiceRegion(Region *reg){
	int cont = 0;
	while (listaAdy[cont].getRegion() != reg)
		cont++;

	return cont;
}

void Grafo::crearListaNiveles(){
	//-- NUEVO
	std::list<Region*> nivelesRegiones;
	std::list<Region*>::iterator ItNivelesRegiones;
	//--

	std::set<Region*> visitados;

	//std::list<Region*>::iterator ItLista;
	//std::list<Position*>::iterator ItLista;

	std::set<Chokepoint*>::const_iterator ItChoke;

	//--
	// crea la lista para el recorrido por niveles
	Region *inicio = BWTA::getStartLocation(Broodwar->self())->getRegion();

	// inserta la region de inicio en la lista
	nivelesRegiones.push_back(inicio);
	niveles.push_back(new Position(inicio->getCenter().x(), inicio->getCenter().y()));
	
	visitados.insert(inicio);

	ItNivelesRegiones = nivelesRegiones.begin();
	while (ItNivelesRegiones != nivelesRegiones.end()){
		// itera sobre los chokepoints para agregar las regiones adyacentes a la lista
		ItChoke = (*ItNivelesRegiones)->getChokepoints().begin();

		while (ItChoke != (*ItNivelesRegiones)->getChokepoints().end()){
			if ((*ItChoke)->getRegions().first != (*ItNivelesRegiones)){
				
				if (visitados.find((*ItChoke)->getRegions().first) == visitados.end()){
					std::set<BaseLocation*>::const_iterator b = (*ItChoke)->getRegions().first->getBaseLocations().begin();
					
					nivelesRegiones.push_back((*ItChoke)->getRegions().first);

					while (b != (*ItChoke)->getRegions().first->getBaseLocations().end()){
						niveles.push_back(new Position((*b)->getPosition().x(), (*b)->getPosition().y()));
						b++;
					}

					visitados.insert((*ItChoke)->getRegions().first);
				}
			}
			else{
				if (visitados.find((*ItChoke)->getRegions().second) == visitados.end()){
					std::set<BaseLocation*>::const_iterator b = (*ItChoke)->getRegions().second->getBaseLocations().begin();

					nivelesRegiones.push_back((*ItChoke)->getRegions().second);

					while (b != (*ItChoke)->getRegions().second->getBaseLocations().end()){
						niveles.push_back(new Position((*b)->getPosition().x(), (*b)->getPosition().y()));
						b++;
					}

					visitados.insert((*ItChoke)->getRegions().second);
				}
			}

			ItChoke++;
		}

		ItNivelesRegiones++;
	}
}


Position* Grafo::primerNodoNiveles(){
	ItNiveles = niveles.begin();
	return (*ItNiveles);
}


Position* Grafo::siguienteNodoNiveles(){
	ItNiveles++;
	if (ItNiveles == niveles.end())
		return NULL;
	else
		return (*ItNiveles);
}

void Grafo::dibujarPuntosVisitar(){
	std::list<Position*>::iterator It = niveles.begin();

	while (It != niveles.end()){
		Broodwar->drawBoxMap((*It)->x(), (*It)->y(), (*It)->x() + 8, (*It)->y() + 8, Colors::White, true);
		It++;
	}
	//Broodwar->printf("Dibuje los puntos en el mapa");
}

/*void Grafo::dibujarRegionesNiveles(){
	std::list<Region*>::iterator It = niveles.begin();

	while (It != niveles.end()){
		Polygon p = (*It)->getPolygon();
		for(int j=0;j<(int)p.size();j++)
		{
			Position point1=p[j];
			Position point2=p[(j+1) % p.size()];
			Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Green);
		}

		It++;
	}


	// une con una linea los centros de las regiones a visitar
	if (niveles.size() > 1){
		std::list<Region*>::iterator It1, It2;

		It1 = niveles.begin();
		It2 = niveles.begin();
		It2++;

		while (It2 != niveles.end()){
			Broodwar->drawLineMap((*It1)->getCenter().x(), (*It1)->getCenter().y(), (*It2)->getCenter().x(),(*It2)->getCenter().y(), Colors::White);
			It1++;
			It2++;
		}
	}

}*/