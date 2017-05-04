#include <stdio.h>
#include <stdlib.h>
#include <enet/enet.h>
#include "SDL.h"
#include "logging.h"
#include "client.h"
#include "server.h"

static void ExitCleanUp()
{
	Logging_Deinit();
	enet_deinitialize();
	SDL_Quit();
}

int main(int argc, char** argv)
{
	atexit(ExitCleanUp);
	
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

	if (Logging_Init() == -1)
	{
		printf("Logging couldn't be initialized\n");
		return -1;
	}

	if (SDL_Init(0) == -1)
	{
		LogSDLError("SDL_Init");
		return -2;
	}

	if (enet_initialize() != 0)
	{
		DebugLog("An error occurred while initializing ENet.");
		if(isServer)
			return -3;
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
