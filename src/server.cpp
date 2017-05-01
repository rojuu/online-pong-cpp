#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_net.h"

const Uint16 PORT = 8080;

void LogSDLError(const char* message)
{
	printf("%s, SDLError: %s\n", message, SDL_GetError());
}

void LogSDLNetError(const char* message)
{
	printf("%s, SDLNetError: %s\n", message, SDLNet_GetError());
}

void ExitCleanUp()
{
	SDLNet_Quit();
	SDL_Quit();
}

int main(int argc, char** argv)
{
	atexit(ExitCleanUp);

	IPaddress ip, *remoteip;
	TCPsocket server, client;
	char message[1024];
	int len;
	Uint32 ipaddr;

	if (SDL_Init(0) == -1)
	{
		LogSDLError("SDL_Init");
		return 1;
	}

	if (SDLNet_Init() == -1)
	{
		LogSDLError("SDLNet_Init");
		return 2;
	}
	
	if (SDLNet_ResolveHost(&ip, NULL, PORT) == -1)
	{
		LogSDLNetError("SDLNet_ResolveHost");
		return 3;
	}

	server = SDLNet_TCP_Open(&ip);
	if (!server)
	{
		LogSDLNetError("SDLNet_TCP_Open");
		return 4;
	}

	bool quit = false;
	printf("Listening to port *%u\n", PORT);
	while (!quit)
	{
		client = SDLNet_TCP_Accept(server);
		if (!client)
		{
			SDL_Delay(100);
			continue;
		}

		remoteip = SDLNet_TCP_GetPeerAddress(client);
		if (!remoteip)
		{
			LogSDLNetError("SDLNet_TCP_GetPeerAddress");
			continue;
		}

		ipaddr = SDL_SwapBE32(remoteip->host);
		printf("Accepted a connection from %d.%d.%d.%d port %hu\n",
			ipaddr >> 24,
			(ipaddr >> 16) & 0xff,
			(ipaddr >> 8) & 0xff,
			ipaddr & 0xff,
			remoteip->port);

		while (true)
		{
			len = SDLNet_TCP_Recv(client, message, 1024);
			if (!len)
			{
				LogSDLNetError("SDLNet_TCP_Recv");
				continue;
			}

			printf("Received: %.*s\n", len, message);

			if (message[0] == 'Q')
			{
				printf("Quitting on a Q received\n");
				SDLNet_TCP_Close(client);
				quit = true;
				break;
			}
		}
	}

	return 0;
}
