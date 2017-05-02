#include <stdio.h>
#include <stdlib.h>
#include "server.h"
#include <enet/enet.h>
#include "SDL.h"
#include "logging.h"
//#include "SDL_net.h"

const Uint16 PORT = 8080;

ENetHost *server;

void server_ExitCleanUp()
{
	enet_host_destroy(server);
	enet_deinitialize();
}

int run_server(int argc, char** argv)
{
	ENetAddress address;
	ENetPeer client = { 0 };
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
		return 1;
	}

	ENetEvent event;
	int eventStatus = 1;

	Uint32 ticks = SDL_GetTicks();
	
	DebugLog("Listening to port*%u", PORT);
	while (1)
	{
		eventStatus = enet_host_service(server, &event, 1000);

		if (eventStatus > 0)
		{
			switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
					DebugLog("A new connection from %x:%u",
						event.peer->address.host,
						event.peer->address.port);
					client = *event.peer;
					break;

				case ENET_EVENT_TYPE_RECEIVE:
					printf("A packet of length %u containing %s was received from %s on channel %u.",
						event.packet->dataLength,
						event.packet->data,
						event.peer->data,
						event.channelID);
					/* Clean up the packet now that we're done using it. */
					enet_packet_destroy(event.packet);
					// Lets broadcast this message to all
					//enet_host_broadcast(server, 0, event.packet);
					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					DebugLog("%s disconnected.", event.peer->data);
						// Reset client's information
						event.peer->data = NULL;
					break;
			}
		}

		if (SDL_GetTicks() - ticks > 10)
		{
			char* message = "Hello";
			ticks = SDL_GetTicks();
			if (strlen(message) > 0)
			{
				ENetPacket *packet = enet_packet_create(message, strlen(message) + 1, ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(&client, 0, packet);
			}
		}
	}

	return 0;
}
