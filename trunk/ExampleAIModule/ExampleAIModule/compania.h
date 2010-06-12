#pragma once
#include <BWAPI.h>

using namespace BWAPI;

class compania
{
public:
	compania(void);
	void asignarUnidad(Unit *U);
	void aplicarStim(); // aplica el stim_pack a las unidades pertenecientes a la compañia si no estan dentro de un contenedor (bunker, dropship)

	~compania(void);

private:
	void conteoUnidades(void);
	void ponerACubierto(Unit *U);
};

