//-----------------------------------------------------------------------------------------
/**
 * \file       camera.cpp
 * \author     Šárka Prokopová
 * \date       2022/4/28
 * \brief      Evaluating positions for camera and switching between 
 *			static views and free mode
 *
*/
//-----------------------------------------------------------------------------------------
#include "camera.h"

// positions for static camera and their center 
// first position
glm::vec3 firstStatPosition = glm::vec3(1.0f, 4.0f, 1.5f);
glm::vec3 firstStatCenter = glm::vec3(0.0f, 2.0f, 2.0f);
// second position
glm::vec3 secStatPosition = glm::vec3(1.0f, 4.0f, 5.1f);
glm::vec3 secStatCenter = glm::vec3(0.0f, -0.0f, -0.3f);
// actual position in free mode
glm::vec3 positionInFreeMode = glm::vec3(0.0f, 4.0f, 1.5f);
glm::vec3 centerInFreeMode = glm::vec3(0.0f, 2.0f, 1.5f);
glm::vec3 actualPosInFreeMode = glm::vec3(0.0f, 4.0f, 3.0f);
glm::vec3 actualCenterInFreeMode = glm::vec3(0.0f, 4.0f, 3.0f);
// what position is active
glm::vec3 activeStaticPosition = firstStatPosition;
glm::vec3 activeStaticCenter = firstStatCenter;
// to compute teleport
glm::vec3 actualPosition = firstStatPosition;
glm::vec3 actualCenter = firstStatCenter;

// switch between two different static positions
void cameraHandler::changePosition(bool* cameraPosition) {
	*cameraPosition = !*cameraPosition;
	switch (*cameraPosition) {
	case 0:
		activeStaticPosition = firstStatPosition;
		activeStaticCenter = firstStatCenter;
		break;

	case 1:
		activeStaticPosition = secStatPosition;
		activeStaticCenter = secStatCenter;
		break;
	}

}

// computing center view
glm::vec3  cameraHandler::computeCenterView(Camera* camera, GameUniformVariables* gameUniVars, glm::vec3* cameraUpVector) {
	glm::vec3 cameraCenter;
	glm::vec3 cameraViewDirection = camera->direction;

	glm::vec3 rotationAxis = glm::cross(cameraViewDirection, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 cameraTransform = glm::rotate(glm::mat4(1.0f), -glm::radians(camera->elevationAngle), rotationAxis);

	*cameraUpVector = glm::vec3(cameraTransform * glm::vec4(*cameraUpVector, 0.0f));
	cameraViewDirection = glm::vec3(cameraTransform * glm::vec4(cameraViewDirection, 0.0f));

	gameUniVars->reflectorPositionLocation = camera->position;
	gameUniVars->reflectorDirectionLocation = cameraViewDirection;

	cameraCenter = camera->position + cameraViewDirection;

	return cameraCenter;
}

// compute difference
bool cameraHandler::vectorDif(glm::vec3 t, glm::vec3 b) {
	float pressize = 0.01;
	float resX = t.x - b.x;
	float resY = t.y - b.y;
	float resZ = t.z - b.z;
	if (resX <= pressize && resY <= pressize && resZ <= pressize) {
		return true;
	}
	return false;
}

// compute teleport between positions
void cameraHandler::computeCameraPosition(GameState& gameState) {

	if (!gameState.freeCameraMode) {
		actualPosition = mix(actualPosition, activeStaticPosition, teleportSpeed);
		actualCenter = mix(actualCenter, activeStaticCenter, teleportSpeed);
	
	}

}

// increase speed of movement
void cameraHandler::increaseCameraSpeed(Camera* camera, float deltaSpeed = CAMERA_SPEED_INCREMENT) {

	if (camera->speed <= 0.1f)
		deltaSpeed = 0.75f;
	camera->speed =
		std::min(camera->speed + deltaSpeed, CAMERA_SPEED_MAX);
	
	
}

// decrease speed of movement
void cameraHandler::decreaseCameraSpeed(Camera* camera, float deltaSpeed = CAMERA_SPEED_INCREMENT) {

	if (abs(camera->speed) <= 0.2f)
		deltaSpeed = 0.75f;
	camera->speed =
		std::max(camera->speed - deltaSpeed, -CAMERA_SPEED_MAX);


}

// turning camera left
void cameraHandler::turnCameraLeft(Camera* camera, float deltaAngle) {

	camera->viewAngle += deltaAngle;

	if (camera->viewAngle > 360.0f)
		camera->viewAngle -= 360.0f;

	float angle = glm::radians(camera->viewAngle);

	camera->direction.x = cos(angle);
	camera->direction.y = sin(angle);

}

// turning camera right
void cameraHandler::turnCameraRight(Camera* camera, float deltaAngle) {

	camera->viewAngle -= deltaAngle;

	if (camera->viewAngle < 0.0f)
		camera->viewAngle += 360.0f;

	float angle = glm::radians(camera->viewAngle);

	camera->direction.x = cos(angle);
	camera->direction.y = sin(angle);
}

// turning camera
void cameraHandler::turnCamera(Camera* camera, float deltaAngle) {

	camera->viewAngle += deltaAngle;

	if (camera->viewAngle < 0.0f)
		camera->viewAngle += 360.0f;
	if (camera->viewAngle > 360.0f)
		camera->viewAngle -= 360.0f;

	float angle = glm::radians(camera->viewAngle);

	camera->direction.x = cos(angle);
	camera->direction.y = sin(angle);
}

// control borders for camera
void cameraHandler::controlBorders(Camera* camera) {
	if (camera->position.x < -2.8) {
		camera->position.x = -2.8;
	}
	if (camera->position.x > 2.8) {
		camera->position.x = 2.8;
	}
	if (camera->position.y < -2.8) {
		camera->position.y = -2.8;
	}
	if (camera->position.y > 2.8) {
		camera->position.y = 2.8;
	}
}