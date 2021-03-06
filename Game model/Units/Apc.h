 #pragma once
#include "../Unit.h"
#include <list>

class Apc : public Unit {
public:
	Apc(Point position, bool isMine);
	~Apc();

	virtual void nextTurn();
	virtual unsigned int restoreMPs();
	virtual unsigned int getTerrainMod(terrain_t t);
	virtual unsigned int getAttackMod(basicUnitType_t basicType, bool reduced);	//devuelve el mod, dependiendo de si la unit esta o no reducida

	bool canBoard(bool player);
	unsigned int canUnload();
	bool healLoadedUnits();
	bool startUnloading();

	void getPossibleUnloads(std::list<Action>& ul);

	bool load(Unit * u);		
	bool unload(Point whereTo);		//descarga siempre en orden inverso a como cargo

private:
	std::list<Unit *> loadedUnits;
};