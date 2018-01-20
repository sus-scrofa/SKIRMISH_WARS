#include "Building.h"

#define HQ_CP			8
#define DEFAULT_CP		4

#define	CP_PER_TURN		2
#define RED_CP_PER_TURN	1	//reduced capture points

void setCapturePoints(building_t type, unsigned int& cp);


Building::Building(building_t type, player_t player)
{
	this->player = player;
	this->type = type;

	setCapturePoints(type, capturePoints);
}

Building :: ~Building() { ; }

player_t Building::getPlayer()
{
	return player;
}

bool Building::capture(bool isReduced, player_t player) 
{ 
	unsigned int lostCps;
	bool wasCaptured = false;

	if (player != this->player) {
		if (isReduced)
			lostCps = RED_CP_PER_TURN;
		else
			lostCps = CP_PER_TURN;
			
		if (lostCps < capturePoints) {
			capturePoints -= lostCps;
		}
		else {
			setCapturePoints(type, capturePoints);
			wasCaptured = true;
		}
	}

	return wasCaptured;
}

void Building::uncapture()
{
	setCapturePoints(type, capturePoints);
}

bool Building::isBeingCaptured()
{
	return bool(capturePoints);
}

void setCapturePoints(building_t type, unsigned int& cp)
{ 
	if (type == HEADQUARTERS)
		cp = HQ_CP;
	else
		cp = DEFAULT_CP;
}