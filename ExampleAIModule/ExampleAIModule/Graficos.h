#pragma once

#include <BWAPI.h>
#include <BWTA.h>

using namespace BWAPI;

class Graficos
{
public:
	// TODO: Agregar los metodos que dibujan de la clase Scout...

	// Dibuja un recuadro alrededor de la unidad pasada como parametro
	static void resaltarUnidad(Unit *u);
	static void resaltarUnidad(Unit *u, Color c);

	// Dibuja un rectángulo en  la posición y con las dimensiones pasadas como parámetro
	static void dibujarCuadro(TilePosition* p, int tilesAncho, int tilesAlto);
	static void dibujarCuadro(TilePosition* p, int tilesAncho, int tilesAlto, Color c, bool relleno);
	static void dibujarCirculo(TilePosition p, int tilesAncho, int tilesAlto);
	
	// Dibuja en el mapa los resultados del analisis del terreno realizado por el BWTA
	static void dibujarTerreno(bool show_visibility_data, bool analyzed); 

private:
	// Muestra en pantalla estadisticas del juego
	static void drawStats();
};
