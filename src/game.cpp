#pragma once

#include "main.h"
#include "logging.h"
#include <limits>
#include <algorithm>
#include <cmath>
#undef max
#undef min

const int BALL_RADIUS = 3;
const int PADDLE_WIDTH = 6;
const int PADDLE_HEIGHT = 60;
const int PADDLE_OFFSET = 20;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

struct Vector2 { float x, y; };

const int GAME_PADDLE_COUNT = 2;

struct GameState
{
	Vector2 ballPosition;
	Vector2 ballVelocity;
	float paddleYs[GAME_PADDLE_COUNT];
};

// Check if a circle is colliding with a rectangle. This is done swept, 
// so it avoids any tunneling. Returns collision time between 0-1. If >= 1, not colliding.
// out Vector3 normal is the normal of the surface the circle collided with.
internal float SweptCircleWithRectangle(
	float circleX, float circleY, float radius, float velocityX, float velocityY,
	float rectX, float rectY, float rectWidth, float rectHeight,
	float *normalX, float *normalY
) {
	//TODO: maybe do some broad-phasing for performance gains.

	//Based on this: https://www.gamedev.net/articles/programming/general-and-gameplay-programming/swept-aabb-collision-detection-and-response-r3084/
	//Inv stands for inverse time.
	//Both specify how far away the closest edges of the objects are from each other
	float xInvEntry, yInvEntry;
	float xInvExit, yInvExit;

	//Find the distance between the objects
	// on the near and far sides for both x and y
	if (velocityX > 0.0f) {
		xInvEntry = (rectX - rectWidth / 2.f) - (circleX+ radius);
		xInvExit = (rectX + rectWidth / 2.f) - (circleX - radius);
	} else {
		xInvEntry = (rectX + rectWidth / 2.f) - (circleX - radius);
		xInvExit = (rectX - rectWidth / 2.f) - (circleX + radius);
	}

	if (velocityY > 0.0f) {
		yInvEntry = (rectY - rectHeight / 2.f) - (circleY + radius);
		yInvExit = (rectY + rectHeight / 2.f) - (circleY - radius);
	} else {
		yInvEntry = (rectY + rectHeight / 2.f) - (circleY - radius);
		yInvExit = (rectY - rectHeight / 2.f) - (circleY + radius);
	}

	//Actual entry/exit times
	float xEntry, yEntry;
	float xExit, yExit;
	//Find time of collision and time of leaving for each axis
	if (velocityX == 0.0f) {
		xEntry = -std::numeric_limits<float>::infinity();
		xExit = std::numeric_limits<float>::infinity();
	} else {
		xEntry = xInvEntry / velocityX;
		xExit = xInvExit / velocityX;
	}

	if (velocityY == 0.0f) {
		yEntry = -std::numeric_limits<float>::infinity();
		yExit = std::numeric_limits<float>::infinity();
	} else {
		yEntry = yInvEntry / velocityY;
		yExit = yInvExit / velocityY;
	}

	float entryTime = std::max(xEntry, yEntry);
	float exitTime = std::min(xExit, yExit);

	// if there was no collision
	if (entryTime > exitTime || xEntry < 0.0f && yEntry < 0.0f || xEntry > 1.0f || yEntry > 1.0f) {
		*normalX = 0.0f;
		*normalY = 0.0f;
		return 1.0f;
	} else { // if there was a collision
		// calculate normal of collided surface
		if (xEntry > yEntry) {
			if (xInvEntry < 0.0f) {
				*normalX = 1.0f;
				*normalY = 0.0f;
			} else {
				*normalX = -1.0f;
				*normalY = 0.0f;
			}
		} else {
			if (yInvEntry < 0.0f) {
				*normalX = 0.0f;
				*normalY = 1.0f;
			} else {
				*normalX = 0.0f;
				*normalY = -1.0f;
			}
		}

		// return the time of collision
		return entryTime;
	}

}

internal void update_state(float dt, GameState* gameState) {
	float velocityX = gameState->ballVelocity.x * dt;
	float velocityY = gameState->ballVelocity.y * dt;
	float collisionTime = 1.f;
	float normalX = 0, normalY = 0;
	

	int paddleIdx = velocityX < 0 ? 0 : 1;
	if(velocityX != 0) {
		float paddleX;
		if(paddleIdx) {
			paddleX = SCREEN_WIDTH - PADDLE_OFFSET;
		} else {
			paddleX = PADDLE_OFFSET;
		}

		collisionTime = SweptCircleWithRectangle(
			gameState->ballPosition.x , gameState->ballPosition.y, BALL_RADIUS, velocityX, velocityY,
			paddleX, gameState->paddleYs[paddleIdx], PADDLE_WIDTH, PADDLE_HEIGHT,
			&normalX, &normalY);
	}

	// Adjust for paddle collisions
	if(collisionTime < 1.f) {
		if(normalY != 0){
			velocityY *= collisionTime;

			velocityY *= -1;
			gameState->ballVelocity.y *= -1;
		}
		if(normalX != 0){
			velocityX *= collisionTime;

			velocityX *= -1;
			gameState->ballVelocity.x *= -1;

			float diffMax = (BALL_RADIUS + PADDLE_HEIGHT);
			float diff = gameState->ballPosition.y - gameState->paddleYs[paddleIdx];
			float diffAbs = std::abs(diff);
			float diffSign = diff / diffAbs;
			if(diffAbs <= diffMax){
				gameState->ballVelocity.y += (1 / (diff / diffMax)) * gameState->ballVelocity.y * 0.2f;
			}
		}

		gameState->ballVelocity.x *= 1.2f;
		gameState->ballVelocity.y *= 1.2f;
	}
	
	gameState->ballPosition.x += velocityX;
	gameState->ballPosition.y += velocityY;

	// Wall collision
	if(gameState->ballVelocity.x < 0){
		if(gameState->ballPosition.x < 0){
			gameState->ballPosition.x = 0;
			gameState->ballVelocity.x *= -1;
		}
	}
	if(gameState->ballVelocity.x > 0){
		if(gameState->ballPosition.x > SCREEN_WIDTH){
			gameState->ballPosition.x = SCREEN_WIDTH;
			gameState->ballVelocity.x *= -1;
		}
	}
	if(gameState->ballVelocity.y < 0){
		if(gameState->ballPosition.y < 0){
			gameState->ballPosition.y = 0;
			gameState->ballVelocity.y *= -1;
		}
	}
	if(gameState->ballVelocity.y > 0){
		if(gameState->ballPosition.y > SCREEN_HEIGHT){
			gameState->ballPosition.y = SCREEN_HEIGHT;
			gameState->ballVelocity.y *= -1;
		}
	}
}
