//-----------------------------------------------------------------------------------------
/**
 * \file       water.h
 * \author     Šárka Prokopová
 * \date       2022/4/28
 * \brief      File for generating water texture
 *
*/
//-----------------------------------------------------------------------------------------
#ifndef __WATER_H
#define __WATER_H
#include "pgr.h"
#include <time.h>
#include "data.h"

// size of one square
const int SQUARE_SIZE = 48; 

// create normal square
const int WATER_RES = 17;
const float WATER_Z = 1.1f;
const float SQUARE_SIZE_F = 1.0f;
const int VERTEX_SIZE = 8;
const int TRIANGLE_VERTICES = 6;
const int SQUARE_STRIDE = TRIANGLE_VERTICES * VERTEX_SIZE;

static int REFLECTION_WIDTH = 320;
static int REFLECTION_HEIGHT = 180;

static int REFRACTION_WIDTH = 1280;
static int REFRACTION_HEIGHT = 720;


void addVertex(GLfloat* buffer, int& index, float x, float y, float z, float u, float v);
void generateWater(GLfloat waterVertices[]);
void createSquare(GLfloat waterVertices[], float x1, float y1, float x2, float y2, int index);

class waterBufferMaker {
public:

	waterBufferMaker() {
		initialiseReflectionFrameBuffer();
		initialiseRefractionFrameBuffer();
	}

	GLuint createFrameBuffer();
	void cleanUp();
	void bindReflectionFrameBuffer();
	void bindRefractionFrameBuffer();
	void unbindCurrentFrameBuffer();
	void initialiseReflectionFrameBuffer();
	void initialiseRefractionFrameBuffer();
	void bindFrameBuffer(int frameBuffer, int width, int height);
	GLuint createTextureAttachment(int width, int height);
	GLuint createDepthTextureAttachment(int width, int height);
	GLuint createDepthBufferAttachment(int width, int height);
	GLuint getReflectionTexture();
	GLuint getRefractionTexture();
	GLuint getRefractionDepthTexture();
	void setDudvMapTex(GLuint tex) { dudvMapTex = tex; }
	GLuint getdudvMapTexID() { return dudvMapTex; }

private:
	GLuint reflectionFrameBuffer;
	GLuint reflectionTexture;
	GLuint reflectionDepthBuffer;

	GLuint refractionFrameBuffer;
	GLuint refractionTexture;
	GLuint refractionDepthTexture;

	GLuint dudvMapTex;
};

#endif 
