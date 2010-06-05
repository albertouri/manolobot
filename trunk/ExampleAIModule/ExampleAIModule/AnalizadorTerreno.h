#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h> 

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
	void dibujar(void);
	BWAPI::Position * obtenerCentroChokepoint();
	bool analisisListo(void);

private:
	void drawStats();
	bool show_visibility_data;
	
};
