#pragma once

#include <BWAPI.h>

class Scout
{
public:
	Scout(BWAPI::Unit *unidad);
	~Scout(void);
	void explorar(void);
	void cambiarModo(int m);

private:
	void dibujarPosiciones(void);
	void dibujarGrilla(void);
};
