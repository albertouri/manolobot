#include <BWAPI.h>
#include "compania.h"
#include <list>

Unit *unidad1, *unidad2, *unidad3;
int i=1;

compania::compania(void)
{
}

compania::~compania(void)
{
}

void compania::asignarUnidad(Unit *U){
	if(i == 1) unidad1 = U;
	else if(i==2) unidad2 = U;
	else if(i==3) unidad3 = U;
	i++;

	if ((unidad1!=NULL)&&(unidad1->exists())) unidad1->rightClick(*(new TilePosition(1,1)));

}