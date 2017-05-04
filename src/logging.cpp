#include <stdio.h>
#include <stdarg.h> 
#include "SDL.h"
#include "logging.h"
#include "SDL_mutex.h"

SDL_mutex* PRINT_MUTEX;

int Logging_Init()
{
	PRINT_MUTEX = SDL_CreateMutex();
	if (PRINT_MUTEX)
		return 0;
	else
		return -1;
}

int Logging_Deinit()
{
	SDL_DestroyMutex(PRINT_MUTEX);
	return 0;
}

void LogSDLError(const char* message)
{
	while (!SDL_LockMutex(PRINT_MUTEX)) SDL_Delay(10);
	fprintf(stderr, "%s, SDLError: %s\n", message, SDL_GetError());
	SDL_UnlockMutex(PRINT_MUTEX);
}

void DebugLog(const char* message, ...)
{
	while (!SDL_LockMutex(PRINT_MUTEX)) SDL_Delay(10);
	va_list v;
	va_start(v, message);
	char buffer[1024];
	vsprintf(buffer, message, v);
	printf("%s\n", buffer);
	va_end(v);
	SDL_UnlockMutex(PRINT_MUTEX);
}
