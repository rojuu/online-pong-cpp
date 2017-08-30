#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include <enet/enet.h>
#include "SDL.h"
#include "logging.h"
#include "client.h"
#include "server.h"

internal void ExitCleanUp() {
	enet_deinitialize();
	SDL_Quit();
}

int main(int argc, char **argv) {
	atexit(ExitCleanUp);

	bool isServer = false;

	if (argc > 1){
		for (int i = 0; i < argc; i++){
			if (strcmp(argv[i], "-s") == 0){
				isServer = true;
			}
			else if (strcmp(argv[i], "-server") == 0){
				isServer = true;
			}
		}
	}

#ifdef SERVER // Maybe we want to build a server separately at some point
	isServer = true;
#endif

	if (SDL_Init(0) == -1) {
		LogSDLError("SDL_Init");
		return -1;
	}

	if (enet_initialize() != 0) {
		DebugLog("An error occurred while initializing ENet.");
		return -2;
	}

	int ret = -3;
	if (isServer){
		ret = run_server(argc, argv);
	}
	else{
		ret = run_client(argc, argv);
	}

	return ret;
}
