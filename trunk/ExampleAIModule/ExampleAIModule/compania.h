#pragma once
#include <BWAPI.h>
#include "Graficos.h"

using namespace BWAPI;

class compania
{
public:
	compania(BWAPI::Color);
	void asignarUnidad(Unit *U);
	void aplicarStim(std::list<Unit*> lista); // aplica el stim_pack a las unidades pertenecientes a la compañia si no estan dentro de un contenedor (bunker, dropship)

	int countMarines(); // elimina de la lista correspondiente las unidades destruidas, y retorna la cantidad de unidades vivas
	int countMedics(); // elimina de la lista correspondiente las unidades destruidas, y retorna la cantidad de unidades vivas
	int countFirebats(); // elimina de la lista correspondiente las unidades destruidas, y retorna la cantidad de unidades vivas

	void atacar(Unit *u); // manda a la compañia a atacar a la unidad pasada como parametro
	void onFrame();

	~compania(void);

private:
	void conteoUnidades(void);
	void ponerACubierto(Unit *U);
	int contarUnidades(std::list<Unit*> *lista);

	// listas de unidades de la compañia
	std::list<Unit*> listMarines;
	std::list<Unit*> listMedics;
	std::list<Unit*> listFirebats;

};

