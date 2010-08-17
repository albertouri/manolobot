#pragma once

#include <BWAPI.h>
#include <BWTA.h>
#include "Utilidades.h"

using namespace BWAPI;
using namespace BWTA;

class CompaniaTransporte
{
public:
	CompaniaTransporte(Position* baseEnem, Region* regEnem);
	~CompaniaTransporte(void);
	void onFrame();
	void asignarUnidad(Unit* u);
	bool listaTransportar();

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
	
};
