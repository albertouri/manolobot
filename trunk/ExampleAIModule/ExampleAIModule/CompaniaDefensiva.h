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

	// Metodo que se ejecuta en cada frame del juego para realizar distintas tareas
	void onFrame();

	// Elimina de la lista correspondiente las unidades destruidas, y retorna la cantidad de unidades vivas de la compañia defensiva
	int countMarines(); 

	// Agrega la unidad pasada como parametro a la compañia defensiva
	void asignarUnidad(Unit *U);

	// Ordena a los marines de la compañía atacar la unidad pasada como parámetro
	void atacar(Unit *u);
	
	// Realiza un movimiento de ataque hacia la posicion pasada como parametro
	void atacar(Position p);

	// Retorna true si en la compañia defensiva hay menos marines de los necesarios
	bool faltanMarines();
	
	// Retorna true si en la compañia defensiva hay menos ghosts de los necesarios
	bool faltanGhosts();
	
	// Retorna true si en la compañia defensiva hay menos medicos de los necesarios
	bool faltanMedics();

private:
	// Listas de unidades de la compañia defensiva
	std::list<Unit*> listMarines;
	std::list<Unit*> listGhosts;
	std::list<Unit*> listMedics;

	// Color especifico para resaltar los soldados de cada compañia, seteado en el constructor
	Color c;

	// Elimina de la lista correspondiente las unidades que no existan mas
	void controlarEliminados();

	// Busqueda de objetivos para cada tipo de unidad de la compañia defensiva
	Unit* buscarObjetivosGhost();
	Unit* buscarObjetivosMedics();
	Unit* buscarObjetivosMarines();
	
	// Busca objetivos para cada tipo de unidad y los ataca
	void defenderBaseGhosts();
	void defenderBaseMedics();
	void defenderBaseMarines();

	// Resalta las unidades de la compañia defensiva
	void recuadrarUnidades();
};
