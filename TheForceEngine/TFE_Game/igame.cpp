#include "igame.h"
#include <TFE_DarkForces/darkForcesMain.h>
#include <TFE_Outlaws/outlawsMain.h>

namespace TFE
{
	IGame* createGame(GameID id)
	{
		IGame* game = nullptr;
		switch (id)
		{
			case Game_Dark_Forces:
			{
				game = new DarkForces();
			} break;
			case Game_Outlaws:
			{
				game = new Outlaws();
			} break;
		}
		if (game)
		{
			game->id = id;
		}

		return game;
	}

	void freeGame(IGame* game)
	{
		if (game)
		{
			game->exitGame();
			delete game;
		}
	}
}