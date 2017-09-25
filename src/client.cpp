#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include <enet/enet.h>
#include "SDL.h"
#include "logging.h"
#include "network.h"
#include "game.cpp"
#include <queue>


const char* HOST = "127.0.0.1";
const Uint16 PORT = 8080;
ENetHost * client;
ENetPeer * server;

SDL_Window* window;

bool runNetwork = true;
internal void ExitCleanUp(){
	runNetwork = false;
	enet_host_destroy(client);
	enet_deinitialize();
	SDL_DestroyWindow(window);
}


std::queue<ENetPacket*> SERVER_MESSAGE;

internal int network_thread(void *ptr) {
	//NETWORK MESSAGE
#if 1
	ENetEvent event;
	int eventStatus;
	while (runNetwork){
		eventStatus = enet_host_service(client, &event, 3000);
		if (eventStatus > 0) {
			switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT: {
					DebugLog("A new connection from %x:%u",
						event.peer->address.host,
						event.peer->address.port);
					break;
				}

				case ENET_EVENT_TYPE_RECEIVE: {
					SERVER_MESSAGE.push(event.packet);
					break;
				}

				case ENET_EVENT_TYPE_DISCONNECT: {
					DebugLog("%s disconnected.", event.peer->data);
					// Reset client's information
					event.peer->data = NULL;
					break;
				}
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

int run_client(int argc, char** argv) {
	ENetAddress address;
	atexit(ExitCleanUp);

	bool hasConnection = true;

	client = enet_host_create(NULL /* create a client host */,
		1 /* only allow 1 outgoing connection */,
		2 /* allow up 2 channels to be used, 0 and 1 */,
		57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
		14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);

	if (client == NULL) {
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

	assert(window != NULL);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED);
	assert(renderer != NULL);

	SDL_Thread *network_t;
	if (hasConnection) {
		network_t = SDL_CreateThread(network_thread, "NetworkThread", (void*)NULL);
	}

	// Events
	SDL_Event e;
	bool quit = false;

	// Drawing vars
	const int paddleWidth = PADDLE_WIDTH;
	const int paddleHeight = PADDLE_HEIGHT;
	const int paddleOffset = PADDLE_OFFSET;

	const int ballWidth = BALL_RADIUS * 2;
	const int ballHeight = ballWidth;

	SDL_Rect paddleRects[GAME_PADDLE_COUNT] = { 0 };
	SDL_Rect* paddleRect = 0;

	SDL_Rect ballRect = { 0 };

	GameState gameState;
	gameState.ballPosition = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	gameState.ballVelocity = { -100.f, 10.f };

	for (int i = 0; i < GAME_PADDLE_COUNT; ++i) {
		gameState.paddleYs[i] = SCREEN_HEIGHT / 2 - paddleHeight / 2;
	}

	//TODO: Figure out if we need so many timers
	//TODO: Figure out if SDL_GetPerformanceCounter is bad shit and
	// if we should use SDL_GetTicks() instead
	float CurrentTime = (float)SDL_GetPerformanceCounter() /
							(float)SDL_GetPerformanceFrequency();
	float StartTime = CurrentTime;
	float TimeFromStart = CurrentTime - StartTime;
	float LastTime = 0;
	float DeltaTime = 0;
	float NetworkRate = 60; //How many messages per second
	float TimeFromLastMessage = 1.f / NetworkRate;

//MAIN LOOP
#if 1
	while (!quit) {
		LastTime = CurrentTime;
		float PerformanceCounter = (float)SDL_GetPerformanceCounter();
		float PerformanceFrequency = (float)SDL_GetPerformanceFrequency();
		CurrentTime = PerformanceCounter / PerformanceFrequency;
		TimeFromStart = CurrentTime - StartTime;
		DeltaTime = (float)(CurrentTime - LastTime);
		TimeFromLastMessage += DeltaTime;

		if(DeltaTime > 0.1f) DeltaTime = 0.1f;

		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}

			if (e.type == SDL_MOUSEMOTION ||
				e.type == SDL_MOUSEBUTTONDOWN ||
				e.type == SDL_MOUSEBUTTONUP
			) {
				int x, y;
				SDL_GetMouseState(&x, &y);
				gameState.paddleYs[0] = y;
			}
		}

//NETWORKING
#if 1
		if(hasConnection && TimeFromLastMessage > 1.f/NetworkRate) {
			TimeFromLastMessage = 0;

			{
				ENetPacket* p = 0;
				while(!SERVER_MESSAGE.empty()) {
					p = SERVER_MESSAGE.front();
					SERVER_MESSAGE.pop();
				}

				if(p) {
					ServerMessage m = *(ServerMessage*)p->data;
					
					DebugLog("Ballposition %f", m.state.ballPosition.x);

					int opponentId = (m.clientId - 1) * -1;
					gameState.paddleYs[1] = m.state.paddleYs[opponentId];
					gameState.ballPosition = m.state.ballPosition;
					gameState.ballVelocity = m.state.ballVelocity;

					enet_packet_destroy(p);
				}
			}

			{
				ClientMessage m;
				m.i = gameState.paddleYs[0];
				Packet p;
				p.size = sizeof(m);
				p.message = &m;
				ENetPacket* packet = enet_packet_create(p.message, p.size,
					ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(server, 0, packet);
			}
		}
#endif
//NETWORKING END

//RENDERING
#if 1
		// Color defines
#define WHITE 0xFF, 0xFF, 0xFF, 0xFF
#define BLACK 0x00, 0x00, 0x00, 0xFF

		// Clear screen
		SDL_SetRenderDrawColor(renderer, BLACK);
		SDL_RenderClear(renderer);

		// Render paddles
		SDL_SetRenderDrawColor(renderer, WHITE);
		for (int i = 0; i < GAME_PADDLE_COUNT; i++) {
			float y = gameState.paddleYs[i];

			paddleRect = &paddleRects[i];
			paddleRect->w = paddleWidth;
			paddleRect->h = paddleHeight;
			paddleRect->x = paddleOffset - paddleWidth / 2
				+ (SCREEN_WIDTH - paddleOffset * 2) * i; //Enemy paddle on right
			paddleRect->y = y - paddleHeight / 2;

			SDL_RenderFillRect(renderer, paddleRect);
		}

		// Render ball
		SDL_SetRenderDrawColor(renderer, WHITE);

		{
			Vector2 ballPosition = gameState.ballPosition;
			ballRect.w = ballWidth;
			ballRect.h = ballHeight;
			ballRect.x = ballPosition.x - ballWidth / 2;
			ballRect.y = ballPosition.y - ballHeight / 2;

			SDL_RenderFillRect(renderer, &ballRect);
		}

		// Draw vertical line of dots
		SDL_SetRenderDrawColor(renderer, WHITE);
		for (int i = 2; i < SCREEN_HEIGHT; i += 4) {
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
