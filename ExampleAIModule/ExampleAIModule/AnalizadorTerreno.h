#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h> 
#include "Utilidades.h"

using namespace BWTA;
using namespace BWAPI;

// Variable que determina si el analisis del terreno fue realizado
static bool analyzed;

// Esta variable es utilizada para saber si el analisis del terreno finalizo en el frame anterior
static bool analysis_just_finished;

// Region donde el bot inicia el juego
static BWTA::Region* home;

// Region donde el enemigo inicia el juego
static BWTA::Region* enemy_base;

// Thread que realiza el analisis del terreno
DWORD WINAPI AnalyzeThread();

class AnalizadorTerreno
{
public:
	AnalizadorTerreno(void);
	~AnalizadorTerreno(void);
	
	// Retorna true si el BWTA finalizo de analizar el terreno
	bool analisisListo(void);

	// Devuelve un puntero a la region en la que se inicia el juego
	Region* regionInicial(); 
	
	// Obtiene el chokepoint que se debe defender desde la posicion inicial donde se inicia el juego
	Chokepoint* obtenerChokepoint(); 
	
	// Devuelve la posicion del centro del chokepoint inicial que se debe defender
	Position * obtenerCentroChokepoint(); 
	
	// Dibuja datos sobre el mapa (regiones del bwta, tipo de disparos realizados, etc...)
	void dibujarResultados(void);

	// Calcula el angulo que tiene el chokepoint pasado como parametro
	int calcularAngulo(Chokepoint *c);
	
	// Calcula el angulo que debera tener un grupo de bunkers para defender cierto chokepoint cuyo angulo es pasado como parametro
	int calcularAnguloGrupo(int angulo);
	
	/* Determina en que cuadrante esta la posicion pasada como parametro (divide al mapa en 4 cuadrantes: 
	1- mitad superior izquierda del mapa 
	2- mitad superior derecha del mapa
	3- mitad inferior izquierda del mapa
	4- mitad inferior derecha del mapa */
	int getCuadrante(Position p); 

	// Determina la orientacion de un grupo de bunkers, es decir si las misile turrets se ubican de un lado u otro de los bunkers
	int getOrientacion(Chokepoint *c, Region *r);

private:
	// Flags relativas al BWTA
	bool show_visibility_data;
	bool show_bullets;
	
	// Metodos relativos al BWTA
	void drawStats();
	void drawBullets();
	void drawVisibilityData();
	void drawTerrainData();
	
};
