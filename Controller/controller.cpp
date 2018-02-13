#include "controller.h"

Controller::Controller(network * net)
{
}

Controller::~Controller()
{
}

bool Controller::sendNameIs(std::string name)
{
	nameIs package(name);
	return sendPackage(&package);

}

bool Controller::sendMapIs(std::string mapName, unsigned char checksum)
{
	mapIs package(mapName, checksum);
	return sendPackage(&package);
}

bool Controller::sendMove(unsigned char rowOrigin, unsigned char columnOrigin, unsigned char rowDestination, unsigned char columnDestination)
{
	move package(rowOrigin, columnOrigin, rowDestination, columnDestination);
	return sendPackage(&package);
}

bool Controller::sendPurchase(std::string ejercitoID, unsigned char row, unsigned char column)
{
	purchase package(ejercitoID, row, column);
	return sendPackage(&package);
}

bool Controller::sendAttack(unsigned char dice, unsigned char rowOrigin, unsigned char columnOrigin, unsigned char rowDestination, unsigned char columnDestination)
{
	attack package(dice, rowOrigin, columnOrigin, rowDestination, columnDestination);
	return sendPackage(&package);
}

bool Controller::sendOneBytePackage(unsigned char type)
{
	genericPackage * package=NULL;
	bool state = false;
	switch (type)
	{
	case ACK:
		package = new ack;
		break;
	case NAME:
		package = new name;
		break;
	case YOU_START:
		package = new youStart;
		break;
	case I_START:
		package = new iStart;
		break;
	case PASS:
		package = new pass;
		break;
	case YOU_WON:
		package = new youWon;
		break;
	case PLAY_AGAIN:
		package = new playAgain;
		break;
	case GAME_OVER:
		package = new gameOver;
		break;
	case ERROR:
		package = new error;
		break;
	case QUIT:
		package = new quit;
		break;

		
	default:
		break;
	}

	if (package != NULL)
	{
		state = sendPackage(package);
		delete package;
	}
	return state;
}

void Controller::startPlayTimer()
{
}

void Controller::resetPlayTimer()
{
}

void Controller::stopPlayTimer()
{
}

bool Controller::sendPackage(genericPackage * package)
{
	bool state = false;
	if ((net->getCurrentState()) == CONNECTED)
	{
		state = (net->sendData(package->getBuff(), package->getSizeBuff()));
	}
	return state;
}
