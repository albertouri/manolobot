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
	std::set<Region*> visitados;
	//std::list<std::pair<Region*, bool>>::iterator ItLista;
	std::list<Region*>::iterator ItLista;
	std::set<Chokepoint*>::const_iterator ItChoke;

	//std::pair<Region*, bool> par;

	//--
	// crea la lista para el recorrido por niveles
	Region *inicio = BWTA::getStartLocation(Broodwar->self())->getRegion();

	// inserta la region de inicio en la lista
	//par = std::make_pair(listaAdy[indiceRegion(inicio)].getRegion(), false);
	//niveles.push_back(par);
	niveles.push_back(inicio);
	visitados.insert(inicio);

	ItLista = niveles.begin();
	while (ItLista != niveles.end()){
		// itera sobre los chokepoints para agregar las regiones adyacentes a la lista
		//ItChoke = (*ItLista).first->getChokepoints().begin();
		ItChoke = (*ItLista)->getChokepoints().begin();

		//while (ItChoke != (*ItLista).first->getChokepoints().end()){
		while (ItChoke != (*ItLista)->getChokepoints().end()){
			if ((*ItChoke)->getRegions().first != (*ItLista)){
				//par = std::make_pair((*ItChoke)->getRegions().first, false);
				if (visitados.find((*ItChoke)->getRegions().first) == visitados.end()){
					niveles.push_back((*ItChoke)->getRegions().first);
					visitados.insert((*ItChoke)->getRegions().first);
				}
			}
			else{
				//par = std::make_pair((*ItChoke)->getRegions().second, false);
				if (visitados.find((*ItChoke)->getRegions().second) == visitados.end()){
					niveles.push_back((*ItChoke)->getRegions().second);
					visitados.insert((*ItChoke)->getRegions().second);
				}
			}

			ItChoke++;
		}

		ItLista++;
	}
}


Region* Grafo::primerNodoNiveles(){
	ItNiveles = niveles.begin();
	return (*ItNiveles);
}


Region* Grafo::siguienteNodoNiveles(){
	ItNiveles++;
	if (ItNiveles == niveles.end())
		return NULL;
	else
		return (*ItNiveles);
}

void Grafo::dibujarRegionesNiveles(){
	/*std::list<Region*>::iterator It = niveles.begin();

	while (It != niveles.end()){
		Polygon p = (*It)->getPolygon();
		for(int j=0;j<(int)p.size();j++)
		{
			Position point1=p[j];
			Position point2=p[(j+1) % p.size()];
			Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Green);
		}

		It++;
	}*/


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

}