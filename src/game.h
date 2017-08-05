#pragma once

struct Vector2 { float x, y; };

const int GAME_PADDLE_COUNT = 2;

struct GameState
{
    // Range -1..1
    Vector2 ballPosition;
    // Range -1..1
	Vector2 ballVelocity;
    // Range -1..1
	float paddleYs[GAME_PADDLE_COUNT];
};
