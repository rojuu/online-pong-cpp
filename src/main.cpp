#include <stdio.h>
#include <stdlib.h>
#include <enet/enet.h>
#include "SDL.h"
#include "logging.h"
#include "client.h"
#include "server.h"

void main_ExitCleanUp()
{
	enet_deinitialize();
	SDL_Quit();
}

int main(int argc, char** argv)
{
	atexit(main_ExitCleanUp);

	bool isServer = false;

	if (argc > 1)
	{
		for (size_t i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], "-s") == 0)
			{
				isServer = true;
			}
			else if(strcmp(argv[i], "-server") == 0)
			{
				isServer = true;
			}
		}
	}

	if (SDL_Init(0) == -1)
	{
		LogSDLError("SDL_Init");
		return -1;
	}

	if (enet_initialize() != 0)
	{
		DebugLog("An error occurred while initializing ENet.");
		if(isServer)
			return -2;
	}

	int ret = 0;
	if (isServer)
	{
		ret = run_server(argc, argv);
	}
	else
	{
		ret = run_client(argc, argv);
	}

	return ret;
}
