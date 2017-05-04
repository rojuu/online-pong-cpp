#pragma once

int Logging_Init();
int Logging_Deinit();
void LogSDLError(const char* message);
void DebugLog(const char* message, ...);
