#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include <enet/enet.h>
#include "SDL.h"
#include "logging.h"
#include "packet.h"


const char* HOST = "127.0.0.1";
const Uint16 PORT = 8080;
ENetHost * client;
ENetPeer * server;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Window* window;

struct Vector2 { float x, y; };

bool runNetwork = true;
static void ExitCleanUp()
{
	runNetwork = false;
	enet_host_destroy(client);
	enet_deinitialize();
	SDL_DestroyWindow(window);
}

static int network_thread(void *ptr)
{
	//NETWORK MESSAGE
#if 1
	ENetEvent event;
	int eventStatus;
	while (runNetwork)
	{
		eventStatus = enet_host_service(client, &event, 3000);
		if (eventStatus > 0)
		{
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				DebugLog("A new connection from %x:%u",
					event.peer->address.host,
					event.peer->address.port);
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				DebugLog("Message from server : %s", event.packet->data);
				// Lets broadcast this message to all
				// enet_host_broadcast(client, 0, event.packet);
				enet_packet_destroy(event.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				DebugLog("%s disconnected.", event.peer->data);
				// Reset client's information
				event.peer->data = NULL;
				break;
			}
		}
		/*if (strlen(message) > 0) {
		ENetPacket *packet = enet_packet_create(message, strlen(message) + 1, 
			ENET_PACKET_FLAG_RELIABLE);

		enet_peer_send(server, 0, packet);
		}*/
	}
#endif
	//NETWORK MESSAGE END
	return 0;
}

int run_client(int argc, char** argv)
{
	ENetAddress address;
	atexit(ExitCleanUp);
	SDL_Thread *network_t;

	bool hasConnection = true;
	
	client = enet_host_create(NULL /* create a client host */,
		1 /* only allow 1 outgoing connection */,
		2 /* allow up 2 channels to be used, 0 and 1 */,
		57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
		14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);

	if (client == NULL)
	{
		DebugLog("An error occurred while trying to create an ENet client host.");
		hasConnection = false;
	}

	enet_address_set_host(&address, "localhost");
	address.port = PORT;

	server = enet_host_connect(client, &address, 2, 0);

	if (server == NULL) {
		DebugLog("No available peers for initializing an ENet connection");
		hasConnection = false;
	}

	window = SDL_CreateWindow("Pong",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		LogSDLError("SDL_CreateWindow");
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		LogSDLError("SDL_CreateRenderer");
		return 2;
	}

	if (hasConnection)
	{
		network_t = SDL_CreateThread(network_thread, "NetworkThread", (void*)NULL);
	}
	
	SDL_Event e;
	bool quit = false;

	const int paddleWidth = 6;
	const int paddleHeight = 60;
	const int paddleOffset = 20;

	const int ballWidth = 6;
	const int ballHeight = ballWidth;

	const int paddleCount = 2;
	SDL_Rect paddleRects[paddleCount] = { 0 };
	SDL_Rect* paddleRect = 0;

	SDL_Rect ballRect = { 0 };
	Vector2 ballPosition = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	Vector2 ballVelocity = { -10, 0 };

	int paddleYs[paddleCount];
	for (size_t i = 0; i < paddleCount; i++)
	{
		paddleYs[i] = SCREEN_HEIGHT / 2 - paddleHeight / 2;
	}

	float CurrentTime = (float)SDL_GetPerformanceCounter() / (float)SDL_GetPerformanceFrequency();
	float StartTime = CurrentTime;
	float TimeFromStart = CurrentTime - StartTime;
	float LastTime = 0;
	float DeltaTime = 0;
	float NetworkRate = 1; //How many messages per second
	float TimeFromLastMessage = 1 / NetworkRate;

//MAIN LOOP
#if 1
	while (!quit)
	{
		LastTime = CurrentTime;
		CurrentTime = (float)SDL_GetPerformanceCounter() / (float)SDL_GetPerformanceFrequency();
		TimeFromStart = CurrentTime - StartTime;
		DeltaTime = (float)(CurrentTime - LastTime);
		TimeFromLastMessage += DeltaTime;

		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}

			if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)
			{
				int x, y;
				SDL_GetMouseState(&x, &y);
				paddleYs[0] = y;
			}
		}

#if 1
		if(hasConnection && TimeFromLastMessage > 1/NetworkRate)
		{
			TimeFromLastMessage = 0;
			Packet p;
			char* m = "HELLLOOO";
			p.size = sizeof("HELLLOOO");
			p.message = m;
			ENetPacket* packet = enet_packet_create(p.message, p.size, ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(server, 0, packet);
		}
#endif

//RENDERING
#if 1
		// Color defines
#define WHITE 0xFF, 0xFF, 0xFF, 0xFF
#define BLACK 0x00, 0x00, 0x00, 0xFF

		//Clear screen
		SDL_SetRenderDrawColor(renderer, BLACK);
		SDL_RenderClear(renderer);

		//Render paddles
		SDL_SetRenderDrawColor(renderer, WHITE);
		for (size_t i = 0; i < paddleCount; i++)
		{
			paddleRect = &paddleRects[i];
			paddleRect->w = paddleWidth;
			paddleRect->h = paddleHeight;
			paddleRect->x = paddleOffset - paddleWidth / 2 + (SCREEN_WIDTH - paddleOffset * 2) * i;
			paddleRect->y = paddleYs[i] - paddleHeight / 2;

			SDL_RenderFillRect(renderer, paddleRect);
		}

		//Render ball
		SDL_SetRenderDrawColor(renderer, WHITE);
		{
			ballRect.w = ballWidth;
			ballRect.h = ballHeight;
			ballRect.x = ballPosition.x - ballWidth / 2;
			ballRect.y = ballPosition.y - ballHeight / 2;

			SDL_RenderFillRect(renderer, &ballRect);
		}

		//Draw vertical line of dots
		SDL_SetRenderDrawColor(renderer, WHITE);
		for (int i = 2; i < SCREEN_HEIGHT; i += 4)
		{
			SDL_RenderDrawPoint(renderer, SCREEN_WIDTH / 2, i);
		}

		SDL_RenderPresent(renderer);
#endif	
//END RENDERING
	}
#endif
//MAIN LOOP END
	return 0;
}
