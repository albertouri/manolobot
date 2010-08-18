#pragma once

#include <BWAPI.h>
#include <BWTA.h>
#include "Utilidades.h"
#include "compania.h"

using namespace BWAPI;
using namespace BWTA;

class CompaniaTransporte
{
public:
	CompaniaTransporte(Position* baseEnem, Region* regEnem, compania* c);
	~CompaniaTransporte(void);
	void onFrame();
	void asignarUnidad(Unit* u);

	// retorna true si la compañia de transporte esta lista a transportar las unidades
	bool listaTransportar();

	// ejecuta los movimientos hacia la base enemiga
	void ejecutarTransporte();

private:
	std::list<Unit*> listDropships;
	std::list<Unit*> listWraiths;
	std::list<Position*> pathBaseEnemiga;

	// puntero a la posicion del centro de comando enemigo
	Position *baseEnemiga;
	Region *regionBaseEnemiga;

	// puntero al punto de la region de la base enemiga mas lejano del chokepoint defendido
	Position *bordeMasLejano;

	Position *puntoDesembarco;
	void dibujarPath();
	void crearPath();

	void controlarEliminados();
	
	std::list<Position*>::iterator ItPosiciones;
	Unit *liderFormacion;

	// variable que una vez que se hace true evita realizar un control mejorando la eficiencia
	bool ready;

	// puntero a la compañia a transportar
	compania *aero;

	// es true si el comandante fue cargado en una nave de transporte
	bool comandanteCargado;

	bool ejecutandoTransporte;
};
