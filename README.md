# online-pong-cpp
To build you need to setup your compiler so it can find SDL 2.0.5 and Enet 1.3.13

High level documentation of code:

main.cpp
Contains main entry point of the application. Looks for '-s' or '-server' command line argument. If found we start as server, otherwise as client. We can also build the application with the SERVER ifdef to build a server only version. We then either call 'run_server' or 'run_client' from server.cpp or client.cpp

client.cpp
Clients are just dummy clients that draw whatever state the server tells them. Each client is however responsible for their own paddle movement and sends the paddle location to the server. You can specify the servers IP address in the SERVERADDRESS file that should be located in the application's current directory. The client then updates the opponents paddle and ball positions based on the messages it recieves from the server.

server.cpp
Responsible for maintaining synced state between players. Each player sends their paddle position to the server. The server runs the main game logic based on the state it has. The server is responsible for the ball position and sends it to the clients.

game.cpp
Contains the actual game logic as just "pure" ish functions that operate on the gamestate. The server mainly calls these.
