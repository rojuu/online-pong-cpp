#pragma once

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

struct Vector2 { float x, y; };

const int GAME_PADDLE_COUNT = 2;

struct GameState
{
    Vector2 ballPosition;
	Vector2 ballVelocity;
	int paddleYs[GAME_PADDLE_COUNT];
};
