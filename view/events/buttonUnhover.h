#pragma once

#include "../../Game model/FSM/SkirmishEvent.h"
#include "../button.h"

class buttonUnhover : public SkirmishEvent
{
public:
	buttonUnhover(button * b);
	~buttonUnhover();

	button * getButton();

private:
	button * b;
};

