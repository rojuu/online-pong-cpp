#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> 
#include <enet/enet.h>
#include "SDL.h"
//#include "SDL_net.h"

const char* HOST = "127.0.0.1";
const Uint16 PORT = 8080;
//TCPsocket socket;
ENetHost * client;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Window* window;

struct Vector2 { float x, y; };

void LogSDLError(const char* message)
{
	printf("%s, SDLError: %s\n", message, SDL_GetError());
}

//void LogSDLNetError(const char* message)
//{
//	printf("%s, SDLNetError: %s\n", message, SDLNet_GetError());
//}

void DebugLog(const char* message, ...)
{
	va_list v;
	va_start(v, message);
	char buffer[1024];
	vsprintf(buffer, message, v);
	printf("%s\n", buffer);
	va_end(v);
}

void ExitCleanUp()
{
	//SDLNet_TCP_Close(socket);
	//SDLNet_Quit();
	enet_host_destroy(client);
	enet_deinitialize();
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char** argv)
{
	atexit(ExitCleanUp);

	/*int length;
	IPaddress ip;*/
	bool hasConnection = true;
	
	float pos = 0.134f;

	//INIT
	if (SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		LogSDLError("SDL_Init");
		return 1;
	}

	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		hasConnection = false;
	}
	
	client = enet_host_create(NULL /* create a client host */,
		1 /* only allow 1 outgoing connection */,
		2 /* allow up 2 channels to be used, 0 and 1 */,
		57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
		14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);

	if (client == NULL)
	{
		fprintf(stderr,
			"An error occurred while trying to create an ENet client host.\n");
		hasConnection = false;
	}

	/*if (SDLNet_Init() == -1)
	{
		LogSDLError("SDLNet_Init");
		hasConnection = false;
	}

	if (SDLNet_ResolveHost(&ip, HOST, PORT) == -1)
	{
		LogSDLNetError("SDLNet_ResolveHost");
		hasConnection = false;
	}

	socket = SDLNet_TCP_Open(&ip);
	if (!socket)
	{
		LogSDLNetError("SDLNet_TCP_Open");
		hasConnection = false;
	}*/

	window = SDL_CreateWindow("Pong",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		LogSDLError("SDL_CreateWindow");
		return 2;
	}
	
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		LogSDLError("SDL_CreateRenderer");
		return 3;
	}

	//INIT VARIABLES
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
	float NetworkRate = 66; //How many messages per second
	float TimeFromLastMessage = 1/NetworkRate;

	//MAIN LOOP
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

		//NETWORK MESSAGE
		if (hasConnection && TimeFromLastMessage > 1/NetworkRate)
		{
			//Do stuff
		}

		////NETWORK MESSAGE
		//if (hasConnection && TimeFromLastMessage > 1/NetworkRate)
		//{
		//	DebugLog("TimeFromStart: %f", TimeFromStart);
		//	char* buffer = (char*)malloc(sizeof(float));
		//	*buffer = (char)TimeFromStart;
		//	int result = SDLNet_TCP_Send(socket, buffer, sizeof(float));
		//	if (result < sizeof(float))
		//	{
		//		LogSDLNetError("SDLNet_TCP_Send");
		//	}
		//}

		//RENDERING
		{
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
		} //END RENDERING
	}
	
	return 0;
}