#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Window* window;

struct Vector2 { float x, y; };

void LogSDLError(const char* message)
{
	printf("%s, SDLError: %s\n", message, SDL_GetError());
}

void ExitCleanUp()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char** argv)
{
	atexit(ExitCleanUp);
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LogSDLError("SDL couldn't be initialized");
		return 1;
	}

	window = SDL_CreateWindow("Pong",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		LogSDLError("Window couldn't be loaded");
		return 2;
	}
	
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		LogSDLError("Renderer couldn't be loaded");
		return 3;
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
		
		printf("FPS: %f\n", 1/DeltaTime);

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