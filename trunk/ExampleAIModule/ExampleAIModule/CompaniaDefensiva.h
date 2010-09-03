#pragma once

#include <BWAPI.h>
#include <BWTA.h>
#include "Graficos.h"
#include "Utilidades.h"

using namespace BWAPI;
using namespace BWTA;

class CompaniaDefensiva
{
public:
	CompaniaDefensiva(Color ID);
	~CompaniaDefensiva(void);

	// metodo que se ejecuta en cada frame del juego
	void onFrame();

	// elimina de la lista correspondiente las unidades destruidas, y retorna la cantidad de unidades vivas
	int countMarines(); 

	// agrega una unidad a la compañia defensiva
	void asignarUnidad(Unit *U);

	// realiza un movimiento de ataque hacia la posicion pasada como parametro
	void atacar(Unit *u);
	void atacar(Position p);

	bool faltanMarines();
	bool faltanGhosts();
	bool faltanMedics();

private:
	// lista de marines de la compañia defensiva
	std::list<Unit*> listMarines;
	std::list<Unit*> listGhosts;
	std::list<Unit*> listMedics;

	// elimina de la lista correspondiente las unidades que no existan mas
	void controlarEliminados();

	// color especifico para resaltar los soldados de cada compañia, seteado en el constructor
	Color c;

	Unit* buscarObjetivosGhost();
	Unit* buscarObjetivosMedics();

	void defenderBaseGhosts();
	void defenderBaseMedics();

};
