#include <stdio.h>
#include <stdlib.h>
#include <list>
#include "server.h"
#include <enet/enet.h>
#include "SDL.h"
#include "logging.h"

const Uint16 PORT = 8080;

SDL_mutex* PRINT_MUTEX;
SDL_mutex* MESSAGE_MUTEX;

ENetHost *server;

struct Message
{
	ENetPacket* packet;
	int clientID;
};

std::list<Message> CLIENT_MESSAGES;

static int network_thread(void *ptr)
{
	const size_t MAX_CLIENTS = 2;
	int clientCount = 0;
	ENetPeer client[MAX_CLIENTS] = { 0 };
	ENetEvent event;
	int eventStatus = 1;

	DebugLog("Listening to port *%u", PORT);
	while (1)
	{
		eventStatus = enet_host_service(server, &event, 1000);
		if (eventStatus > 0)
		{
			switch (event.type)
			{
				case ENET_EVENT_TYPE_CONNECT:
				{	if (clientCount == MAX_CLIENTS) break;
					SDL_LockMutex(PRINT_MUTEX);
					DebugLog("A new connection from %x:%u",
						event.peer->address.host,
						event.peer->address.port);

					SDL_UnlockMutex(PRINT_MUTEX);
					client[clientCount++] = *event.peer;
					break;
				}

				case ENET_EVENT_TYPE_RECEIVE:
				{
					SDL_LockMutex(PRINT_MUTEX);
					DebugLog("A packet of length %u containing %s was received from %u:%u on channel %u.",
						event.packet->dataLength,
						event.packet->data,
						event.peer->address.host,
						event.peer->address.port,
						event.channelID);
					
					SDL_UnlockMutex(PRINT_MUTEX);
					Message m = { 0 };
					m.packet = event.packet;
					m.clientID = client[0].connectID == event.peer->connectID ? 0 : 1;
					SDL_LockMutex(MESSAGE_MUTEX);
					CLIENT_MESSAGES.push_back(m);
					SDL_UnlockMutex(MESSAGE_MUTEX);
					break;
				}

				case ENET_EVENT_TYPE_DISCONNECT:
				{
					SDL_LockMutex(PRINT_MUTEX);
					DebugLog("%s disconnected.", event.peer->data);
					SDL_UnlockMutex(PRINT_MUTEX);
					// Reset client's information
					event.peer->data = NULL;
					break;
				}
			}
		}
	}
}

static void ExitCleanUp()
{
	SDL_DestroyMutex(MESSAGE_MUTEX);
	SDL_DestroyMutex(PRINT_MUTEX);
	enet_host_destroy(server);
	enet_deinitialize();
}

int run_server(int argc, char** argv)
{
	ENetAddress address;
	atexit(ExitCleanUp);

	SDL_Thread* network_t;

	PRINT_MUTEX = SDL_CreateMutex();
	MESSAGE_MUTEX = SDL_CreateMutex();

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

	network_t = SDL_CreateThread(network_thread, "NetworkThread", (void*)NULL);

	while(1)
	{
		SDL_Delay(4000);

		SDL_LockMutex(PRINT_MUTEX);
		DebugLog("Updating");

		SDL_LockMutex(MESSAGE_MUTEX);
		if(CLIENT_MESSAGES.size() > 0)
		{
			for(auto msg : CLIENT_MESSAGES)
			{
				char* data = (char*)msg.packet->data;
				DebugLog(data);
				enet_packet_destroy(msg.packet);
			}
			CLIENT_MESSAGES.clear();
		}
		SDL_UnlockMutex(MESSAGE_MUTEX);

		SDL_UnlockMutex(PRINT_MUTEX);
	}

#if 0
	if (SDL_GetTicks() - ticks > 10)
	{
		char* message = "Hello";
		ticks = SDL_GetTicks();
		if (strlen(message) > 0)
		{
			ENetPacket *packet = enet_packet_create(message, strlen(message)+1
				, ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(&client, 0, packet);
		}
	}
#endif

	return 0;
}
