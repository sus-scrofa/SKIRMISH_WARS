#pragma once
#include "button.h"

class tileButton : public button
{
public:

	tileButton(ALLEGRO_BITMAP * bmp, float leftX, float topY, float width, float height);
	~tileButton();

	virtual void draw();
};
