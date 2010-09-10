#pragma once
#include <BWAPI.h>
#include "Graficos.h"
#include "AnalizadorTerreno.h"

using namespace BWAPI;

class compania
{
public:
	compania(AnalizadorTerreno *at, Color ID);
	void asignarUnidad(Unit *U);
	void aplicarStim(std::list<Unit*> lista); // aplica el stim_pack a las unidades pertenecientes a la compa�ia si no estan dentro de un contenedor (bunker, dropship)

	int countMarines(); // elimina de la lista correspondiente las unidades destruidas, y retorna la cantidad de unidades vivas
	int countMedics(); // elimina de la lista correspondiente las unidades destruidas, y retorna la cantidad de unidades vivas
	int countFirebats(); // elimina de la lista correspondiente las unidades destruidas, y retorna la cantidad de unidades vivas
	int countTanks();
	int countGoliaths();
	int countScienceVessels();

	// manda a la compa�ia a atacar a la unidad pasada como parametro
	void atacar(Unit *u);

	void onFrame();

	// mueve la compa�ia a la posicion pasada como parametro
	void moverCompania(Position pos);

	// retorna true si la unidad pasada como parametro pertenece a la compa�ia
	bool pertenece(Unit *u);

	~compania(void);

	// retorna true si en la compa�ia hay cierta cantidad de unidades de cada tipo
	bool listaParaAtacar();

	// retorna la cantidad de transportes necesarios para cargar todas las unidades de la compa�ia
	int cantidadTransportes();

	// ordena a las unidades de la compa�ia subir a los transportes de la lista pasada como parametro
	void abordarTransporte(std::list<Unit*> *transportes);

	// retorna true si todas las unidades de la compa�ia estan cargadas
	bool companiaAbordo();


	void setBasesEnemigas(TilePosition* enemigo);


	// retorna un puntero al comandante de la compa�ia

	Unit* getComandante();

private:
	Color c; // color especifico para resaltar las unidades de cada compa�ia, seteado en el constructor
	Unit *comandante; // puntero al comandante de la compa�ia
	AnalizadorTerreno* analizador; // puntero al analizador del terreno
	TilePosition * posicionEnemigo;
	bool esperar;

	// listas de unidades de la compa�ia
	std::list<Unit*> listMarines;
	std::list<Unit*> listMedics;
	std::list<Unit*> listFirebats;
	std::list<Unit*> listTanks;
	std::list<Unit*> listGoliath;
	std::list<Unit*> listScienceVessel;

	std::list<Unit*> listRefuerzos;

	//Setea el comandante principal y el comandante de cada compa�ia
	void setComandantes(void);

	void conteoUnidades(void);
	void ponerACubierto(Unit *U);
	int contarUnidades(std::list<Unit*> *lista);

	void asignarAPelotones(Unit *u);
	void asignarARefuerzos(Unit *U);
	// elimina de la lista las unidades que hayan sido destruidas
	void actualizarEstado(std::list<Unit*> *lista);

	// mueve todas las tropas a la posicion anterior.
	void retirada(void);

	// retorna la primer unidad da�ada y que no este siendo curada en la lista pasada como parametro
	Unit* buscarDa�ado(std::list<Unit*> lista);

	// controla que los soldados de la compa�ia se mantengan cerca del comandante
	void controlarDistancia();

	// variable que mantiene la cantidad de transportes necesarios para cargar la compa�ia completa
	int cantTransportes;

	// recalcula la cantidad de transportes necesarios para cargar la compa�ia completa
	void calcularTransportes();

	//setea la compa�ia para que espere por el transporte o que deje de esperarlo
	void setComportanmientoEsperando(void);
	void setComportanmientoNormal(void);
};


