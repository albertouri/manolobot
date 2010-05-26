#include <BWAPI.h>
#include "compania.h"
#include <list>

 std::list<Unit*> lista;
Unit *unidad1, *unidad2, *unidad3;
int i=1;

compania::compania(void)
{
}

compania::~compania(void)
{
}

void compania::asignarUnidad(Unit *U){
	if(i == 1) unidad1 = U;/*
	else if(i==2) unidad2 = U;
	else if(i==3) unidad3 = U;*/
	i++;
	
	lista.push_front(U);
	
	conteoUnidades();
	if ((unidad1!=NULL)&&(unidad1->exists())) unidad1->rightClick(*(new TilePosition(1,1)));

}

void compania::conteoUnidades(){
	int j=0;
	std::list<Unit*>::iterator It1;
    It1 = lista.begin();
	while(It1 != lista.end()){
		if(!(*It1)->exists()) It1 = lista.erase(It1);	
		else {It1++; j++;}

	}
	Broodwar->printf("cantidad = %d", j);
	
}