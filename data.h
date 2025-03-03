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

#define WINDOW_WIDTH   800
#define WINDOW_HEIGHT  800
#define WINDOW_TITLE   "PGR semestral"

// keys used in the key map
enum { KEY_LEFT_ARROW, KEY_RIGHT_ARROW, KEY_UP_ARROW, KEY_DOWN_ARROW, KEY_SPACE, KEYS_COUNT };


#define CAMERA_VIEW_ANGLE_DELTA 2.0f // in degrees
#define CAMERA_SPEED_INCREMENT  0.025f
#define CAMERA_SPEED_MAX        0.8f


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
		"data/textures/w5.png"						  //texture name
};

typedef struct ObjectProp {

	glm::vec3     front;     // front vector or rotation axis
	glm::vec3     up;        // used if align is true
	glm::vec3     position;  // position of the object
	float         size;      // size of the object
	float         angle;     // rotation angle in radians
	bool          align;     // to use align method

} ObjectProp;

const ObjectProp platformProps {
		glm::vec3(0.0, 1.0, 0.0),     // rotation axis
		glm::vec3(0.0, 0.0, 0.0),      // not using
		glm::vec3(-20.4, 12.7, 1.29),   // position
		0.1f,						// size
		4.75,						// rotation
		false				       // align

};

const ObjectProp boatProps{
		glm::vec3(1.0, 0.0, 0.0),     // rotation axis
		glm::vec3(0.0, 0.0, 0.0),      // not using
		glm::vec3(1.4, 3.7, 0.09),   // position
		1.0f,						// size
		8.0,						// rotation
		false				       // align
};

const ObjectProp boatSecProps{
		glm::vec3(1.0, 0.0, 0.0),     // rotation axis
		glm::vec3(0.0, 0.0, 0.0),      // not using
		glm::vec3(1.1, -1.7, 0.09),   // position
		0.4f,						// size
		-4.7,						// rotation
		false				       // align
};

const ObjectProp lampProps{
		glm::vec3(1.0, 0.0, 0.0),     // rotation axis
		glm::vec3(0.0, 0.0, 0.0),      // not using
		glm::vec3(6.0, 2.0, 1.21),   // position
		1.0f,						// size
		4.7,						// rotation
		false				       // align
};

const ObjectProp bridgeProps{
		glm::vec3(0.1, 0.0, 0.0),     // rotation axis
		glm::vec3(0.0, 0.0, 0.0),      // not using
		glm::vec3(6.4, 1.7, 0.09),   // position
		4.0f,						// size
		7.83,						// rotation
		false				       // align
};


// amount of points in curve for camera
const size_t curveSize = 14;

// curve points for camera
const glm::vec3 curveData[] = {
	 glm::vec3(0.00, -2.0, 0.0),

	 glm::vec3(0.8, -1.65, 0.2),

	 glm::vec3(1.2, -1.3, 0.6),
	 glm::vec3(1.2, -0.6, 0.8),
	 glm::vec3(1.2, 0.1, 0.6),
	 glm::vec3(1.2, 0.8, 0.4),

	 glm::vec3(0.8, 1.15, 0.2),

	 glm::vec3(0.0, 1.5, 0.0),

	 glm::vec3(-0.8, 1.15, 0.2),

	 glm::vec3(-1.2, 0.8, 0.4),
	 glm::vec3(-1.2, 0.1, 0.6),
	 glm::vec3(-1.2, -0.6, 0.8),
	 glm::vec3(-1.2, -1.3, 0.4),

	 glm::vec3(-0.8, -1.65, 0.0),

};
// amount of vertices for banner
const int bannerNumQuadVertices = 4;
// vertices
const float bannerVertexData[bannerNumQuadVertices * 5] = {

	// x      y      z     u     v
	-1.0f,  0.15f, 0.0f, 0.0f, 1.0f,
	-1.0f, -0.15f, 0.0f, 0.0f, 0.0f,
	 1.0f,  0.15f, 0.0f, 3.0f, 1.0f,
	 1.0f, -0.15f, 0.0f, 3.0f, 0.0f
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
const size_t corpseCurvePointsTotal = 6;
// coordinates of points 
const glm::vec3 corpseCurvePoints[] = {
	glm::vec3(-2.18f, -2.05f, -0.06f),
	glm::vec3(-2.27f, -2.13f, -0.08f),
	glm::vec3(-2.4f, -2.13f, -0.06f),
	glm::vec3(-2.53f, -2.13f, -0.08f),
	glm::vec3(-2.64f, -2.01f, -0.06f),
	glm::vec3(-2.53, -2.07f, -0.08f),
	glm::vec3(-2.4f, -2.07f, -0.06f),
	glm::vec3(-2.27f, -2.07f, -0.08f),
};

#endif
