#include "Board.h"
#include <iostream>
#include <algorithm>
#include <allegro5\allegro_primitives.h>

#define BOARD_LEFT_X contentStartX + (contentWidth  - tileSide * B_W) / 2
#define BOARD_TOP_Y contentStartY + (contentHeight - tileSide * B_H) / 2

using namespace std;

Board::Board(ALLEGRO_DISPLAY * display, float startX, float startY, float width, float height)
	:contentBox(display, startX, startY, width, height, true)
{
	displaySectionType = BOARD;
	if (valid)
	{
		tileSide = min(contentWidth / B_W, contentHeight / B_H);
	}
}

Board::~Board()
{
}

float Board::getTileSide()
{
	return tileSide;
}

tileButton * Board::getTileButton(unsigned int tileX, unsigned int tileY)
{
	if (tileX <= B_W && tileY <= B_H)
	{
		list<button *>::iterator it;
		for (it = buttonList.begin(); it != buttonList.end() && ((tileButton *)(*it))->getTilePosition() != Point(tileX, tileY); it++) {}
		if (it == buttonList.end())	//si no aparecio un boton correspondiente a la tile con oordenadas tileX, tileY
		{
			return nullptr;
		}
		else	//si aparecio el boton correspondiente a la tile con coordenadas tileX, tileY
		{
			return (tileButton *)(*it);
		}
	}
	else
	{
		return nullptr;
	}
}

void Board::setTileButton(ALLEGRO_BITMAP * tileBmp, unsigned int tileX, unsigned int tileY)
{
	//crear un nuevo boton con la imagen recibida
	tileButton * tB = new tileButton(tileBmp, 
									BOARD_LEFT_X + tileX * tileSide, 
									BOARD_TOP_Y + tileY * tileSide, 
									tileSide, tileSide,
									Point(tileX, tileY));
	if (tB->isValid()) 
	{
		buttonList.push_back(tB);	//TODO: avisar si es que no se cargo
	}
}

void Board::drawContent()
{
	al_draw_filled_rectangle(BOARD_LEFT_X, BOARD_TOP_Y, BOARD_LEFT_X + B_W * tileSide, 
		BOARD_TOP_Y + B_H * tileSide, { 0,0,0,1 });	//TODO: placeholder del tablero

	for (std::list<button *>::iterator it = buttonList.begin(); it != buttonList.end(); it++)
	{
		(*it)->draw();
	}
}

void Board::resizeContent()
{
	tileSide = min(contentWidth / B_W, contentHeight / B_H);
	for (std::list<button *>::iterator it = buttonList.begin(); it != buttonList.end(); it++)
	{
		(*it)->resize(tileSide, tileSide);
	}
}
