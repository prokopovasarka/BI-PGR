//-----------------------------------------------------------------------------------------
/**
 * \file       data.h
 * \author     Šárka Prokopová
 * \date       2022/4/28
 * \brief      parameters for props and material
 *
*/
//-----------------------------------------------------------------------------------------
#ifndef __DATA_H
#define __DATA_H

#define WINDOW_WIDTH   1000
#define WINDOW_HEIGHT  800
#define WINDOW_TITLE   "PGR semestral"

// keys used in the key map
enum { KEY_LEFT_ARROW, KEY_RIGHT_ARROW, KEY_UP_ARROW, KEY_DOWN_ARROW, KEY_SPACE, KEYS_COUNT };


#define CAMERA_VIEW_ANGLE_DELTA 2.0f // in degrees
#define CAMERA_SPEED_INCREMENT  0.025f
#define CAMERA_SPEED_MAX        1.0f


/// maximum angle to view with mouse 
#define CAMERA_ELEVATION_MAX_Y 45.0f
#define CAMERA_ELEVATION_MAX_X 70.0f

#define SCENE_WIDTH  1.0f
#define SCENE_HEIGHT 1.0f
#define SCENE_DEPTH  1.0f

// fog
#define FOG_COLOR glm::vec4(0.0f, 0.2f, 0.0f, 0.5f);

//water data
const float WATER_MAX_HEIGHT = 0.5;
const float WATER_MIN_HEIGHT = -0.4;

/// amount of towers to generate
const int towerAmount = 1;

typedef struct Material {

	glm::vec3     ambient;
	glm::vec3     diffuse;
	glm::vec3     specular;
	float         shininess;
	std::string   texture;

} Material;

const Material waterMaterial = {
		glm::vec3(0.5f, 0.5f, 0.5f),  //ambient
		glm::vec3(0.5f, 0.5f, 0.5f),  //deffuse
		glm::vec3(0.7f, 0.7f, 0.7f),  //specular
		1.0f,                         //shinines					 
};

typedef struct ObjectProp {

	glm::vec3     front;     // front vector or rotation axis
	glm::vec3     up;        // used if align is true
	glm::vec3     position;  // position of the object
	float         size;      // size of the object
	float         angle;     // rotation angle in radians
	bool          align;     // to use align method

} ObjectProp;

// amount of points in curve for camera
const size_t curveSize = 14;

// curve points for camera
const glm::vec3 curveData[] = {
	 glm::vec3(0.00, -2.0, 1.3),

	 glm::vec3(0.8, -1.65, 1.5),

	 glm::vec3(1.2, -1.3, 1.7),
	 glm::vec3(1.2, -0.6, 1.8),
	 glm::vec3(1.2, 0.1, 1.7),
	 glm::vec3(1.2, 0.8, 1.5),

	 glm::vec3(0.8, 1.15, 1.3),

	 glm::vec3(0.0, 1.5, 1.2),

	 glm::vec3(-0.8, 1.15, 1.3),

	 glm::vec3(-1.2, 0.8, 1.2),
	 glm::vec3(-1.2, 0.1, 1.4),
	 glm::vec3(-1.2, -0.6, 1.6),
	 glm::vec3(-1.2, -1.3, 1.3),

	 glm::vec3(-0.8, -1.65, 1.2),

};
// amount of vertices for banner
const int barNumQuadVertices = 4;
// vertices
const float barVertexData[barNumQuadVertices * 5] = {
	// x      y      z     u     v
	-1.0f,  0.15f, 0.0f, 0.0f, 1.0f, // levý horní roh
	-1.0f, -0.15f, 0.0f, 0.0f, 0.0f, // levý dolní roh
	 1.0f,  0.15f, 0.0f, 1.0f, 1.0f, // pravý horní roh (u x bude závislé na rychlosti)
	 1.0f, -0.15f, 0.0f, 1.0f, 0.0f  // pravý dolní roh (u x bude závislé na rychlosti)
};

// amount of vertexes for explosion
const int explosionVertexCount = 4;
// 4 vertices for explosion
const float explosionVertexData[explosionVertexCount * 5] = {

	// x      y     z     u     v
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
	 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
};

// amount of points in curve
const size_t duckCurvePointsTotal = 6;
const float radius = 0.2f;
const float PI = 3.14f;
// coordinates of points 
const glm::vec3 duckCurvePoints[] = {
	glm::vec3(1.4f + radius * cos(0.0f), 1.0f + radius * sin(0.0f), 1.15f),
	glm::vec3(1.4f + radius * cos(PI / 3), 1.0f + radius * sin(PI / 3), 1.15f),
	glm::vec3(1.4f + radius * cos(2 * PI / 3), 1.0f + radius * sin(2 * PI / 3), 1.15f),
	glm::vec3(1.4f + radius * cos(PI), 1.0f + radius * sin(PI), 1.15f),
	glm::vec3(1.4f + radius * cos(4 * PI / 3), 1.0f + radius * sin(4 * PI / 3), 1.15f),
	glm::vec3(1.4f + radius * cos(5 * PI / 3), 1.0f + radius * sin(5 * PI / 3), 1.15f),
};

#endif
