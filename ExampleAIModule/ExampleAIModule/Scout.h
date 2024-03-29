#pragma once

#include <BWAPI.h>
#include <BWTA.h>
#include "Grafo.h"
#include "Nodo.h"

using namespace BWAPI;
using namespace BWTA;

class Scout
{
public:
	Scout(Unit *unidad);
	Scout(Unit *unidad, Grafo *g);

	~Scout(void);
	//void asignarNuevoScout(Unit* nuevoScout);
	bool exists(void);
	void explorar(void);
	void cambiarModo(int m);
	void setExplorador(Unit *u);
	Unit* getExplorador();


private:
	Position* posActual; //-- nuevo

	Grafo *grafo;
	int tiempoMax;
	bool primeraExploracion;
	
	

	void dibujarPosiciones(void);
	void dibujarGrilla(void);

	//Region *regActual;
};
