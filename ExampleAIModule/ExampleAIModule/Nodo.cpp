#include "Nodo.h"

Nodo::Nodo(void)
{
	siguiente = NULL;
}

Nodo::Nodo(Region* reg)
{
	siguiente = NULL;
	r = reg;
}

Nodo::~Nodo(void)
{
}


Region* Nodo::getRegion(){
	return r;
}

void Nodo::setRegion(Region *reg){
	r = reg;
}

void Nodo::setSiguiente(Nodo *nodo){
	siguiente = nodo;
}

Nodo* Nodo::getSiguiente(){
	return siguiente;
}

