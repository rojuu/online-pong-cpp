#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_net.h"

const Uint16 PORT = 3000;
TCPsocket socket;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Window* window;

struct Vector2 { float x, y; };

int IpStringToNumber(const char* DottedQuad, Uint32 *    IpAddr)
{
	Uint32            byte3;
	Uint32            byte2;
	Uint32            byte1;
	Uint32            byte0;
	char              dummyString[2];

	/* The dummy string with specifier %1s searches for a non-whitespace char
	* after the last number. If it is found, the result of sscanf will be 5
	* instead of 4, indicating an erroneous format of the ip-address.
	*/
	if (sscanf_s(DottedQuad, "%u.%u.%u.%u%1s",
		&byte3, &byte2, &byte1, &byte0, dummyString) == 4)
	{
		if ((byte3 < 256)
			&& (byte2 < 256)
			&& (byte1 < 256)
			&& (byte0 < 256)
			)
		{
			*IpAddr = (byte3 << 24)
				+ (byte2 << 16)
				+ (byte1 << 8)
				+ byte0;

			return 0;
		}
	}

	return 1;
}

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
	SDLNet_TCP_Close(socket);
	SDLNet_Quit();
	SDL_DestroyWindow(window);
	SDL_Quit();
	system("pause");
}

int main(int argc, char** argv)
{
	char message[1024];
	int len;
	IPaddress ip;
	if (IpStringToNumber("127.0.0.1", &ip.host) != 0)
	{
		printf("IpAddress parsing failed.\n");
		return 1;
	}

	atexit(ExitCleanUp);
	if (SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		LogSDLError("SDL_Init");
		return 2;
	}

	if (SDLNet_Init() == -1)
	{
		LogSDLError("SDLNet_Init");
		return 3;
	}

	if (SDLNet_ResolveHost(&ip, NULL, PORT) == -1)
	{
		LogSDLNetError("SDLNet_ResolveHost");
		return 4;
	}

	socket = SDLNet_TCP_Open(&ip);
	if (!socket)
	{
		LogSDLNetError("SDLNet_TCP_Open");
		return 5;
	}

	window = SDL_CreateWindow("Pong",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		LogSDLError("SDL_CreateWindow");
		return 6;
	}
	
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		LogSDLError("SDL_CreateRenderer");
		return 7;
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

	Uint64 CurrentTime = SDL_GetPerformanceCounter();
	Uint64 LastTime = 0;
	double DeltaTime = 0;
	while (!quit)
	{
		LastTime = CurrentTime;
		CurrentTime = SDL_GetPerformanceCounter();
		DeltaTime = (double)((CurrentTime - LastTime) / (double)SDL_GetPerformanceFrequency());

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
		
		//printf("FPS: %f\n", 1/DeltaTime);

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