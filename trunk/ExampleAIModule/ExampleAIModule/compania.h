#pragma once
#include <BWAPI.h>


using namespace BWAPI;

class compania
{
public:
	compania(void);
	void asignarUnidad(Unit *U);

	~compania(void);

private:
	void conteoUnidades(void);
};

