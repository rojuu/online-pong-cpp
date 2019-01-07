all: main

main: src/*.cpp src/*.h
	mkdir -p bin
	g++ src/*.cpp -std=c++11 -w -Isrc -Ilibs/enet-1.3.13/include -Llibs/enet-1.3.13 -lenet -F/Library/Frameworks -framework SDL2 -o bin/online-pong