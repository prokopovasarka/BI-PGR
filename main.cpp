//-----------------------------------------------------------------------------------------
/**
 * \file       main.cpp
 * \author     Šárka Prokopová
 * \date       2025/3/3
 * \brief      house by the lake
 *
 *  Interactive scene where user can move around and click on objects.
 *  Scene has changing weather - cloudy weather after allowing, lights, 
*	interactive duck animation, explosion and Maxwell cat move.
*/
//-----------------------------------------------------------------------------------------

#include <iostream>
#include <time.h>
#include <list>
#include "pgr.h"
#include "gameEngine.h"
#include "data.h"
#include "water.h"
#include "spline.h"


gameEngine* gameHandler = new gameEngine();
waterBufferMaker* waterFBOHandler;
renderObjects gameEngine::renderHandler;
cameraHandler gameEngine::camHandler;
splineHandler gameEngine::splineFucHandler;

float loadingBarWidth = 0.0f; //var for loading bar

bool duckAnimation; //if animation is on or off

//---------------------------------------------------------CREATE OBJECTS--------------------------------------------------------------------
//create duck
void gameEngine::duckHandler::createDuck() {
	Object* newDuck = new Object;

	newDuck->position = m_loadProps["duck"].position;
	newDuck->direction = m_loadProps["duck"].front;
	newDuck->size = m_loadProps["duck"].size;

	newDuck->destroyed = false;
	newDuck->startTime = gameState.elapsedTime;
	newDuck->currentTime = newDuck->startTime;

	gameObjects.duck = newDuck;
}

// create Maxwell object
void gameEngine::maxwellHandler::createMaxwell() {
	Object* newMaxwell = new Object;

	newMaxwell->position = m_loadProps["maxwell"].position;
	newMaxwell->direction = m_loadProps["maxwell"].front;
	newMaxwell->origPos = m_loadProps["maxwell"].position;
	newMaxwell->size = m_loadProps["maxwell"].size;
	newMaxwell->angle = m_loadProps["maxwell"].size;

	newMaxwell->destroyed = false;
	newMaxwell->startTime = gameState.elapsedTime;
	newMaxwell->currentTime = newMaxwell->startTime;

	gameObjects.maxwellObj = newMaxwell;
}

//----------------------------------------------------------INTERACTION WITH OBJECTS------------------------------------------------------
void createExplosion(glm::vec3 position) {
	Explosion* newExplosion = new Explosion;

	newExplosion->speed = 0.0f;
	newExplosion->destroyed = false;

	newExplosion->startTime = gameState.elapsedTime;
	newExplosion->currentTime = newExplosion->startTime;

	newExplosion->size = 0.5;
	newExplosion->direction = glm::vec3(0.0f, 1.0f, 0.0f);

	newExplosion->frameDuration = 0.02f;
	newExplosion->frames = 84;

	newExplosion->position = glm::vec3(position.x+0.6, position.y-0.3, position.z);
	newExplosion->end = newExplosion->startTime + newExplosion->frames * newExplosion->frameDuration;

	explosions.push_back(newExplosion);
}

void controlExplosion(Explosion* explosion) {

	if (explosion->currentTime > explosion->end)
		explosion->destroyed = true;
}

// updating duck
void gameEngine::duckHandler::updateDuck(float elapsedTime) {
	if (duckAnimation) {
		gameObjects.duck->startTime += elapsedTime;
		float a = gameObjects.duck->startTime;
		a *= 0.6;
		gameObjects.duck->position = splineFucHandler.evaluateClosedCurve(duckCurvePoints, duckCurvePointsTotal, a); //catmull-Rom
		glm::vec3 newDirection = -glm::normalize(splineFucHandler.evalClosedCurveFirstDev(duckCurvePoints, duckCurvePointsTotal, a));

		gameObjects.duck->direction = mix(gameObjects.duck->direction, newDirection, 0.1f);
	}
}

//update maxwell rotation
void gameEngine::maxwellHandler::updateMaxwell(float deltaTime) {

	float heightOffset = 1.5f;  // height
	float range = 0.2f;  // range

	// movement
	float newHeight = range * sin(gameObjects.maxwellObj->currentTime) + heightOffset;

	// newposition
	gameObjects.maxwellObj->position.z = newHeight;

	// update time
	gameObjects.maxwellObj->currentTime += deltaTime;
}

// setup lightning structures to pass them by uniform
void gameEngine::setupLights() {
	// sphere - point light
	sphereLight.ambient = glm::vec3(1.0);
	sphereLight.diffuse = glm::vec3(1.0, 1.0, 0.5f);
	sphereLight.specular = glm::vec3(1.0);
	// sun - directional light
	sun.ambient = glm::vec3(0.2f);
	sun.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	sun.specular = glm::vec3(1.0);
	// camera reflector - spot light
	cameraReflector.ambient = glm::vec3(0.2f);
	cameraReflector.diffuse = glm::vec3(1.0);
	cameraReflector.specular = glm::vec3(1.0);
	cameraReflector.spotCosCutOff = 0.95f;
	cameraReflector.spotExponent = 0.0;

}

// changing light 
void gameEngine::evalLightIntensity() {
	const float cycleDuration = 60.0f; // 120 second loop

	float timeInCycle = fmod(gameState.elapsedTime, cycleDuration);

	float phase;
	if (timeInCycle < cycleDuration / 2.0f) {
		phase = timeInCycle / (cycleDuration / 2.0f);  // 0 -> 1
	}
	else {
		phase = (cycleDuration - timeInCycle) / (cycleDuration / 2.0f); // 1 -> 0
	}
	phase = pow(phase, 2.0f);
	// change light
	gameUniVars.lightIntensity = glm::mix(1.0f, -1.2f, phase);

}

// changing intensity of point light
void gameEngine::changePointLight() {

	float max_light = 11.0f;
	float min_light = 0.0;

	gameUniVars.pointLightIntensity += 5.3;
	if (gameUniVars.pointLightIntensity > max_light) {
		gameUniVars.pointLightIntensity = min_light;
	}
}

//-------------------------------------------------------------RESTART GAME------------------------------------------------------------------------------
void gameEngine::restartGame() {

	gameState.elapsedTime = 0.001f * (float)glutGet(GLUT_ELAPSED_TIME); // milliseconds => seconds

	if (gameObjects.camera == NULL) {
		gameObjects.camera = new Camera;
	}

	gameObjects.camera->position = glm::vec3(-0.0f, 5.3f, 1.3f);
	gameObjects.camera->viewAngle = 90.0f; // degrees
	gameObjects.camera->direction = glm::vec3(cos(glm::radians(gameObjects.camera->viewAngle)), sin(glm::radians(gameObjects.camera->viewAngle)), 0.0f);
	gameObjects.camera->speed = 0.0f;
	gameObjects.camera->destroyed = false;
	gameObjects.camera->startTime = gameState.elapsedTime;
	gameObjects.camera->currentTime = gameObjects.camera->startTime;
	gameObjects.camera->elevationAngle = 0.0f;

	// animated obj
	duckAnimation = true;
	m_duckHandler.createDuck();
	m_maxwellHandler.createMaxwell();

	// exit from free camera
	if (gameState.freeCameraMode == true) {
		gameState.freeCameraMode = false;
		glutPassiveMotionFunc(NULL);
	}
	// reset key map
	for (int i = 0; i < KEYS_COUNT; i++) {
		gameState.keyMap[i] = false;
	}

	gameState.gameOver = false;
	// setting all uniform variables
	gameUniVars.pointLightIntensity = 0.0f;
	gameUniVars.useLighting = true;
	gameUniVars.spotLight = false;
}



//-------------------------------------------------------------------DRAW GEOMETRY AND STUFF-------------------------------------------------------
void gameEngine::screenHandler::drawWindowContents( bool drawWater ) {
	// setup parallel projection
	glm::mat4 orthoProjectionMatrix = glm::ortho(
		-SCENE_WIDTH, SCENE_WIDTH,
		-SCENE_HEIGHT, SCENE_HEIGHT,
		-10.0f * SCENE_DEPTH, 10.0f * SCENE_DEPTH
	);
	glm::mat4 orthoViewMatrix;

	orthoViewMatrix = glm::lookAt(
		actualPosition, // position
		actualCenter, // center
		glm::vec3(0.0f, 0.0f, 1.0f)  // up
	);

	glm::mat4 viewMatrix = orthoViewMatrix;
	glm::mat4 projectionMatrix = orthoProjectionMatrix;
	// compute to know where is the center - first time after restart
	if (firstTime) {
		glm::vec3 cameraUpVector = glm::vec3(0.0f, 0.0f, 1.0f);
		camHandler.computeCenterView(gameObjects.camera, &gameUniVars, &cameraUpVector);
		firstTime = false;
	}
	if (gameState.freeCameraMode) {
		glm::vec3 cameraPosition = gameObjects.camera->position;
		glm::vec3 cameraUpVector = glm::vec3(0.0f, 0.0f, 1.0f);
		camHandler.controlBorders(gameObjects.camera);
		glm::vec3 cameraCenter = camHandler.computeCenterView(gameObjects.camera, &gameUniVars, &cameraUpVector);

		viewMatrix = glm::lookAt(
			cameraPosition,
			cameraCenter,
			cameraUpVector
		);
	}
	if (gameState.curveMotion) { // set camera for curve motion
		glm::vec3 cameraPosition = gameObjects.camera->position;
		glm::vec3 cameraUpVector = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 cameraCenter = camHandler.computeCenterView(gameObjects.camera, &gameUniVars, &cameraUpVector);

		viewMatrix = glm::lookAt(
			cameraPosition,
			cameraCenter,
			cameraUpVector
		);
	}

	projectionMatrix = glm::perspective(glm::radians(60.0f), (float)gameState.windowWidth / (float)gameState.windowHeight, 0.1f, 10.0f);

	glUseProgram(shaderProgram.program);
	glUniform1f(shaderProgram.timeLocation, gameState.elapsedTime);
	
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 2, -1);

	renderHandler.getDrawHandler().drawDuck(viewMatrix, projectionMatrix, shaderProgram, gameUniVars, m_loadProps["duck"], gameObjects.duck->position, gameObjects.duck->direction); // draw duck, v=2
	glStencilFunc(GL_ALWAYS, 3, -1);
	renderHandler.getDrawHandler().drawMaxwell(viewMatrix, projectionMatrix, shaderProgram, gameUniVars, m_loadProps["maxwell"], gameObjects.maxwellObj->position, gameObjects.maxwellObj->direction); //draw maxwell
	glDisable(GL_STENCIL_TEST);
	renderHandler.getDrawHandler().drawEverything(viewMatrix, projectionMatrix, drawWater, m_loadProps, waterFBOHandler); // draw almost all meshes

	//update loading bar
	renderHandler.getDrawHandler().drawBar(viewMatrix, projectionMatrix, loadingBarWidth);


	glDisable(GL_STENCIL_TEST);
	// create explosion
	std::list <Explosion*> ::iterator it;
	for (int i = 0; i < 4; i++) {
		for (it = explosions.begin(); it != explosions.end(); ++it) {
			renderHandler.getDrawHandler().drawExplosion(viewMatrix, projectionMatrix, *it);
		};
	};

	glUseProgram(0);
}

// Called to update the display. You should call glutSwapBuffers after all of your
// rendering to display what you rendered.
void gameEngine::screenHandler::displayCallback() {
	GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;

	waterFBOHandler->bindReflectionFrameBuffer();
	glClear(mask);
	glEnable(GL_CLIP_DISTANCE0);
	gameEngine::screenHandler::drawWindowContents(false);
	waterFBOHandler->unbindCurrentFrameBuffer();

	glDisable(GL_CLIP_DISTANCE0);
	waterFBOHandler->bindRefractionFrameBuffer();
	glClear(mask);
	glEnable(GL_CLIP_DISTANCE1);
	gameEngine::screenHandler::drawWindowContents(false);
	waterFBOHandler->unbindCurrentFrameBuffer();

	glClear(mask);
	glDisable(GL_CLIP_DISTANCE1);
	gameEngine::screenHandler::drawWindowContents(true);
	glutSwapBuffers();
}

void gameEngine::screenHandler::reshapeCallback(int newWidth, int newHeight) {

	gameState.windowWidth = newWidth;
	gameState.windowHeight = newHeight;

	glViewport(0, 0, (GLsizei)newWidth, (GLsizei)newHeight);
}

// control collisions with tower
bool collisionController(glm::vec3 position){
	glm::vec3 objPos = glm::vec3(0.0f, 0.02f, -0.7f);
	float sizeX = 0.8;
	float sizeY = 0.8;
	bool colX = (position.x + 0.1 >= objPos.x && position.x <= objPos.x + sizeX) || (position.x - 0.1 <= objPos.x && position.x >= objPos.x - sizeX);
	bool colY = (position.y + 0.1 >= objPos.y && position.y <= objPos.y + sizeY) || (position.y - 0.1 <= objPos.y && position.y >= objPos.y - sizeY);
	return colX && colY;
}

// update animations, light etc. on map
void gameEngine::updateObjects(float elapsedTime) {

	float timeDelta = elapsedTime - gameObjects.camera->currentTime;
	gameObjects.camera->currentTime = elapsedTime;

	m_duckHandler.updateDuck(timeDelta); // new position for duck
	if (gameState.isCloudy)
		gameHandler->evalLightIntensity(); // change light intensity with according to game time
	else 
		gameUniVars.lightIntensity = 0.9f; //else eval intensity back to normal

	loadingBarWidth = glm::clamp(gameObjects.camera->speed / 10.0f, 0.0f, 1.0f); //clam curr speed

	currentColor = mix(currentColor, day, timeDelta);

	if (gameState.curveMotion == true) { // show spline motion
		gameObjects.camera->startTime += timeDelta;
		float a = gameObjects.camera->startTime;
		a *= 0.1;
		gameObjects.camera->position = splineFucHandler.evaluateMovementCurve(curveData, curveSize, a);
		gameObjects.camera->direction = glm::normalize(splineFucHandler.evalMovementCurveFirstDev(curveData, curveSize, a));
	}
	else {// normal camera static/free
		glm::vec3 new_position = gameObjects.camera->position + timeDelta * gameObjects.camera->speed * gameObjects.camera->direction;
		if (!collisionController(new_position)) {
			gameObjects.camera->position = new_position;
		}
		else {
			gameObjects.camera->speed = 0.0f;
		}
		gameObjects.camera->position = new_position;
	
		camHandler.controlBorders(gameObjects.camera);
		camHandler.computeCameraPosition(gameState);
	}
	// update explosion
	std::list <Explosion*>::iterator it;
	it = explosions.begin();
	while (it != explosions.end()) {
		Explosion* explosion = (Explosion*)(*it);
		explosion->currentTime = elapsedTime;

		controlExplosion(explosion);

		if (explosion->destroyed == true) {
			it = explosions.erase(it);
		}
		else {
			++it;
		}
	}

	//blow cat after explosion
	if (gameState.blowMaxwell) {
		m_maxwellHandler.updateMaxwell(timeDelta); //new movement for maxwell
	}
	
	// set color
	glClearColor(currentColor.x, currentColor.y, currentColor.z, 1);
}


//-----------------------------------------------------------UPDATE SCREEN--------------------------------------------------------------

void gameEngine::screenHandler::timerCallback(int) {

	// update scene time
	gameState.elapsedTime = 0.001f * (float)glutGet(GLUT_ELAPSED_TIME); // milliseconds => seconds

	// call appropriate actions according to the currently pressed keys in key map
	// (combinations of keys are supported but not used in this implementation)
	if (gameState.keyMap[KEY_RIGHT_ARROW] == true ) {
		camHandler.turnCameraRight(gameObjects.camera, CAMERA_VIEW_ANGLE_DELTA);
	}
	if (gameState.keyMap[KEY_LEFT_ARROW] == true ) {
		camHandler.turnCameraLeft(gameObjects.camera, CAMERA_VIEW_ANGLE_DELTA);
	}
	if (gameState.keyMap[KEY_UP_ARROW] == true && gameState.freeCameraMode ) {
		camHandler.increaseCameraSpeed(gameObjects.camera, CAMERA_SPEED_INCREMENT);

	}
	if (gameState.keyMap[KEY_DOWN_ARROW] == true && gameState.freeCameraMode ) {
		camHandler.decreaseCameraSpeed(gameObjects.camera, CAMERA_SPEED_INCREMENT);

	}
	// update objects in the scene
	gameHandler->updateObjects(gameState.elapsedTime);
	// set timeCallback next invocation
	glutTimerFunc(33, timerCallback, 0);

	glutPostRedisplay();
}

// passive mouse callback - camera motion
void gameEngine::screenHandler::passiveMouseMotionCallback(int mouseX, int mouseY) {
	if (!gameState.curveMotion) {
		if (mouseY != gameState.windowHeight / 2) {

			float cameraElevationAngleDelta = 0.5f * (mouseY - gameState.windowHeight / 2);
			if (fabs(gameObjects.camera->elevationAngle + cameraElevationAngleDelta) < CAMERA_ELEVATION_MAX_Y)
				gameObjects.camera->elevationAngle += cameraElevationAngleDelta;


			glutWarpPointer(gameState.windowWidth / 2, gameState.windowHeight / 2);

			glutPostRedisplay();
		}
		if (mouseX != gameState.windowWidth / 2) {

			float cameraElevationAngleDelta = 0.5f * (mouseX - gameState.windowWidth / 2);
			camHandler.turnCamera(gameObjects.camera, -cameraElevationAngleDelta);

			glutWarpPointer(gameState.windowWidth / 2, gameState.windowHeight / 2);

			glutPostRedisplay();
		}
	}
}

// interaction with mouse
void gameEngine::screenHandler::mouseCallback(int buttonPressed, int buttonState, int mouseX, int mouseY) {

	// interaction with left button down
	if ((buttonPressed == GLUT_LEFT_BUTTON) && (buttonState == GLUT_DOWN)) {
		int objectID = 0;
		glReadPixels(mouseX, gameState.windowHeight - mouseY - 1,
			1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &objectID
		);
		if (objectID == 1) { // sphere
			gameHandler->changePointLight();
		} if (objectID == 2) { // duck animation
			duckAnimation = !duckAnimation;
		} if (objectID == 3) { // boat
			gameState.blowMaxwell = !gameState.blowMaxwell;
			if (gameState.blowMaxwell) {
				gameObjects.maxwellObj->currentTime = 0.0;
				createExplosion(m_loadProps["maxwell"].position);
			}
			else {
				gameObjects.maxwellObj->position = gameObjects.maxwellObj->origPos;
			}
		}
	}
	if ((buttonPressed == GLUT_RIGHT_BUTTON) && (buttonState == GLUT_DOWN)) {
		int objectID = 0;
		glReadPixels(mouseX, gameState.windowHeight - mouseY - 1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &objectID
		);
	}
}

//-----------------------------------------------------------------------KEYBOARD SETTINGS-----------------------------------------------
// shortcuts to change mood
void gameEngine::keyBoardHandler::keyboardCallback(unsigned char keyPressed, int mouseX, int mouseY) {

	switch (keyPressed) {
	case 27: // escape
#ifndef __APPLE__
		glutLeaveMainLoop();
#else
		exit(0);
#endif
		break;
	case 'r': // restart game & load data from config
		gameHandler->initializeApplication();
		gameHandler->restartGame();
		break;
	case 'c': // switch camera
		if (!gameState.curveMotion) {

			gameState.freeCameraMode = !gameState.freeCameraMode;
			if (gameState.freeCameraMode) {
				glutPassiveMotionFunc(gameEngine::screenHandler::passiveMouseMotionCallback);
				glutWarpPointer(gameState.windowWidth / 2, gameState.windowHeight / 2);
			}
			else {
				gameObjects.camera->speed = 0;
				glutPassiveMotionFunc(NULL);
				gameUniVars.spotLight = false;
			}
		}
		break;
	case 'p': // change between static positions
		if (!gameState.curveMotion) {
			camHandler.changePosition(&cameraPosition);
		}
		break;

	case 'm': // turn on curve motion
		gameState.curveMotion = !gameState.curveMotion;
		if (!gameState.curveMotion) {
			gameObjects.camera->speed = 0;
			gameState.freeCameraMode = false;
			gameObjects.camera->direction = freeCamDir;
			gameObjects.camera->position = freeCamPos;
		}
		else {
			freeCamDir = gameObjects.camera->direction;
			freeCamPos = gameObjects.camera->position;
		}
		break;
	case ' ': // turn camera reflektor
		if (gameState.freeCameraMode)
			gameUniVars.spotLight = !gameUniVars.spotLight;
		break;
	case 'f': // turn on fog
		gameUniVars.isFog = !gameUniVars.isFog;
		break;
	case 'd':
		gameState.isCloudy = !gameState.isCloudy;
		break;
	case 'w':
		camHandler.moveCamUp(gameObjects.camera, 0.3f);
		break;
	case 's':
		camHandler.moveCamDown(gameObjects.camera, -0.3f);
		break;
	default:
		;
	}
}

//special key is pressed
void gameEngine::keyBoardHandler::specialKeyboardCallback(int specKeyPressed, int mouseX, int mouseY) {

	if (gameState.gameOver == true)
		return;

	switch (specKeyPressed) {
	case GLUT_KEY_RIGHT:
		gameState.keyMap[KEY_RIGHT_ARROW] = true;
		break;
	case GLUT_KEY_LEFT:
		gameState.keyMap[KEY_LEFT_ARROW] = true;
		break;
	case GLUT_KEY_UP:
		gameState.keyMap[KEY_UP_ARROW] = true;
		break;
	case GLUT_KEY_DOWN:
		gameState.keyMap[KEY_DOWN_ARROW] = true;
		break;
	default:
		;
	}
}

// special keys released
void gameEngine::keyBoardHandler::specialKeyboardUpCallback(int specKeyReleased, int mouseX, int mouseY) {

	if (gameState.gameOver == true)
		return;

	switch (specKeyReleased) {
	case GLUT_KEY_RIGHT:
		gameState.keyMap[KEY_RIGHT_ARROW] = false;
		break;
	case GLUT_KEY_LEFT:
		gameState.keyMap[KEY_LEFT_ARROW] = false;
		break;
	case GLUT_KEY_UP:
		gameState.keyMap[KEY_UP_ARROW] = false;
		break;
	case GLUT_KEY_DOWN:
		gameState.keyMap[KEY_DOWN_ARROW] = false;
		break;
	default:
		;
	}
}
//---------------------------------------------------------CREATING MENU----------------------------------------------------------
// Function processing game menu
void gameEngine::gameMenu(int choice) {
	switch (choice)
	{
	case 0:
		//changing position
		gameState.freeCameraMode = false;
		camHandler.changePosition(&cameraPosition);
		break;
	case 1:
		//changing position
		gameState.freeCameraMode = false;
		camHandler.changePosition(&cameraPosition);
		break;
	case 2:
		//free camera mode
		gameState.freeCameraMode = true;
		if (gameState.freeCameraMode) {
			glutPassiveMotionFunc(gameEngine::screenHandler::passiveMouseMotionCallback);
			glutWarpPointer(gameState.windowWidth / 2, gameState.windowHeight / 2);
		}
		break;
	case 3:
		// fog
		gameUniVars.isFog = !gameUniVars.isFog;
		break;
	case 4:
		// change cloudy weather
		gameState.isCloudy = !gameState.isCloudy;
		break;
	case 5:
		// explosion with blowing maxwell
		if (!gameState.blowMaxwell) {
			gameState.blowMaxwell = !gameState.blowMaxwell;
			if (gameState.blowMaxwell) {
				gameObjects.maxwellObj->currentTime = 0.0;
				createExplosion(m_loadProps["maxwell"].position);
			}
		}
		break;
	case 6:
		//chill maxwell to static position
		if (gameState.blowMaxwell) {
			gameState.blowMaxwell = !gameState.blowMaxwell;
			gameObjects.maxwellObj->position = gameObjects.maxwellObj->origPos;
		}
		break;
	case 7:
		// sphere light intensity
		gameHandler->changePointLight();
		break;
	case 8:
		gameHandler->initializeApplication();
		gameHandler->restartGame();
		break;
	case 9:
		glutLeaveMainLoop();
		break;
	}
}


// Function generating simple menu
void gameEngine::createMenu(void) {
	if (controlMenu) {
		glutDestroyMenu(controlMenu);  // destroy if exists
	}

	controlMenu = glutCreateMenu(gameMenu);

	int cameraSubmenu = glutCreateMenu(gameMenu);
	glutAddMenuEntry("Static cam 1", 0);
	glutAddMenuEntry("Static cam 2", 1);
	glutAddMenuEntry("Free cam", 2);

	int weatherSubmenu = glutCreateMenu(gameMenu);
	glutAddMenuEntry("ON/OFF Fog", 3);
	glutAddMenuEntry("ON/OFF Cloudy weather", 4);

	int clickSubmenu = glutCreateMenu(gameMenu);
	glutAddMenuEntry("Blow Maxwell Up", 5);
	glutAddMenuEntry("Pet Maxwell", 6);
	glutAddMenuEntry("Light", 7);

	glutCreateMenu(gameMenu);
	glutAddSubMenu("Camera", cameraSubmenu);
	glutAddSubMenu("Weather", weatherSubmenu);
	glutAddSubMenu("Actions", clickSubmenu);
	glutAddMenuEntry("Restart", 8);
	glutAddMenuEntry("Exit", 9);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}


// Called after the window and OpenGL are initialized. Called exactly once, before the main loop.
void gameEngine::initializeApplication() {
	m_loadProps = loadConfig(CONFIG_PATH); //load data from config to map - only for restart

	glutDisplayFunc(m_screenHandler.displayCallback);
	// register callback for change of window size
	glutReshapeFunc(m_screenHandler.reshapeCallback);
	// register callbacks for keyboard
	glutKeyboardFunc(m_keyBoardHandler.keyboardCallback);     // key pressed
	glutSpecialFunc(m_keyBoardHandler.specialKeyboardCallback);     // special key pressed
	glutSpecialUpFunc(m_keyBoardHandler.specialKeyboardUpCallback); // key released

	glutTimerFunc(33, m_screenHandler.timerCallback, 0);

	// initialize PGR framework (GL, DevIl, etc.)
	if (!pgr::initialize(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR))
		pgr::dieWithError("pgr init failed, required OpenGL not supported?");

	// initialize random seed
	srand((unsigned int)time(NULL));
	glClearColor(0.06f, 0.08f, 0.16f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND); // enable blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gameUniVars.useLighting = true;
	gameUniVars.isFog = false;
	gameState.curveMotion = false;
	gameState.isCloudy = false;
	gameUniVars.lightIntensity = 0.9f;

	waterFBOHandler= new waterBufferMaker();
	// initialize shaders
	renderHandler.getInitHandler().initializeShaderPrograms();
	setupLights();
	renderHandler.getInitHandler().setLight( sun, cameraReflector, sphereLight );
	// create geometry for all models used
	renderHandler.getInitHandler().initializeModels(waterFBOHandler);
	glutMouseFunc(m_screenHandler.mouseCallback);
	gameObjects.camera = NULL;

	createMenu();

	gameHandler->restartGame();
}


// Clean all structures
void gameEngine::finalizeApplication() {

	delete gameObjects.camera;
	gameObjects.camera = NULL;
	delete gameObjects.duck;
	delete gameHandler;
	delete waterFBOHandler;
	renderHandler.cleanupModels();
	renderHandler.cleanupShaderPrograms();
}

// init application
int main(int argc, char** argv) {

	// initialize windowing system
	glutInit(&argc, argv);

#ifndef __APPLE__
	glutInitContextVersion(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
#else
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
#endif

	// initial window size
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow(WINDOW_TITLE);


	gameHandler->initializeApplication();

#ifndef __APPLE__
	glutCloseFunc(gameHandler->finalizeApplication);
#else
	glutWMCloseFunc(finalizeApplication);
#endif

	glutMainLoop();

	return 0;
}