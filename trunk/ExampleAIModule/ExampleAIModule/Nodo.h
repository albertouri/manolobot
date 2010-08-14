#pragma once
#include <BWAPI.h>
#include <BWTA.h>

using namespace BWAPI;
using namespace BWTA;

class Nodo
{
public:
	Nodo(void);
	~Nodo(void);
	Nodo(Region* reg);

	Region* getRegion();
	void setRegion(Region *reg);
	void setSiguiente(Nodo *nodo);
	Nodo* getSiguiente();
	
private:
	Region *r;
	Nodo *siguiente;
};
