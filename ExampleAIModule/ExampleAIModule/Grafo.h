#pragma once

#include "Nodo.h"

using namespace BWAPI;
using namespace BWTA;

class Grafo
{
public:
	Grafo(int cantNodos);
	~Grafo(void);

	void agregarNodo(Nodo *n);

	/*Region* primerNodoNiveles();
	Region* siguienteNodoNiveles();*/
	Position* primerNodoNiveles();
	Position* siguienteNodoNiveles();

	void dibujarPuntosVisitar();
	
	//void dibujarRegionesNiveles();

private:
	// estructura de datos para almacenar el grafo (lista de adyacencia)
	Nodo* listaAdy;
	
	// puntero para el recorrido por niveles, apunta al nodo que se esta recorriendo actualmente
	//std::listRegiones<Region*> nivelesRegiones;
	std::list<Position*> niveles;

	//std::list<Region*>::iterator ItNivelesRegiones;
	std::list<Position*>::iterator ItNiveles;

	// devuelve el indice de la region pasada como parametro en la lista de adyacencia
	int indiceRegion(Region *reg);

	void crearListaNiveles();
};
