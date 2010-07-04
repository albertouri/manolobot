#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h> 

using namespace BWTA;
using namespace BWAPI;

static bool analyzed;
static bool analysis_just_finished;
static BWTA::Region* home;
static BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();

class AnalizadorTerreno
{
public:
	AnalizadorTerreno(void);
	~AnalizadorTerreno(void);
	
	Position * obtenerCentroChokepoint(); // devuelve la posicion del centro del chokepoint que se debe defender
	bool analisisListo(void); // retorna true si el BWTA finalizo de analizar el terreno

	Region* regionInicial(); // devuelve un puntero a la region en la que se inicia el juego
	Chokepoint* obtenerChokepoint(); // Obtiene el chokepoint que se debe defender desde la posicion inicial
	void dibujarResultados(void); // hace un dibujito

	TilePosition* AnalizadorTerreno::calcularPrimerTile(Region* r, Chokepoint* c); // retorna la posicion donde deberia ubicarse un bunker para defender el chokepoint pasado como parametro, retorna NULL si no pudo encontrar una posicion posible
	int calcularAngulo(Position *p1, Position *p2);

private:
	bool show_visibility_data;
	
};
