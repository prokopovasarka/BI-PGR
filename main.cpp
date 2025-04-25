//-----------------------------------------------------------------------------------------
/**
 * \file       main.cpp
 * \author     Šárka Prokopová
 * \date       2025/3/3
 * \brief      house by the lake
 *
 *  Interactive scene where user can move around and click on objects.
 *  Scene has changing daytime, lights, interactive corpse animation.
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

Object* corpseObj; //init corpse object
bool corpseAnimation; //if animation is on or off

//-----------------------------------------------------------CREATING BANNER-------------------------------------------------------------
Object* gameEngine::createBanner() {
	Object* newBanner = new Object;

	newBanner->position = glm::vec3(0.0f, 0.0f, 0.0f);
	newBanner->direction = glm::vec3(1.0f, 0.0f, 0.0f);
	newBanner->size = 3.0f;

	newBanner->destroyed = false;
	newBanner->startTime = gameState.elapsedTime;
	newBanner->currentTime = newBanner->startTime;

	return newBanner;
}
//----------------------------------------------------------INTERACTION WITH OBJECTS------------------------------------------------------
void createExplosion(glm::vec3 position) {
	Explosion* newExplosion = new Explosion;

	newExplosion->speed = 0.0f;
	newExplosion->destroyed = false;

	newExplosion->startTime = gameState.elapsedTime;
	newExplosion->currentTime = newExplosion->startTime;

	newExplosion->size = 0.3;
	newExplosion->direction = glm::vec3(0.0f, 0.0f, 1.0f);

	newExplosion->frameDuration = 0.02f;
	newExplosion->frames = 84;

	newExplosion->position = position;
	newExplosion->end = newExplosion->startTime + newExplosion->frames * newExplosion->frameDuration;

	explosions.push_back(newExplosion);
}

void controlExplosion(Explosion* explosion) {

	if (explosion->currentTime > explosion->end)
		explosion->destroyed = true;
}

// creating corpse model

void gameEngine::corpseHandler::createCorpse() {
	corpseObj = new Object;
	corpseObj->position = glm::vec3(-3.2f, -3.0f, -0.08f);
	corpseObj->direction = glm::vec3(0.0f, 1.0f, 0.0f);
	corpseObj->startTime = gameState.elapsedTime;
}

// updating corpse
void gameEngine::corpseHandler::updateCorpse(float  elapsedTime) {
	if (corpseAnimation) {
		corpseObj->startTime += elapsedTime;
		float a = corpseObj->startTime;
		a *= 0.6;
		corpseObj->position = splineFucHandler.evaluateClosedCurve(corpseCurvePoints, corpseCurvePointsTotal, a);
		corpseObj->direction = -glm::normalize(splineFucHandler.evalClosedCurveFirstDev(corpseCurvePoints, corpseCurvePointsTotal, a));
	}
}

// changing light 
void gameEngine::evalLightIntensity() {

	int time = int(gameState.elapsedTime * 1000) % (60 * 1000);
	float new_time = float(time) / 1000;
	if (new_time < 60 / 2) { 
		new_time = (new_time / (60 / 2));
	}
	else { 
		new_time = ((60 - new_time) / (60 / 2));
	}
	gameUniVars.lightIntensity = 1 - new_time;

}

// changing intensity of point light
void gameEngine::changePointLight() {

	float max_light = 0.9f;
	float min_light = 0.0;

	gameUniVars.pointLightIntensity += 0.3;
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
	corpseAnimation = true;
	m_corpseHandler.createCorpse();

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

	renderHandler.getDrawHandler().drawCorpse(viewMatrix, projectionMatrix, corpseObj); // draw corpse, v=2
	renderHandler.getDrawHandler().drawEverything(viewMatrix, projectionMatrix, drawWater); // draw almost all meshes

	if (gameState.drawBanner) {
		if (gameObjects.banner == NULL) {
			gameObjects.banner = gameHandler->createBanner();
		}
		else {
			if (!gameState.freeCameraMode) {
				renderHandler.getDrawHandler().drawBanner(gameObjects.banner, orthoViewMatrix, orthoProjectionMatrix);
			}
			else {
				gameState.drawBanner = false;
			}
			
		}	
		float time = (gameObjects.banner->currentTime - gameObjects.banner->startTime);
		float f = (glm::floor(time * 0.05f) - time * 0.05f) * 4 + 1.0;

		if (f < -2.988400f) {
			gameState.drawBanner = false;
		}
	}
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

	m_corpseHandler.updateCorpse(timeDelta); // new position for corpse
	gameHandler->evalLightIntensity(); // change light intensity with according to game time

	currentColor = mix(currentColor, day, timeDelta);

	if (gameState.curveMotion == true) { // show spline motion
		gameObjects.camera->startTime += timeDelta;
		float a = gameObjects.camera->startTime;
		a *= 0.6;
		gameObjects.camera->position = splineFucHandler.evaluateClosedCurve(curveData, curveSize, a);
		gameObjects.camera->direction = glm::normalize(splineFucHandler.evalClosedCurveFirstDev(curveData, curveSize, a));
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
	if (gameState.drawBanner == true ) {
		if (gameObjects.banner == NULL) {
			gameObjects.banner = gameHandler->createBanner();
		}
		else {
			gameObjects.banner->currentTime = gameState.elapsedTime;
		}
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
		} if (objectID == 2) { // corpse
			corpseAnimation = !corpseAnimation;
		} if (objectID == 3) { // boat
			createExplosion(boatProps.position);
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
	case 'r': // restart game
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
	case 'b': //shows banner if camera is static
		if (gameObjects.banner == NULL) {
			gameObjects.banner = gameHandler->createBanner();
		}
		if (!gameState.freeCameraMode) {
			gameState.drawBanner = true;
		}
		break;
	case 'w':
		camHandler.moveCamUp(gameObjects.camera, 3.0f);
		break;
	case 's':
		camHandler.moveCamDown(gameObjects.camera, -3.0f);
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
		// explosion
		createExplosion(boatProps.position);
		break;
	case 5:
		// sphere light intensity
		gameHandler->changePointLight();
		break;
	case 6:
		glutLeaveMainLoop();
		break;
	}
}


// Function generating simple menu
void gameEngine::createMenu(void) {
	int menu = glutCreateMenu(gameMenu);

	int cameraSubmenu = glutCreateMenu(gameMenu);
	glutAddMenuEntry("Static cam 1", 0);
	glutAddMenuEntry("Static cam 2", 1);
	glutAddMenuEntry("Free cam", 2);

	int fogSubmenu = glutCreateMenu(gameMenu);
	glutAddMenuEntry("ON/OFF", 3);

	int clickSubmenu = glutCreateMenu(gameMenu);
	glutAddMenuEntry("Boat", 4);
	glutAddMenuEntry("Light", 5);

	glutCreateMenu(gameMenu);
	glutAddSubMenu("Camera", cameraSubmenu);
	glutAddSubMenu("Fog", fogSubmenu);
	glutAddSubMenu("Actions", clickSubmenu);
	glutAddMenuEntry("Exit", 6);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}


// Called after the window and OpenGL are initialized. Called exactly once, before the main loop.
void gameEngine::initializeApplication() {

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

	gameUniVars.useLighting = true;
	gameUniVars.isFog = false;
	gameState.curveMotion = false;

	waterFBOHandler= new waterBufferMaker();
	// initialize shaders
	renderHandler.getInitHandler().initializeShaderPrograms();
	// create geometry for all models used
	renderHandler.getInitHandler().initializeModels(waterFBOHandler);
	glutMouseFunc(m_screenHandler.mouseCallback);
	gameObjects.camera = NULL;
	gameObjects.banner = NULL;

	createMenu();

	gameHandler->restartGame();
}


// Clean all structures
void gameEngine::finalizeApplication() {

	delete gameObjects.camera;
	gameObjects.camera = NULL;
	if (gameObjects.banner != NULL) {
		delete gameObjects.banner;
		gameObjects.banner = NULL;
	}
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