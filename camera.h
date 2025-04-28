//-----------------------------------------------------------------------------------------
/**
 * \file       camera.h
 * \author     Šárka Prokopová
 * \date       2025/4/28
 * \brief      Evaluating positions for camera and switching between
 *			static views and free mode
 *
*/
//-----------------------------------------------------------------------------------------
#ifndef __CAMERA_POSITION_H
#define __CAMERA_POSITION_H
#include <time.h>
#include "pgr.h"
#include "render_stuff.h"

// booleans
// teleport speed for cam changes
extern float teleportSpeed;
// if it's first time run
extern bool firstTime;
// gamestate
extern GameState gameState;

/// <summary>
/// Handling camera, position and changes
/// </summary>
class cameraHandler {
public:
	cameraHandler() = default;
	glm::vec3 computeCenterView(Camera* camera, GameUniformVariables* gameUniVars, glm::vec3* cameraUpVector);
	void computeCameraPosition(GameState& gameState);
	void changePosition(bool* cameraPosition);
	void increaseCameraSpeed(Camera* camera, float deltaSpeed);
	void decreaseCameraSpeed(Camera* camera, float deltaSpeed);
	void turnCameraLeft(Camera* camera, float deltaAngle);
	void turnCameraRight(Camera* camera, float deltaAngle);
	void turnCamera(Camera* camera, float deltaAngle);
	void controlBorders(Camera* camera);
	bool vectorDif(glm::vec3 t, glm::vec3 b);
	void cameraHandler::moveCamUp(Camera* camera, float distance);
	void cameraHandler::moveCamDown(Camera* camera, float distance);
};

#endif 