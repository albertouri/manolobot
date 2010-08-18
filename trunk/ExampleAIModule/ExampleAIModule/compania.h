#pragma once
#include <BWAPI.h>
#include "Graficos.h"

using namespace BWAPI;

class compania
{
public:
	compania(Color ID);
	void asignarUnidad(Unit *U);
	void aplicarStim(std::list<Unit*> lista); // aplica el stim_pack a las unidades pertenecientes a la compa�ia si no estan dentro de un contenedor (bunker, dropship)

	int countMarines(); // elimina de la lista correspondiente las unidades destruidas, y retorna la cantidad de unidades vivas
	int countMedics(); // elimina de la lista correspondiente las unidades destruidas, y retorna la cantidad de unidades vivas
	int countFirebats(); // elimina de la lista correspondiente las unidades destruidas, y retorna la cantidad de unidades vivas
	int countTanks();
	int countGoliaths();
	int countScienceVessels();

	void atacar(Unit *u); // manda a la compa�ia a atacar a la unidad pasada como parametro
	void onFrame();

	void moverCompania(Position pos); // mueve la compa�ia a la posicion pasada como parametro

	bool pertenece(Unit *u); // retorna true si la unidad pasada como parametro pertenece a la compa�ia

	~compania(void);

	bool listaParaAtacar();

	// retorna la cantidad de transportes necesarios para cargar todas las unidades de la compa�ia
	int cantidadTransportes();

	void abordarTransporte(std::list<Unit*> transportes);
	bool companiaAbordo();

private:
	Color c; // color especifico para cada compa�ia, seteado en el constructor
	Unit *comandante;

	// listas de unidades de la compa�ia
	std::list<Unit*> listMarines;
	std::list<Unit*> listMedics;
	std::list<Unit*> listFirebats;
	std::list<Unit*> listTanks;
	std::list<Unit*> listGoliath;
	std::list<Unit*> listScienceVessel;

	void conteoUnidades(void);
	void ponerACubierto(Unit *U);
	int contarUnidades(std::list<Unit*> *lista);
	void actualizarEstado(std::list<Unit*> *lista); // elimina de la lista las unidades que hayan dejado de existir...

	Unit* buscarDa�ado(std::list<Unit*> lista); // retorna la primer unidad da�ada y que no este siendo curada en la lista pasada como parametro

	void controlarDistancia(); // controla que los soldados de la compa�ia se mantengan cerca del comandante

	// variable que mantiene la cantidad de transportes necesarios para cargar la compa�ia completa
	int cantTransportes;

	// recalcula la cantidad de transportes necesarios para cargar la compa�ia completa
	void calcularTransportes();
};


