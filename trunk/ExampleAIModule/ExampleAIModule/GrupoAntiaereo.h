#pragma once

#include <BWAPI.h>
#include <BWTA.h>

#include "Utilidades.h"
#include "AnalizadorTerreno.h"
#include "Graficos.h"

using namespace BWAPI;
using namespace BWTA;

class GrupoAntiaereo
{
public:
	// la region pasada como parametro es la region a defender
	GrupoAntiaereo(Region *r);

	~GrupoAntiaereo(void);
	void onFrame();
	bool faltanMisileTurrets();
	void agregarUnidad(Unit *u);

	// devuelve el tilePosition donde debera ubicarse el proximo misileTurret (borrar el objeto retornado por este metodo despues de utilizarlo)
	TilePosition* getPosicionMisileTurret();

	int cantMaximaTurrets();

	void onUnitShow(Unit *u);

private:
	int cantidadNecesariaTurrets;
	bool puedoConstruir(TilePosition t, UnitType tipo);
	TilePosition *encontrarPosicion(int orientacion, Position posCentral);

	Unit *objetivoActual;

	int getCantMisileTurrets();

	static const int IZQUIERDA = 0;
	static const int DERECHA = 1;
	static const int ARRIBA = 2;
	static const int ABAJO = 3;

	std::list<std::pair<TilePosition*, Unit*>> listMisileTurrets;
};
