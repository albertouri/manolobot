#pragma once
#include <BWAPI.h>
#include "AnalizadorTerreno.h"
#include "Utilidades.h"
#include "compania.h"
#include "CompaniaDefensiva.h"
#include "Scout.h"
#include "Graficos.h"
#include <math.h>
#include "GrupoBunkers.h"
#include <stdlib.h>
#include "Grafo.h"
#include "CompaniaTransporte.h"
#include "GrupoAntiaereo.h"

using namespace BWAPI;
using namespace BWTA;


class unit_Manager
{
public:
	unit_Manager(AnalizadorTerreno *analizador);
	void executeActions();
	void resetBuildingSemaphore();
	
	void setGoals(int goals[34]);
	void setResearchs(int researchs[Utilidades::maxResearch]);
	void asignarUnidadACompania(Unit* unit);
	
	void verificarBunkers(); // verifica si un bunker esta siendo atacado
	void repararUnidad(Unit *u); // repara la unidad pasada como parametro

	void onUnitCreate(Unit *u);
	void onUnitDestroy(Unit *u);
	void onNukeDetect(Position p);
	void onUnitShow(Unit *u);

	bool* getResearchsDone();

	// setea el estado actual
	void setEstadoActual(int estado);

	~unit_Manager(void);

private:
	//-- VARIABLES GLOBALES

	// arreglo que mantiene la cantidad de unidades de cada tipo que hay actualmente
	int cantUnidades[34];
	// arreglo que mantiene la cantidad de unidades de cada tipo que deberia haber
	int goalCantUnidades[34];
	// arreglo que mantiene las investigaciones que deben realizarce
	int goalResearch[Utilidades::maxResearch];
	// arreglo que mantiene informacion sobre si una investigacion se realizo o no
	bool researchDone[Utilidades::maxResearch];
	// puntero a un SCV que se encargara de reparar los bunkers que sean atacados
	Unit *reparador1, *reparador2;

	// puntero a la posicion del centro de comando enemigo
	Position *baseEnemiga;
	Region *regionBaseEnemiga;

	Region *regionBasePrincipalEnemiga;

	// variable auxiliar para encontrar la posicion del centro de comando enemigo
	bool primerConstruccionDescubierta;
	// esta variable es true si el analisis del terreno finalizo
	bool analisisListo;

	//-- Grupos de bunkers
	GrupoBunkers *grupoB1, *grupoB2;

	// mantiene la posicion del centro de comando
	TilePosition *centroComando;
	//puntero al centro de comando;
	Unit *centroDeComando;

	// Compañias de unidades
	compania* Easy;
	//compania* Charlie;
	
	// compañia que defiende la base
	CompaniaDefensiva* Fox;

	Grafo *grf;
	CompaniaTransporte *ct;
	GrupoAntiaereo *anti;

	int latenciaScout;

	Scout* magallanes;
	Player* enemigo;

	int SCVgatheringMinerals, SCVgatheringGas;

	// Lista de unidades que estan en construccion actualmente
	std::list<Unit*> unidadesEnConstruccion;

	// puntero a la ultima unidad finalizada, se calcula en cada frame con una llamada a controlarFinalizacion()
	//Unit *ultimaFinalizada; 

	int frameLatency;
	int buildingSemaphore;

	AnalizadorTerreno* analizador;

	// estado actual tomado del strategy manager
	int estadoActual;

	//-- METODOS

	// construye una unidad en la ubicacion y del tipo pasados como parametro
	void buildUnit(TilePosition *pos, int id);

	// construye un add-on cuyo tipo se corresponde con el pasado como parametro
	void buildUnitAddOn(int id);

	// construye una refineria en la posicion pasada como parametro. Si la posicion es NULL construye en el geiser mas cercano
	void makeRefinery(TilePosition *pos);

	// retorna un puntero a un SCV
	Unit* getWorker();

	// retorna un puntero a una unidad del tipo pasado como parametro
	Unit* getUnit(int IDTipo);

	// manda a la unidad pasada como parametro a recolectar minerales
	void sendGatherCristal(Unit* worker);

	// manda a la unidad pasada como parametro a recolectar gas
	void sendGatherGas(Unit* worker);

	// construye un SCV
	void trainWorker();

	// entrena un marine
	void trainMarine();

	// entrena un medico
	void trainMedic();

	// entrena un ghost
	void trainGhost();

	// construye un Vulture
	void trainVulture();

	// construye un Goliath
	void trainGoliath();

	// construye un tanque de asedio
	void trainTankSiege();

	void trainUnit(int ID);

	// calcula una TilePosition disponible para construir una unidad del tipo pasado como parametro
	// despues de usar este metodo ejecutar delete sobre el resultado para liberar memoria
	TilePosition* getTilePositionAviable(UnitType* U);

	//método especial para ubicar los supply depot
	TilePosition* getTilePositionForSupply(AnalizadorTerreno *analizador);

	//método para determinar si una unidad entra en la region
	bool isInsideRegion(AnalizadorTerreno *analizador, UnitType* U, TilePosition* P);

	// despues de usar este metodo ejecutar delete sobre el resultado para liberar memoria
	TilePosition* getTilePositionAviable(UnitType* U, TilePosition* t);

	bool isFreeOfBuildingsRightAndLeft(UnitType* U, TilePosition* t);
	
	TilePosition* getPosicionDistinta(TilePosition actual);

	// retorna true si una unidad del tipo pasado como parametro esta en construccion, false en caso contrario
	bool construyendo(int ID);

	// si hay unidades en el TilePosition pasado como parametro los mueve a otro build tile
	void moverUnidades(TilePosition *t); 
	
	// este metodo se invoca cuando se genera el evento onUnitCreate, y agrega la unidad en construccion a una lista
	void nuevaUnidadConstruccion(Unit *u);
	
	// retorna la primera unidad que haya sido terminada de construir que encuentre en la lista, NULL si no encuentra ninguna
	Unit* controlarFinalizacion();

	// manda un SCV a finalizar la construccion de la unidad pasada como parametro, ya que no fue finalizada y actualmente no esta siendo construida por ninguna unidad
	void finalizarConstruccion(Unit *u);

	// metodo que controla las diferentes construcciones a realizar en el juego
	void ejecutarConstrucciones();

	// metodo que se debe invocar cada cierta cantidad de frames, para ubicar las unidades enemigas que estan burrowed o cloacked y no sean visibes
	void buscarUnidadesOcultas();
};
