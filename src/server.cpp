#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <list>
#include "server.h"
#include <enet/enet.h>
#include "SDL.h"
#include "logging.h"
#include "network.h"
#include "game.cpp"

const Uint16 PORT = 8080;
const int NETWORK_RATE = 60;

SDL_mutex* PRINT_MUTEX;
//SDL_mutex* MESSAGE_MUTEX;

const int MAX_CLIENTS = 2;
internal ENetPeer* client[MAX_CLIENTS] = { 0 };
int CLIENT_COUNT = 0;

internal bool GAME_RUNNING = false;

ENetHost *server;

struct Message{
	ENetPacket* packet;
	int clientID;
};

std::queue<Message> CLIENT_MESSAGES;

internal int network_thread(void *ptr) {
	ENetEvent event;
	int eventStatus;

	DebugLog("Listening to port *%u", PORT);
	while (1) {
		eventStatus = enet_host_service(server, &event, 1.f/(float)NETWORK_RATE);
		if (eventStatus > 0) {
			switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT: {
					if (CLIENT_COUNT == MAX_CLIENTS) {
						break;
					}
					SDL_LockMutex(PRINT_MUTEX);
					DebugLog("A new connection from %x:%u",
						event.peer->address.host,
						event.peer->address.port);

					SDL_UnlockMutex(PRINT_MUTEX);
					client[CLIENT_COUNT++] = event.peer;

					if(CLIENT_COUNT == MAX_CLIENTS) {
						GAME_RUNNING = true;
					}
					break;
				}

				case ENET_EVENT_TYPE_RECEIVE: {
					//SDL_LockMutex(PRINT_MUTEX);
					// DebugLog(
					// 	"A packet of length %u received from ID: %u on channel %u.",
					// 	event.packet->dataLength,
					// 	event.peer->connectID,
					// 	event.channelID);
					//SDL_UnlockMutex(PRINT_MUTEX);
					Message m = { 0 };
					m.packet = event.packet;
					m.clientID =
						client[0]->connectID == event.peer->connectID ? 0 : 1;
					CLIENT_MESSAGES.push(m);
					break;
				}

				case ENET_EVENT_TYPE_DISCONNECT: {
					SDL_LockMutex(PRINT_MUTEX);
					DebugLog("%s disconnected.", event.peer->data);
					SDL_UnlockMutex(PRINT_MUTEX);
					// Reset client's information
					event.peer->data = NULL;
					break;
				}
			}
		}
	}
}

internal void ExitCleanUp() {
	SDL_DestroyMutex(PRINT_MUTEX);
	enet_host_destroy(server);
	enet_deinitialize();
}

int run_server(int argc, char** argv) {
	ENetAddress address;
	atexit(ExitCleanUp);

	SDL_Thread* network_t;
	SDL_Thread* packet_destroyer_t;

	PRINT_MUTEX = SDL_CreateMutex();

	/* Bind the server to the default localhost.     */
	/* A specific host address can be specified by   */
	/* enet_address_set_host (& address, "x.x.x.x"); */
	address.host = ENET_HOST_ANY;
	address.port = PORT;
	server = enet_host_create(&address	/* the address to bind the server host to */,
		32	/* allow up to 32 clients and/or outgoing connections */,
		2	/* allow up to 2 channels to be used, 0 and 1 */,
		0	/* assume any amount of incoming bandwidth */,
		0	/* assume any amount of outgoing bandwidth */);

	assert(server != NULL);

	network_t = SDL_CreateThread(network_thread, "NetworkThread", (void*)NULL);

	GameState gameState;
	gameState.ballPosition = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	gameState.ballVelocity = { -100.f, 10.f };

	for (int i = 0; i < GAME_PADDLE_COUNT; ++i) {
		gameState.paddleYs[i] = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;
		gameState.scores[i] = 0;
	}

	float CurrentTime = (float)SDL_GetPerformanceCounter() /
							(float)SDL_GetPerformanceFrequency();
	float StartTime = CurrentTime;
	float TimeFromStart = CurrentTime - StartTime;
	float LastTime = 0;
	float DeltaTime = 0;
	float NetworkRate = NETWORK_RATE; //How many messages per second
	float TimeFromLastMessage = 1.f / NetworkRate;

	for(;;) {
		LastTime = CurrentTime;
		CurrentTime = (float)SDL_GetPerformanceCounter() /
			(float)SDL_GetPerformanceFrequency();
		TimeFromStart = CurrentTime - StartTime;
		DeltaTime = (float)(CurrentTime - LastTime);
		TimeFromLastMessage += DeltaTime;

		if(DeltaTime > 0.1f) DeltaTime = 0.1f;

		if(GAME_RUNNING) {
			update_state(DeltaTime, &gameState);
		}

		if(TimeFromLastMessage > 1.f/NetworkRate) {
			TimeFromLastMessage = 0;
			//SDL_LockMutex(PRINT_MUTEX);
			//SDL_UnlockMutex(PRINT_MUTEX);
			
			Message messages[MAX_CLIENTS];
			bool hadMessages[MAX_CLIENTS] = { false };
			while(!CLIENT_MESSAGES.empty()) {
				Message m = CLIENT_MESSAGES.front();
				CLIENT_MESSAGES.pop();

				messages[m.clientID] = m;
				hadMessages[m.clientID] = true;
			}

			for(int i = 0; i < CLIENT_COUNT; ++i) {
				if(hadMessages[i]) {
					ClientMessage m = *(ClientMessage*)(messages[i].packet->data);
					gameState.paddleYs[i] = m.paddleY;
					enet_packet_destroy(messages[i].packet);
				}
			}

			for(int i = 0; i < CLIENT_COUNT; ++i) {
				ServerMessage m;
				m.state = gameState;
				m.clientId = i;

				Packet p;
				p.size = sizeof(m);
				p.message = &m;
				ENetPacket* packet = enet_packet_create(p.message, p.size,
					ENET_PACKET_FLAG_RELIABLE);

				enet_peer_send(client[i], 0, packet);
			}
		}
	}

	return 0;
}
