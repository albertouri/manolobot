#pragma once

#include <BWAPI.h>
#include "Graficos.h"
#include "Utilidades.h"

using namespace BWAPI;

class CompaniaDefensiva
{
public:
	CompaniaDefensiva(Color ID);
	~CompaniaDefensiva(void);
	void onFrame();
	int countMarines(); // elimina de la lista correspondiente las unidades destruidas, y retorna la cantidad de unidades vivas
	void asignarUnidad(Unit *U);
	void atacar(Position p);

private:
	std::list<Unit*> listMarines;

	Color c; // color especifico para cada compañia, seteado en el constructor
};
