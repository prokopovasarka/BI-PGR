//-----------------------------------------------------------------------------------------
/**
 * \file       utilStructures.h
 * \author     Šárka Prokopová
 * \date       2022/4/28
 * \brief      Functions for rendering, initialization and drawing
 *
*/
//-----------------------------------------------------------------------------------------
#ifndef __UTIL_STRUCTURES_H
#define __UTIL_STRUCTURES_H

#include "pgr.h"
#include "data.h"

// geometry is shared among all instances of the same object type
typedef struct MeshGeometry {
	GLuint        vertexBufferObject;   // identifier for the vertex buffer object
	GLuint        elementBufferObject;  // identifier for the element buffer object
	GLuint        vertexArrayObject;    // identifier for the vertex array object
	unsigned int  numTriangles;         // number of triangles in the mesh
	// material
	glm::vec3     ambient;
	glm::vec3     diffuse;
	glm::vec3     specular;
	float         shininess;
	GLuint        texture;

} MeshGeometry;

// parameters of individual objects in the scene (e.g. position, size, speed, etc.)
typedef struct Object {
	glm::vec3 position;
	glm::vec3 direction;
	float	  angle;
	float     speed;
	float     size;
	bool destroyed;

	float startTime;
	float currentTime;

} Object;

// struct for camera object
typedef struct Camera : public Object {

	float viewAngle; // in degrees
	float elevationAngle;

} Camera;

// struct for variables which are send to shaders
typedef struct _gameUniformVariables {

	bool isFog;                // fog switcher
	bool spotLight;            // spot light switcher
	float pointLightIntensity; // intensity of the lamp in the scene
	bool useLighting;          // if lighting is using
	float lightIntensity;      // intensity of the light to create day

	glm::vec3 reflectorPositionLocation;  // position of camera for reflector
	glm::vec3 reflectorDirectionLocation;  // direction of camera for reflector


}GameUniformVariables;

typedef struct _commonShaderProgram {
	// identifier for the shader program
	GLuint program;          // = 0;
	// vertex attributes locations
	GLint posLocation;       // = -1;
	GLint colorLocation;     // = -1;
	GLint normalLocation;    // = -1;
	GLint texCoordLocation;  // = -1;
	GLint secTextureLocation;
	GLint texSampler2Location;
	// uniforms locations
	GLint PVMmatrixLocation;    // = -1;
	GLint VmatrixLocation;      // = -1;  view/camera matrix
	GLint MmatrixLocation;      // = -1;  modeling matrix
	GLint normalMatrixLocation; // = -1;  inverse transposed Mmatrix

	GLint timeLocation;         // = -1; elapsed time in seconds

	// material 
	GLint diffuseLocation;    // = -1;
	GLint ambientLocation;    // = -1;
	GLint specularLocation;   // = -1;
	GLint shininessLocation;  // = -1;
	// texture
	GLint useTextureLocation; // = -1; 
	GLint texSamplerLocation; // = -1;
	//reflector related uniforms
	GLint reflectorLocation;
	GLint reflectorPositionLocation;
	GLint reflectorDirectionLocation;
	GLint reflectorIntensityLocation;
	// fog switcher
	GLint isFogLocation;
	// lights
	GLint spotLightLocation;
	GLint pointLightIntensityLocation;
	GLint lightIntensityLocation;


} SCommonShaderProgram;

// shader for skybox
typedef struct _skyboxFarPlaneShaderProgram {
	// identifier for the shader program
	GLuint program;                 // = 0;
	// vertex attributes locations
	GLint screenCoordLocation;      // = -1;
	// uniforms locations
	GLint inversePVmatrixLocation; // = -1;
	GLint skyboxSamplerLocation;    // = -1;
	GLint isFogLocation;


} skyboxFarPlaneShaderProgram;

// game variables
typedef struct _gameState {

	int windowWidth;    // set by reshape callback
	int windowHeight;   // set by reshape callback

	bool freeCameraMode;        // false;
	bool gameOver;              // false;
	bool keyMap[KEYS_COUNT];
	bool drawBanner = false;    // false

	float elapsedTime;          // game time
	bool curveMotion;           // switch to spline motion

} GameState;

// init explosion
typedef struct Explosion : public Object {

	int    frames;
	float  frameDuration;
	float  end;
} Explosion;

// shader for explosion
typedef struct _explosionShaderProgram {
	GLuint program;
	// vertex attributes locations
	GLint posLocation;
	GLint texCoordLocation;
	// uniforms locations
	GLint PVMmatrixLocation;
	GLint VmatrixLocation;
	GLint timeLocation;
	GLint texSamplerLocation;
	GLint frameDurationLocation;

} ExplosionShaderProgram;

typedef struct _bannerShaderProgram {
	GLuint program;
	GLint posLocation;
	GLint texCoordLocation;
	GLint PVMmatrixLocation;
	GLint timeLocation;
	GLint texSamplerLocation;
} BannerShaderProgram;


#endif
