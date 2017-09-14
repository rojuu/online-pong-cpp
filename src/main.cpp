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

	assert(SDL_Init(0) != -1);
	assert(enet_initialize() == 0);

	int ret = 0;
	if (isServer){
		ret = run_server(argc, argv);
	}
	else{
		ret = run_client(argc, argv);
	}

	return ret;
}
