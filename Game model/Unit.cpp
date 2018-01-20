#include "Unit.h"
#include "Units/Apc.h"
#include <iterator>

Map * Unit::map = nullptr;

#define CANT_REACH	-1

unsigned int getDamage(int initDamage, terrain_t t, unsigned int diceRoll);



Unit::Unit(unit_t type, Point position, bool isMine, unsigned int cost, unsigned int defense, unsigned int minRange,
	unsigned int maxRange) : cost(cost), type(type), isMine(isMine), defense(defense), minRange(minRange), maxRange(maxRange)
{
	if (type >= 0 && type < N_TYPES && Map::isInMap(position)) { //solo puedo verificar si el punto se fue de rango si ya tengo el mapa
		this->position = position;
		healthPoints = MAX_HP;
		state = IDLE;
	}
	else {
		this->position.x = B_W;
		this->position.y = B_H;
		this->healthPoints = 0;
		state = POST_ACTIVE;
		movingPoints = 0;
	}
}

Unit::~Unit()
{
}

unit_t Unit::getType()
{
	return type;
}

unitType_t Unit::getBasicType()
{
	if (FIRST_W <= type && type < FIRST_F)
		return WHEEL;

	else if (FIRST_F <= type && type < FIRST_T)
		return FOOT;

	else if (FIRST_T <= type && type < N_TYPES)
		return TREAD;

	else
		return N_B_TYPES;
}

void Unit::setMap(Map * map)
{
	Unit::map = map;
}

bool Unit::isAlive()
{
	return !(bool(healthPoints));
}

player_t Unit::getPlayer()
{
	return (isMine? USER : OPPONENT);
}

Point Unit::getPosition()
{
	return position;
}

void Unit::heal()
{
	if (healthPoints > 0) { //no curo unidades muertas!
		healthPoints += HEAL_HP;

		if (healthPoints > MAX_HP)
			healthPoints = MAX_HP;

		if (type == APC) {
			((Apc *)this)->healLoadedUnits();
		}
	}
}

bool Unit::move(Action a)
{
	if (map != nullptr && !map->isInMap(a.whereTo) || a.mps > movingPoints
		|| !map->updateUnitPos(this, a.whereTo, a.type == LOAD))
		return false;
	
	position = a.whereTo;
	movingPoints -= a.mps;
	return true;
}

int Unit::attack(Attack a, unsigned int diceRoll)
{
	//if (!map->isInMap(a.whereTo) || a.mps > movingPoints || !map->isInMap(a.target) || !map->hasUnit(a.target)
	//	|| !(isMine ^ map->getPlayer(a.target) == USER) || !map->updateUnitPos(this, a.whereTo) )
	//	return false;

	if (map != nullptr && !map->isInMap(a.target) || !map->hasUnit(a.target) || 
					!(isMine ^ (map->getPlayer(a.target) == USER)) ||!move(a))
		return false;

	Unit * enemy = map->getUnit(a.target);
	terrain_t t = map->getTerrain(a.target);
	int initDamage = getAttackMod(enemy->getBasicType()) - enemy->defense; //aca ya se tiene en cuenta si esta reducido o no
	
	enemy->healthPoints -= getDamage(initDamage, t, diceRoll);

	return true;
}

void Unit::getPossibleActions(std::list<Action>* moves, std::list<Attack> * attacks)
{
	if (state >= ATTACKING || map == nullptr)
		return;				//despues de atacar no puedo hacer nada

	bool newAttacks[B_H * B_W];	//para saber que casillas ya tienen registrado un ataque
	memset(newAttacks, 0, sizeof(bool)*B_H*B_W);	// pongo todo en false: todavia no ataque nada

	if (state <= MOVING)
		getPossibleMoves(moves, position, position, movingPoints);

	if (state == IDLE) {
		getPossibleAttacks(newAttacks, attacks, position); //estos ataques cuestan 0 mps (por defecto)
	}

	if (state == MOVING && maxRange == 1) { //los de rango 1 pueden atacar despues de moverse
		for (std::list<Action>::iterator it = moves->begin(); it != moves->end(); it++) {
			if (it->type == MOVE) { // me fijo de no atacar desde un APC
				getPossibleAttacks(newAttacks, attacks, it->whereTo, it->mps);
				// estos cuentan tantos mps como moverse a la casilla desde donde se hacen
			}
		}
	}
	
	if (state == UNLOADING) {
		((Apc *)this)->getPossibleUnloads(moves);
	}
		
}

void Unit::getPossibleMoves(std::list<Action>* moves, Point start, Point curr, unsigned int mps)
{
	if (map == nullptr || !map->isInMap(curr))
		return;

	unsigned int mod = UINT_MAX;
	action_t actionType = N_ACTIONS; //si esto queda asi es que no hay ninguna accion valida

	if (start != curr) { // me fijo si puedo venir a esta casilla, y cuanto me cuesta

		if (!map->hasFog(curr, getPlayer())) { //solo puedo ir a casillas que puedo ver
			if (map->hasUnit(curr)) {
				if ((getBasicType() == FOOT && map->getUnit(curr)->type == APC
					&& ((Apc *)(map->getUnit(curr)))->canBoard(isMine))) { 
					actionType = LOAD; //solo si tiene un apc mio y soy foot
				}
			}
			else {
				actionType = MOVE; //si no tiene unidad y la puedo ver puede ser un move
			}
		}

		if (actionType != N_ACTIONS) {
			if (map->hasBuilding(curr)) {
				mod = getTerrainMod(ROAD);	//los edificios cuentan como road siempre!
			}
			else {
				mod = getTerrainMod(map->getTerrain(curr));
			}
		}

	}
	else {
		mod = 0;	//no necesito mps para moverme a la casilla donde estoy (para que no haya errores en la primera llamada)

		if (type == APC && ((Apc *)this)->nLoadedUnits() != 0) {
			actionType = UNLOAD; //si soy un apc mi misma casilla es unload
		}
	}


	if (mod <= mps) { //puedo moverme a esta casilla
		mps -= mod; //a los mps que me quedan les tengo que restar lo que cuesta este ultimo paso
		Action action(actionType, curr, movingPoints - mps);
		//mps que cuesta esta accion: los que tiene la unidad, menos los que me quedan

		bool newTile = true;	//para saber si ya habia un camino a esta tile o no
		if (start != curr && (actionType == LOAD || actionType == MOVE)) { //no calculo camino optimo si la distancia es 0

			for (std::list<Action>::iterator it = moves->begin(); it != moves->end() && newTile == true; it++) {
				if (it->whereTo == curr) {
					newTile = false;	// tile repetida!

					if (it->mps > action.mps) {	//si el camino es mas corto, lo actualizo
						it->mps = action.mps;	//(en it estan los que cuestan, movingPoints son los que tiene la unidad
					}									//y mps los que todavia me quedan para gastar de esos)
				}
			}
		}

		if (newTile == true && actionType != N_ACTIONS) { //solo a�ado un item a la lista si es una tile nueva
			moves->push_back(action);
		}

		if (action.type != LOAD) {
			//veo a que tiles puedo ir desde aca
			curr.x -= 1;				// a la izquierda
			getPossibleMoves(moves, start, curr, mps);

			curr.x += 2;				//a la derecha
			getPossibleMoves(moves, start, curr, mps);

			curr.x -= 1; curr.y -= 1;	//arriba
			getPossibleMoves(moves, start, curr, mps);

			curr.y += 2;				//abajo
			getPossibleMoves(moves, start, curr, mps);
		}
	}
}

void Unit::getPossibleAttacks(bool * newAttacks, std::list<Attack>* attacks, Point curr, unsigned int movingPoints)
{
	if (map == nullptr)
		return;

	//voy a recorrer todo el mapa y fijarme cuales unidades enemigas estan dentro de mi rango
	for (unsigned int i = 0; i<B_H; i++) {
		for (unsigned int j = 0; j<B_W; j++) {
			Point p(i, j);
			unsigned int dist = curr.orthogonalDistanceFrom(p); 

			if (minRange <= dist && dist <= maxRange && map->hasUnit(p)
				&& (isMine ^ (map->getPlayer(p) == USER)) && !map->hasFog(p, getPlayer())) {
				//me fijo si esta casilla la puedo atacar con menos MPs desde otro lugar o no

				if (newAttacks[i*B_W + j] == true) { 
					newAttacks[i*B_W + j] = false; //el ataque ya existe: me fijo si puedo mejorar los MPs
					for (std::list<Attack>::iterator it = attacks->begin(); it != attacks->end() && newAttacks[i*B_W + j] == false; it++) {
						if (it->whereTo == curr) {
							newAttacks[i*B_W + j] = true;
							if (movingPoints < it->mps) {
								it->mps = movingPoints;
							}
						}
					}
				}
				else {
					attacks->push_back(Attack(p, curr, movingPoints));
					newAttacks[i*B_W + j] = true; // ataque nuevo
				}
			}
		}
	}
}

bool Unit::isReduced()
{
	if (healthPoints <= REDUCED_HP && healthPoints > 0)
		return true;
	else
		return false;
}

//bool Unit::move(Point target)
//{
//	if (map == nullptr || map->isInMap(target) || state >= ATTACKING || movingPoints == 0 || healthPoints == 0)
//		return false;	//la accion no puede realizarse sin importar lo que haya en el terreno
//
//	bool valid = false;
//	int mpsRemaining = canReach(target, position, movingPoints);
//
//	if (mpsRemaining != CANT_REACH && map->updateUnitPos(this, target) == true) {
//		position = target;
//		movingPoints = mpsRemaining;
//		valid = true;
//	}
//
//	return valid;
//}
//
//int Unit::canReach(Point target, Point curr, unsigned int mps)
//{
//	int maxMps = CANT_REACH;
//	unsigned int mod = terrainMod[map->getTerrain(curr)];
//
//	if (mod <= mps) { //puedo moverme a esta casilla
//		maxMps = mps - mod; //ahora lo maximo que me puede quedar es menos lo que me costo llegar aca
//
//		if (target != curr && maxMps != 0) { //si todavia no llegue, veo si puedo llegar de alguna manera
//			Point newPos(curr.x - 1, curr.y); // a la izquierda
//			if (map->isInMap(newPos) && !map->hasUnit(newPos))
//				maxMps = std::max(canReach(target, newPos, maxMps), maxMps);
//
//			newPos = Point(curr.x + 1, curr.y); // a la derecha
//			if (map->isInMap(newPos) && !map->hasUnit(newPos))
//				maxMps = std::max(canReach(target, newPos, maxMps), maxMps);
//
//			newPos = Point(curr.x, curr.y - 1); // abajo
//			if (map->isInMap(newPos) && !map->hasUnit(newPos))
//				maxMps = std::max(canReach(target, newPos, maxMps), maxMps);
//
//			newPos = Point(curr.x, curr.y + 1); // arriba
//			if (map->isInMap(newPos) && !map->hasUnit(newPos))
//				maxMps = std::max(canReach(target, newPos, maxMps), maxMps);
//		}
//	}
//
//	return maxMps;
//}

unsigned int getDamage(int initDamage, terrain_t t, unsigned int diceRoll)
{
	return rand()%3; //we. obviamente esto no es asi
}