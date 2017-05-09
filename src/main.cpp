#include <stdio.h>
#include <stdlib.h>
#include <enet/enet.h>
#include "SDL.h"
#include "logging.h"
#include "client.h"
#include "server.h"

static void ExitCleanUp()
{
	enet_deinitialize();
	SDL_Quit();
}

int main(int argc, char** argv)
{
	atexit(ExitCleanUp);

	if (SDL_Init(0) == -1)
	{
		LogSDLError("SDL_Init");
		return -1;
	}

	if (enet_initialize() != 0)
	{
		DebugLog("An error occurred while initializing ENet.");
		#ifdef SERVER
			return -2;
		#endif
	}

	int ret = -3;
#ifdef SERVER
		ret = run_server(argc, argv);
#endif
#ifdef CLIENT
		ret = run_client(argc, argv);
#endif

	return ret;
}
