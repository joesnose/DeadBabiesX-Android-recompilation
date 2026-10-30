#include "main.h"
#include "menu.h"
#include "game.h"

void __cdecl main()
{
	XEngine->XInit();

	//different game state/menu stuff will go here eventually

	XEngine->PlayVideo("d:\\data\\movies\\logo.wmv");
	XEngine->PlayVideo("d:\\data\\movies\\intro.wmv");
	

start:

	//Menu.Init();

	//do 
	//{
	//}while (Menu.Update());

	Game.Init();

	while (Game.Update())
	{
	}

	/*
	delete &Menu;
	delete &Game;

	Menu=(*new CMenu);
	Game=(*new CGame);
	*/

	goto start;
}