#include "CompaniaTransporte.h"

CompaniaTransporte::CompaniaTransporte(Position* baseEnem, Region* regEnem, compania* c){
	// Posicion de la base enemiga
	baseEnemiga = baseEnem;
	
	// Region donde esta ubicada la base enemiga
	regionBaseEnemiga = regEnem;

	puntoDesembarco = NULL;
	seteadoPuntoDesembarco = false;

	if (baseEnemiga != NULL){
		//-- crea el camino que recorreran los transportes
		crearPath();
	}

	liderFormacion = NULL;
	ready = false;
	aero = c;
	comandanteCargado = false;

	estadoActual = ESPERANDO_CARGAR;
	esperaDeCarga = 0;
}


CompaniaTransporte::~CompaniaTransporte(void)
{
}



void CompaniaTransporte::crearPath(){
	// Region donde iniciamos el juego
	Region *regInicial = BWTA::getStartLocation(Broodwar->self())->getRegion();
	int ubicacionDefensa = 0;

	// Variables que mantienen si la base enemiga esta sobre alguno de los bordes de la pantalla, sobreDer sera true si la base enemiga esta sobre el borde derecho de la pantalla
	bool sobreDer = false, sobreIzq = false, sobreArr = false, sobreAba = false;

	int CONST_DER = 2, CONST_ABA = 1, CONST_IZQ = 4, CONST_ARR = 3;
	
	// ubico el choke con respecto al centro de la base enemiga
	if ((abs(regionBaseEnemiga->getCenter().x() - (*regionBaseEnemiga->getChokepoints().begin())->getCenter().x())) < (abs(regionBaseEnemiga->getCenter().y() - (*regionBaseEnemiga->getChokepoints().begin())->getCenter().y()))){
		// el choke esta desplazado sobre el eje y, esta arriba o abajo del centro de la base
		if (regionBaseEnemiga->getCenter().y() < (*regionBaseEnemiga->getChokepoints().begin())->getCenter().y()){
			// el choke esta abajo
			ubicacionDefensa = CONST_ABA;
		}
		else{
			// el choke esta arriba
			ubicacionDefensa = CONST_ARR;
		}
	}
	else{
		// el choke esta desplazado sobre el eje x, esta a la derecha o izquierda del centro de la base
		if (regionBaseEnemiga->getCenter().x() < (*regionBaseEnemiga->getChokepoints().begin())->getCenter().x()){
			// el choke esta a la derecha
			ubicacionDefensa = CONST_DER;
		}
		else{
			// el choke esta a la izquierda
			ubicacionDefensa = CONST_IZQ;
		}
	}

	// posiciones mas arriba, mas abajo, mas derecha y mas izquierda de la region
	int masArr = 0, masAba = 0, masDer = 0, masIzq = 0;

	// calcula las bordes del mapa acotados desde el centro de la region
	int acoArr = 0, acoAba = 0, acoDer = 0, acoIzq = 0;

	BWTA::Polygon p = regionBaseEnemiga->getPolygon();
	for(int j=0; j<(int)p.size(); j++)
	{
		if (p[j].x() < p[masIzq].x())
			masIzq = j;

		if (p[j].x() > p[masDer].x())
			masDer = j;

		if (p[j].y() < p[masArr].y())
			masArr = j;

		if (p[j].y() > p[masAba].y())
			masAba = j;

		//--
		if (((p[j].y() > (regionBaseEnemiga->getCenter().y() - 3 * TILE_SIZE)) && (p[j].y() < (regionBaseEnemiga->getCenter().y() + 3 * TILE_SIZE))) && (p[j].x() < p[acoIzq].x()))
			acoIzq = j;

		if (((p[j].y() > (regionBaseEnemiga->getCenter().y() - 3 * TILE_SIZE)) && (p[j].y() < (regionBaseEnemiga->getCenter().y() + 3 * TILE_SIZE))) && (p[j].x() > p[acoDer].x()))
			acoDer = j;

		if (((p[j].x() > (regionBaseEnemiga->getCenter().x() - 3 * TILE_SIZE)) && (p[j].x() < (regionBaseEnemiga->getCenter().x() + 3 * TILE_SIZE))) && (p[j].y() < p[acoArr].y()))
			acoArr = j;

		if (((p[j].x() > (regionBaseEnemiga->getCenter().x() - 3 * TILE_SIZE)) && (p[j].x() < (regionBaseEnemiga->getCenter().x() + 3 * TILE_SIZE))) && (p[j].y() > p[acoAba].y()))
			acoAba = j;
	}

	sobreDer = (p[masDer].x() > ((Broodwar->mapWidth() - 3) * TILE_SIZE));
	sobreIzq = (p[masIzq].x() < (3 * TILE_SIZE));
	sobreAba = (p[masAba].y() > ((Broodwar->mapHeight() - 3) * TILE_SIZE));
	sobreArr = (p[masArr].y() < (3 * TILE_SIZE));

	// posibles puntos de desembarco
	Position *pArr = NULL, *pAba = NULL, *pDer = NULL, *pIzq = NULL;
	
	if (regInicial->getCenter().x() < regionBaseEnemiga->getCenter().x()){
		// empiezo a la izquierda con respecto al enemigo
		if (regInicial->getCenter().y() < regionBaseEnemiga->getCenter().y()){
			// empiezo arriba a la izquierda con respecto al enemigo
			Broodwar->printf("empiezo arriba a la izquierda con respecto al enemigo");
			if (ubicacionDefensa == CONST_ARR){
				Broodwar->printf("Esta defendido arriba");
				//if (sobreAba){
					// no se puede entrar por abajo, entro por la izquierda
					Position *p1 = new Position(regInicial->getCenter().x(), regInicial->getCenter().y());
					Position *p2 = new Position(regInicial->getCenter().x(), p[acoIzq].y());
					puntoDesembarco = new Position(p[acoIzq].x() + TILE_SIZE, p[acoIzq].y());

					//Position *puntoDesembarco = new Position(p[acoIzq].x() + TILE_SIZE, p[acoIzq].y() - TILE_SIZE);

					pathBaseEnemiga.push_back(p1);
					pathBaseEnemiga.push_back(p2);
					pathBaseEnemiga.push_back(puntoDesembarco);
				//}
			}
			else if (ubicacionDefensa == CONST_IZQ){
				Broodwar->printf("Esta defendido izquierda");

				Position *p1 = new Position(regInicial->getCenter().x(), regInicial->getCenter().y());
				Position *p2 = new Position(p[acoArr].x(), regInicial->getCenter().y());
				puntoDesembarco = new Position(p[acoArr].x(), p[acoArr].y() + TILE_SIZE);

				pathBaseEnemiga.push_back(p1);
				pathBaseEnemiga.push_back(p2);
				pathBaseEnemiga.push_back(puntoDesembarco);
			}
		}
		else{
			// empiezo abajo a la izquierda con respecto al enemigo
			Broodwar->printf("empiezo abajo a la izquierda con respecto al enemigo");

			if (ubicacionDefensa == CONST_ABA){
				Broodwar->printf("Esta denfendido abajo");

				//if (sobreAba){
					// no se puede entrar por abajo, entro por la derecha
					Position *p1 = new Position(regInicial->getCenter().x(), regInicial->getCenter().y());
					Position *p2 = new Position(regInicial->getCenter().x(), p[acoIzq].y());
					puntoDesembarco = new Position(p[acoIzq].x() + TILE_SIZE, p[acoIzq].y());

					pathBaseEnemiga.push_back(p1);
					pathBaseEnemiga.push_back(p2);
					pathBaseEnemiga.push_back(puntoDesembarco);
				//}
			}
			else if (ubicacionDefensa == CONST_IZQ){
				Broodwar->printf("Esta defendido izquierda");

				Position *p1 = new Position(regInicial->getCenter().x(), regInicial->getCenter().y());
				Position *p2 = new Position(p[acoArr].x(), regInicial->getCenter().y());
				puntoDesembarco = new Position(p[acoArr].x(), p[acoArr].y() - TILE_SIZE);

				pathBaseEnemiga.push_back(p1);
				pathBaseEnemiga.push_back(p2);
				pathBaseEnemiga.push_back(puntoDesembarco);
			}
		}
	}
	else{
		// empiezo a la derecha con respecto al enemigo
		
		if (regInicial->getCenter().y() < regionBaseEnemiga->getCenter().y()){
			// empiezo arriba a la derecha con respecto al enemigo
			Broodwar->printf("empiezo arriba a la derecha con respecto al enemigo");

			if (ubicacionDefensa == CONST_ARR){
				Broodwar->printf("Esta defendido arriba");
				//if (sobreAba){
					// no se puede entrar por arriba, entro por la derecha
					Position *p1 = new Position(regInicial->getCenter().x(), regInicial->getCenter().y());
					Position *p2 = new Position(regInicial->getCenter().x(), p[acoDer].y());
					puntoDesembarco = new Position(p[acoDer].x() - TILE_SIZE, p[acoDer].y());

					pathBaseEnemiga.push_back(p1);
					pathBaseEnemiga.push_back(p2);
					pathBaseEnemiga.push_back(puntoDesembarco);
				//}
			}
			else if (ubicacionDefensa == CONST_DER){
				Broodwar->printf("Esta defendido derecha");

				Position *p1 = new Position(regInicial->getCenter().x(), regInicial->getCenter().y());
				Position *p2 = new Position(p[acoArr].x(), regInicial->getCenter().y());
				puntoDesembarco = new Position(p[acoArr].x(), p[acoArr].y() + TILE_SIZE);

				pathBaseEnemiga.push_back(p1);
				pathBaseEnemiga.push_back(p2);
				pathBaseEnemiga.push_back(puntoDesembarco);
			}
		}
		else{
			// empiezo abajo a la derecha con respecto al enemigo
			Broodwar->printf("empiezo abajo a la derecha con respecto al enemigo");

			if (ubicacionDefensa == CONST_ABA){
				//if (sobreAba){
					Broodwar->printf("Esta defendido abajo");
					// no se puede entrar por abajo, entro por la derecha
					Position *p1 = new Position(regInicial->getCenter().x(), regInicial->getCenter().y());
					Position *p2 = new Position(regInicial->getCenter().x(), p[acoDer].y());
					puntoDesembarco = new Position(p[acoDer].x() - TILE_SIZE, p[acoDer].y());

					pathBaseEnemiga.push_back(p1);
					pathBaseEnemiga.push_back(p2);
					pathBaseEnemiga.push_back(puntoDesembarco);
				//}
			}
			else if (ubicacionDefensa == CONST_DER){
				Broodwar->printf("Esta defendido derecha");

				Position *p1 = new Position(regInicial->getCenter().x(), regInicial->getCenter().y());
				Position *p2 = new Position(p[acoAba].x(), regInicial->getCenter().y());
				puntoDesembarco = new Position(p[acoAba].x(), p[acoAba].y() - TILE_SIZE);

				pathBaseEnemiga.push_back(p1);
				pathBaseEnemiga.push_back(p2);
				pathBaseEnemiga.push_back(puntoDesembarco);
			}
		}
	}

	ItPosiciones = pathBaseEnemiga.begin();
	
}


void CompaniaTransporte::dibujarPath(){
	std::list<Position*>::iterator It;
	
	// La primer posicion a dibujar es el centro de la region donde inicia el juego el bot
	Position *anterior = new Position(BWTA::getStartLocation(Broodwar->self())->getRegion()->getCenter().x(), BWTA::getStartLocation(Broodwar->self())->getRegion()->getCenter().y());

	// Dibuja una linea entre la posicion anterior y la proxima posicion en el camino a la base enemiga
	It = pathBaseEnemiga.begin();
	
	while (It != pathBaseEnemiga.end()){
		Broodwar->drawLineMap(anterior->x(), anterior->y(), (*It)->x(), (*It)->y(), Colors::White);

		if (It == pathBaseEnemiga.begin())
			delete anterior;	

		anterior = (*It);

		It++;
	}
}


void CompaniaTransporte::onFrame(){
	// Descomentar para dibujar el camino que recorrera la compañia de transporte hacia la base enemiga
	//dibujarPath();

	// Dependiendo del estado actual realiza diferentes acciones
	if (estadoActual == CARGANDO)
		esperaDeCarga++; // Contador de frames que la compañia esta tardando en abordar los transportes

	// Descomentar para mostrar un recuadro sobre el lider de la formacion (si es que existe)
	//recuadrarUnidades();

	if ((Broodwar->getFrameCount() % 12 == 0)){
		if ((liderFormacion == NULL) || (!liderFormacion->exists())){
			controlarEliminados();

			if (listDropships.empty() && listWraiths.empty() && ((estadoActual != ESPERANDO_CARGAR) || (comandanteCargado))){
				// Si todas las unidades de la compañia de transporte fueron destruidas, se vuelve al estado inicial (ESPERANDO_CARGAR)
				estadoActual = ESPERANDO_CARGAR;
				comandanteCargado = false;
				ItPosiciones = pathBaseEnemiga.begin();
				esperaDeCarga = 0; // }}
			}
			else
				reasignarLiderFormacion(); // Si hay alguna unidad viva en la compañia asigna un nuevo lider a la misma
		}
		
		if (estadoActual == CARGANDO){
			if (esperaDeCarga < ESPERA_MAXIMA){
				// Si la compañia de ataque esta cargada, se inicia el transporte
				if (aero->companiaAbordo()){
					estadoActual = TRANSPORTANDO;
					esperaDeCarga = 0;
				}
				else
					aero->abordarTransporte(&listDropships); // Ordena a las unidades de la compañia de ataque que suban a los transportes
			}
			else{
				// Si se alcanzo el tiempo maximo de espera de carga, la compañia de transporte comienza a movese, no importa si la compañia completa
				// abordo los transportes
				estadoActual = TRANSPORTANDO;
			}
		}
	}
	else if (Broodwar->getFrameCount() % 30 == 0){
	
		if (estadoActual == ESPERANDO_CARGAR){
			if (listaTransportar())
				estadoActual = CARGANDO;
		}
		else if (estadoActual == TRANSPORTANDO){
			ejecutarTransporte();
		}
		else if (estadoActual == DESEMBARCANDO){
			if (desembarcoListo())
				estadoActual = RETORNANDO_BASE;
			else
				desembarcar();
		}
		else if (estadoActual == RETORNANDO_BASE){
			retornarBase();
		}
	}
	else if (Broodwar->getFrameCount() % 50 == 0){
		controlarEliminados();
	}
}


void CompaniaTransporte::asignarUnidad(Unit* u){
	// Controla las unidades eliminadas de la compañia
	controlarEliminados();

	if ((u != NULL) && (u->exists())){
		// Dependiendo del tipo de unidad recibida como parametro, se la agrega a una lista determinada
		if (u->getType().getID() == Utilidades::ID_DROPSHIP){
			listDropships.push_back(u);

			if ((aero != NULL) && (aero->getComandante() != NULL) && (!comandanteCargado)){
				(*listDropships.begin())->load(aero->getComandante());
				comandanteCargado = true;
			}
		}
		else if (u->getType().getID() == Utilidades::ID_WRAITH){
			listWraiths.push_back(u);

			if (liderFormacion == NULL)
				liderFormacion = u;
		}
	}
}


bool CompaniaTransporte::listaTransportar(){
	// La compañia de transporte estara lista a transportar si hay suficientes dropships, wraiths y la compañia de ataque esta lista para atacar
	return ((!faltanDropships()) && (!faltanWraiths()) && (aero->listaParaAtacar()));
}


bool CompaniaTransporte::faltanDropships(){
	// La cantidad de dropships esta dada por la cantidad de transportes necesarios para cargar toda la compañia de ataque
	return (listDropships.size() < aero->cantidadTransportes());
}


bool CompaniaTransporte::faltanWraiths(){
	return (listWraiths.size() < 2);
}


void CompaniaTransporte::controlarEliminados(){
	
	if (!listDropships.empty()){
		std::list<Unit*>::iterator Itd;
		
		Itd = listDropships.begin();
		while (Itd != listDropships.end()){
			if (!(*Itd)->exists()){
				listDropships.erase(Itd);
				Itd = listDropships.begin(); // Si se borra esta linea el juego se cuelga
			}
			else{
				Itd++;
			}
		}
	}

	if (!listWraiths.empty()){
		std::list<Unit*>::iterator Itd;
		
		Itd = listWraiths.begin();
		while (Itd != listWraiths.end()){
			if (!(*Itd)->exists()){
				listWraiths.erase(Itd);
				Itd = listWraiths.begin(); // Si se borra esta linea el juego se cuelga
			}
			else{
				Itd++;
			}
		}
	}
}


void CompaniaTransporte::ejecutarTransporte(){
	std::list<Unit*>::iterator It;

	// En cada momento que el lider de la formacion quede en estado idle (no ejecuta ninguna accion), quiere decir que llego al siguiente punto
	// de navegacion hacia la base enemiga, por lo tanto se actualiza al siguiente punto de navegacion y se ordena a todas las unidades moverse
	// al mismo
	if ((liderFormacion != NULL) && (liderFormacion->exists()) && (liderFormacion->isIdle()) && (ItPosiciones != pathBaseEnemiga.end())){

		It = listWraiths.begin();
		while (It != listWraiths.end()){
			if ((*It)->exists())
				(*It)->move(**ItPosiciones);

			It++;
		}

		It = listDropships.begin();
		while (It != listDropships.end()){
			if ((*It)->exists())
				(*It)->move(**ItPosiciones);

			It++;
		}
		ItPosiciones++;
	}

	if ((liderFormacion != NULL) && (liderFormacion->exists()) && (puntoDesembarco != NULL)){

		// Si se esta lo suficientemente cerca del punto de desembarco, se cambia el estado a DESEMBARCANDO y se ordena a los escoltas
		// moverse hacia el centro de la region donde esta la base enemiga
		if (liderFormacion->getPosition().getDistance(*puntoDesembarco) < 250.0){

			It = listWraiths.begin();
			while (It != listWraiths.end()){
				if ((*It)->exists()){
					(*It)->move(regionBaseEnemiga->getCenter());
				}

				It++;
			}

			// manda a los dropships que desembarquen los soldados
			desembarcar();
			estadoActual = DESEMBARCANDO;
		}
	}
	else if (puntoDesembarco == NULL)
		Broodwar->printf("Error: el punto de desembarco es NULL");
}


bool CompaniaTransporte::desembarcoListo(){
	std::list<Unit*>::iterator It = listDropships.begin();

	// Verifica si todas los dropships desembarcaron las unidades transportadas
	while (It != listDropships.end()){
		if ((*It)->exists() && (!(*It)->getLoadedUnits().empty()))
			return false;

		It++;
	}

	return true;
}


void CompaniaTransporte::desembarcar(){
	std::list<Unit*>::iterator It = listDropships.begin();

	// Ordena a todos los dropships desembarcar las unidades transportadas
	while (It != listDropships.end()){
		if ((*It)->exists())
			(*It)->unloadAll(*puntoDesembarco);

		It++;
	}
}


void CompaniaTransporte::retornarBase(){
	std::list<Unit*>::iterator It;

	// Si las unidades estan en una posicion que no es la de inicio, mueve todas las unidades hacia la posicion inicial
	if ((liderFormacion != NULL) && (liderFormacion->exists()) && (liderFormacion->isIdle()) && (ItPosiciones != pathBaseEnemiga.begin())){

		It = listWraiths.begin();
		while (It != listWraiths.end()){
			if ((*It)->exists())
				(*It)->move(*(*pathBaseEnemiga.begin()));

			It++;
		}

		It = listDropships.begin();
		while (It != listDropships.end()){
			if ((*It)->exists())
				(*It)->move(*(*pathBaseEnemiga.begin()));

			It++;
		}
	}
	else{
		// Si las unidades estan suficientemente cerca de la posicion inicial, solamente vuelve al estado inicial
		if ((liderFormacion != NULL) && (liderFormacion->exists()) && (liderFormacion->isIdle()) && (liderFormacion->getPosition().getDistance(*(*(pathBaseEnemiga.begin()))) < 120.0)){
			estadoActual = ESPERANDO_CARGAR;
			ItPosiciones = pathBaseEnemiga.begin();
		}
	}
}


void CompaniaTransporte::reasignarLiderFormacion(){
	std::list<Unit*>::iterator It;
	liderFormacion = NULL;

	//-- recorro la lista de wraiths buscando alguna unidad existente y la asigno como nuevo lider de la formacion
	if (!listWraiths.empty()){
		It = listWraiths.begin();

		while (It != listWraiths.end()){
			if (((*It) != NULL) && ((*It)->exists())){
				liderFormacion = (*It);
				break;
			}

			It++;
		}
	}

	//-- si no encontre ningun wraith existente, recorro la lista de dropships buscando alguna unidad existente y la 
	//-- asigno como nuevo lider de la formacion
	if ((!listDropships.empty()) && (liderFormacion == NULL)){
		It = listDropships.begin();

		while (It != listDropships.end()){
			if (((*It) != NULL) && ((*It)->exists()) && (!(*It)->isLockedDown())){
				liderFormacion = (*It);
				break;
			}

			It++;
		}
	}
}


void CompaniaTransporte::recuadrarUnidades(){
	if ((liderFormacion != NULL) && (liderFormacion->exists()))
		Graficos::resaltarUnidad(liderFormacion, Colors::Green);	
}