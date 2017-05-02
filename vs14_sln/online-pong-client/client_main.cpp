#include "client.h"

void client_main_ExitCleanUp()
{
	enet_deinitialize();
	SDL_Quit();
}

int main(int argc, char** argv)
{
	atexit(client_main_ExitCleanUp);
	if (SDL_Init(0) == -1)
	{
		LogSDLError("SDL_Init");
		return 1;
	}

	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return 2;
	}

	printf("Nothing here\n");
	return 0;
}