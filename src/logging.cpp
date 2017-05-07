#include <stdio.h>
#include <stdarg.h> 
#include "SDL.h"
#include "logging.h"
#include "SDL_mutex.h"

void LogSDLError(const char* message)
{
	fprintf(stderr, "%s, SDLError: %s\n", message, SDL_GetError());
}

void DebugLog(const char* message, ...)
{
	va_list v;
	va_start(v, message);
	char buffer[1024];
	vsprintf(buffer, message, v);
	printf("%s\n", buffer);
	va_end(v);
}
