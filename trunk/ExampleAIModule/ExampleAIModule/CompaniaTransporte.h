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
	
	// Metodo que se ejecuta en cada frame para realizar distintas tareas
	void onFrame();
	
	// Agrega una unidad a la compa�ia de transporte
	void asignarUnidad(Unit* u);

	// Retorna true si hay menos dropships de los necesarios en la compa�ia
	bool faltanDropships();
	
	// Retorna true si hay menos wraiths de los necesarios en la compa�ia
	bool faltanWraiths();

	// retorna true si la compa�ia de transporte esta lista a transportar las unidades
	bool listaTransportar();

	// ejecuta los movimientos hacia la base enemiga
	void ejecutarTransporte();

private:
	// Lista de unidades de la compa�ia de transporte
	std::list<Unit*> listDropships;
	std::list<Unit*> listWraiths;
	std::list<Position*> pathBaseEnemiga;

	// Posicion donde va a desembarcar las unidades la compa�ia de transporte
	Position *puntoDesembarco;

	// Puntero a la posicion del centro de comando enemigo
	Position *baseEnemiga;
	// Region donde esta ubicada la base enemiga
	Region *regionBaseEnemiga;

	// Puntero al punto de la region de la base enemiga mas lejano del chokepoint defendido
	//Position *bordeMasLejano;

	// Dibuja el camino a la base enemiga
	void dibujarPath();
	
	// Dibuja un recuadro sobre el lider de la formacion
	void recuadrarUnidades();
	
	// Crea una lista de posiciones que representa el camino hacia la base enemiga
	void crearPath();
	
	// Ejecuta el desembarco de las unidades transportadas por la compa�ia de transporte
	void desembarcar();

	// Controla las listas de unidades de la compa�ia defensiva y elimina de las mismas las unidades que hayan sido destruidas
	void controlarEliminados();
	
	// Iterador de la lista de posiciones
	std::list<Position*>::iterator ItPosiciones;
	Unit *liderFormacion;

	// Variable que una vez que se hace true evita realizar un control mejorando la eficiencia, representa si la compa�ia de transporte esta lista para realizar el transporte
	bool ready;

	// Puntero a la compa�ia a transportar
	compania *aero;

	// Es true si el comandante fue cargado en una nave de transporte
	bool comandanteCargado;

	//-- Posibles estados de la compa�ia de transporte
	static const int ESPERANDO_CARGAR = 1;
	static const int CARGANDO = 2;
	static const int TRANSPORTANDO = 3;
	static const int DESEMBARCANDO = 4;
	static const int RETORNANDO_BASE = 5;

	// Variable que mantiene el estado actual de la compa�ia de transporte
	int estadoActual;

	//--

	// Retorna true si todos los dropships estan vacios
	bool desembarcoListo();

	// Manda los dropships de vuelta a la base
	void retornarBase();

	// Asigna un nuevo lider al grupo de transporte, si es que hay alguna unidad existente
	void reasignarLiderFormacion();

	// Retorna true si el punto de desembarco fue seteado
	bool seteadoPuntoDesembarco;

	// Contador que determina un tiempo maximo de espera para que se carguen todas las unidades en los transportes. Si se alcanza el tiempo maximo
	// se envia a la compa�ia de transporte al ataque aunque no esten cargadas todas las unidades de la compa�ia
	int esperaDeCarga;
	static const int ESPERA_MAXIMA = 500;
};
