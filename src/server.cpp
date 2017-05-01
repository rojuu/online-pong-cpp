#include <stdio.h>
#include <stdlib.h>
#include <enet/enet.h>
#include "SDL.h"
//#include "SDL_net.h"

const Uint16 PORT = 8080;

ENetAddress address;
ENetHost * server;

void ExitCleanUp()
{
	//SDLNet_TCP_Close(client);
	//SDLNet_TCP_Close(server);
	//SDLNet_Quit();
	enet_host_destroy(server);
	enet_deinitialize();
	SDL_Quit();
}

void LogSDLError(const char* message)
{
	fprintf(stderr, "%s, SDLError: %s\n", message, SDL_GetError());
}

int main(int argc, char** argv)
{
	atexit(ExitCleanUp);
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
		fprintf(stderr,
			"An error occurred while trying to create an ENet server host.\n");
		return 3;
	}

	return 0;
}

//void LogSDLNetError(const char* message)
//{
//	printf("%s, SDLNetError: %s\n", message, SDLNet_GetError());
//}
//
//TCPsocket server, client;
//


//int main(int argc, char** argv)
//{
//	atexit(ExitCleanUp);
//
//	IPaddress ip, *remoteip;
//	int length;
//	Uint32 ipaddr;
//
//	if (SDL_Init(0) == -1)
//	{
//		LogSDLError("SDL_Init");
//		return 1;
//	}
//
//	if (SDLNet_Init() == -1)
//	{
//		LogSDLError("SDLNet_Init");
//		return 2;
//	}
//	
//	if (SDLNet_ResolveHost(&ip, NULL, PORT) == -1)
//	{
//		LogSDLNetError("SDLNet_ResolveHost");
//		return 3;
//	}
//
//	server = SDLNet_TCP_Open(&ip);
//	if (!server)
//	{
//		LogSDLNetError("SDLNet_TCP_Open");
//		return 4;
//	}
//
//	bool quit = false;
//	printf("Listening to port *%u\n", PORT);
//	while (!quit)
//	{
//		client = SDLNet_TCP_Accept(server);
//		if (!client)
//		{
//			SDL_Delay(100);
//			continue;
//		}
//
//		remoteip = SDLNet_TCP_GetPeerAddress(client);
//		if (!remoteip)
//		{
//			LogSDLNetError("SDLNet_TCP_GetPeerAddress");
//			continue;
//		}
//
//		ipaddr = SDL_SwapBE32(remoteip->host);
//		printf("Accepted a connection from %d.%d.%d.%d port %hu\n",
//			ipaddr >> 24,
//			(ipaddr >> 16) & 0xff,
//			(ipaddr >> 8) & 0xff,
//			ipaddr & 0xff,
//			remoteip->port);
//
//		while (true)
//		{
//			char* buffer = (char*)malloc(sizeof(float));
//			length = SDLNet_TCP_Recv(client, buffer, sizeof(float));
//			if (length <= 0)
//			{
//				LogSDLNetError("SDLNet_TCP_Recv");
//				continue;
//			}
//
//			if (buffer)
//			{
//				float* time = (float*)malloc(sizeof(float));
//				*time = (float)*buffer;
//				if (time)
//				{
//					printf("Message: %f\n", time);
//				}
//				free(time);
//			}
//
//#if 0
//			if (message[0] == 'Q')
//			{
//				printf("Quitting on a Q received\n");
//				SDLNet_TCP_Close(client);
//				quit = true;
//				break;
//			}
//#endif
//		}
//	}
//
//	return 0;
//}
