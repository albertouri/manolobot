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

	Region* primerNodoNiveles();
	Region* siguienteNodoNiveles();
	
	void dibujarRegionesNiveles();

private:
	// estructura de datos para almacenar el grafo (lista de adyacencia)
	Nodo* listaAdy;
	
	// puntero para el recorrido por niveles, apunta al nodo que se esta recorriendo actualmente
	//std::list<std::pair<Region*, bool>> niveles;
	//std::list<std::pair<Region*, bool>>::iterator ItNiveles;
	std::list<Region*> niveles;
	std::list<Region*>::iterator ItNiveles;

	// devuelve el indice de la region pasada como parametro en la lista de adyacencia
	int indiceRegion(Region *reg);

	void crearListaNiveles();
};
