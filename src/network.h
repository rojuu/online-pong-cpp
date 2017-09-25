#pragma once

#include "game.cpp"

struct Packet{
	void* message;
	Uint32 size;
};

struct ServerMessage{
	GameState state;
	int clientId;
};

struct ClientMessage{
	float paddleY;
};
