#pragma once

#include <BWAPI.h>
#include <BWTA.h>
#include "Utilidades.h"
#include "compania.h"

using namespace BWAPI;
using namespace BWTA;

class CompaniaTransporte
{
public:
	CompaniaTransporte(Position* baseEnem, Region* regEnem, compania* c);
	~CompaniaTransporte(void);
	void onFrame();
	void asignarUnidad(Unit* u);

	bool faltanDropships();
	bool faltanWraiths();

	// retorna true si la compañia de transporte esta lista a transportar las unidades
	bool listaTransportar();

	// ejecuta los movimientos hacia la base enemiga
	void ejecutarTransporte();

private:
	std::list<Unit*> listDropships;
	std::list<Unit*> listWraiths;
	std::list<Position*> pathBaseEnemiga;

	Position *puntoDesembarco;

	// puntero a la posicion del centro de comando enemigo
	Position *baseEnemiga;
	Region *regionBaseEnemiga;

	// puntero al punto de la region de la base enemiga mas lejano del chokepoint defendido
	//Position *bordeMasLejano;

	void dibujarPath();
	void crearPath();
	void desembarcar();

	void controlarEliminados();
	
	std::list<Position*>::iterator ItPosiciones;
	Unit *liderFormacion;

	// variable que una vez que se hace true evita realizar un control mejorando la eficiencia, representa si la compañia de transporte esta lista para realizar el transporte
	bool ready;

	// puntero a la compañia a transportar
	compania *aero;

	// es true si el comandante fue cargado en una nave de transporte
	bool comandanteCargado;

	//-- posibles estados de la compañia de transporte
	static const int ESPERANDO_CARGAR = 1;
	static const int CARGANDO = 2;
	static const int TRANSPORTANDO = 3;
	static const int DESEMBARCANDO = 4;
	static const int RETORNANDO_BASE = 5;

	// variable que mantiene el estado actual de la compañia de transporte
	int estadoActual;

	//--

	// retorna true si todos los dropships estan vacios
	bool desembarcoListo();

	// manda los dropships de vuelta a la base
	void retornarBase();

	// asigna un nuevo lider al grupo de transporte, si es que hay alguna unidad existente
	void reasignarLiderFormacion();

	bool seteadoPuntoDesembarco;

	// contador que determina un tiempo maximo de espera para que se carguen todas las unidades en los transportes. Si se alcanza el tiempo maximo
	// se envia a la compañia de transporte al ataque aunque no esten cargadas todas las unidades de la compañia
	int esperaDeCarga;
	static const int ESPERA_MAXIMA = 500;
};
