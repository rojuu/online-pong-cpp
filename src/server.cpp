#include <stdio.h>
#include <stdlib.h>
#include "server.h"
#include <enet/enet.h>
#include "SDL.h"
#include "logging.h"
//#include "SDL_net.h"

const Uint16 PORT = 8080;

ENetAddress address;
ENetHost * server;

void server_ExitCleanUp()
{
	//SDLNet_TCP_Close(client);
	//SDLNet_TCP_Close(server);
	//SDLNet_Quit();
	enet_host_destroy(server);
	enet_deinitialize();
}

int run_server(int argc, char** argv)
{
	atexit(server_ExitCleanUp);

	/* Bind the server to the default localhost.     */
	/* A specific host address can be specified by   */
	/* enet_address_set_host (& address, "x.x.x.x"); */
	address.host = ENET_HOST_ANY;
	address.port = PORT;
	server = enet_host_create(&address /* the address to bind the server host to */,
		32      /* allow up to 32 clients and/or outgoing connections */,
		2      /* allow up to 2 channels to be used, 0 and 1 */,
		0      /* assume any amount of incoming bandwidth */,
		0      /* assume any amount of outgoing bandwidth */);

	if (server == NULL)
	{
		DebugLog("An error occurred while trying to create an ENet server host.");
		return 3;
	}

	DebugLog("Listening to port*%u", PORT);
	while (1)
	{
		SDL_Delay(100);
	}

	return 0;
}
