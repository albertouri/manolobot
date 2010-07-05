#pragma once

#include <BWAPI.h>
using namespace BWAPI;

class Utilidades
{
public:
	Utilidades(void);
	~Utilidades(void);


	// Indice de cada unidad en el arreglo IdUnidades (utilizado para setear las goals)
	static const int INDEX_GOAL_ACADEMY = 0;
	static const int INDEX_GOAL_BARRACK = 2;
	static const int INDEX_GOAL_BUNKER = 4;
	static const int INDEX_GOAL_COMMANDCENTER = 5;
	static const int INDEX_GOAL_FACTORY = 11;
	static const int INDEX_GOAL_FIREBAT = 12;
	static const int INDEX_GOAL_MACHINESHOP = 15;
	static const int INDEX_GOAL_MARINE = 16;
	static const int INDEX_GOAL_MEDIC = 17;
	static const int INDEX_GOAL_REFINERY = 22;
	static const int INDEX_GOAL_SCV = 23;
	static const int INDEX_GOAL_TANKSIEGE = 27;
	static const int INDEX_GOAL_DEPOT = 29;
	
	// constantes que representan el ID de cada tipo de unidad terran
	static const int ID_MARINE = 0;
	static const int ID_SCV = 7;
	static const int ID_TANKSIEGE = 5;
	static const int ID_FIREBAT = 32;
	static const int ID_MEDIC = 34;
	static const int ID_COMMANDCENTER = 106;
	static const int ID_DEPOT = 109;
	static const int ID_REFINERY = 110;
	static const int ID_BARRACK = 111;
	static const int ID_ACADEMY = 112;
	static const int ID_FACTORY = 113;
	static const int ID_MACHINESHOP = 120;
	static const int ID_BUNKER = 125;

	// indice de cada investigacion en el arreglo goalResearch
	static const int INDEX_GOAL_STIMPACK = 0;
	static const int INDEX_GOAL_U238 = 1;
	static const int INDEX_GOAL_TANK_SIEGE_MODE = 2;

	// cantidad de investigaciones que provee el juego
	static const int maxResearch = 10;

};
