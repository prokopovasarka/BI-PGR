//-----------------------------------------------------------------------------------------
/**
 * \file       water.cpp
 * \author     Šárka Prokopová
 * \date       2025/4/28
 * \brief      File for generating water - using two buffers for reflection and
 *				refraction, dudv map.
 *
*/
//-----------------------------------------------------------------------------------------
#include "water.h"


void waterBufferMaker::cleanUp() {//call when closing the game
	glDeleteFramebuffers(1, &reflectionFrameBuffer);
	glDeleteTextures(1, &reflectionTexture);
	glDeleteRenderbuffers(1, &reflectionDepthBuffer);
	glDeleteFramebuffers(1, &refractionFrameBuffer);
	glDeleteTextures(1, &refractionTexture);
	glDeleteTextures(1, &refractionDepthTexture);
}


void waterBufferMaker::bindReflectionFrameBuffer() {//call before rendering to this FBO
	bindFrameBuffer(reflectionFrameBuffer, REFLECTION_WIDTH, REFLECTION_HEIGHT);
}

void waterBufferMaker::bindRefractionFrameBuffer() {//call before rendering to this FBO
	bindFrameBuffer(refractionFrameBuffer, REFRACTION_WIDTH, REFRACTION_HEIGHT);
}

void waterBufferMaker::unbindCurrentFrameBuffer() {//call to switch to default frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}


void waterBufferMaker::initialiseReflectionFrameBuffer() {
	reflectionFrameBuffer = createFrameBuffer();
	reflectionTexture = createTextureAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
	reflectionDepthBuffer = createDepthBufferAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
	unbindCurrentFrameBuffer();
}

void waterBufferMaker::initialiseRefractionFrameBuffer() {
	refractionFrameBuffer = createFrameBuffer();
	refractionTexture = createTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
	refractionDepthTexture = createDepthTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
	unbindCurrentFrameBuffer();
}


void waterBufferMaker::bindFrameBuffer(int frameBuffer, int width, int height) {
	glBindTexture(GL_TEXTURE_2D, 0);//To make sure the texture isn't bound
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0, 0, width, height);
}

GLuint waterBufferMaker::createFrameBuffer() {
	GLuint frameBuffer;
	glGenFramebuffers(1, &frameBuffer);
	//generate name for frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	//create the framebuffer
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//indicate that we will always render to color attachment 0
	return frameBuffer;

}

GLuint waterBufferMaker::createTextureAttachment(int width, int height) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
	return texture;
}

GLuint waterBufferMaker::createDepthTextureAttachment(int width, int height) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		texture, 0);
	return texture;
}


GLuint waterBufferMaker::createDepthBufferAttachment(int width, int height) {
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width,height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	return depthBuffer;
}

GLuint waterBufferMaker::getReflectionTexture() {
	return reflectionTexture;
}

GLuint waterBufferMaker::getRefractionTexture() {
	return refractionTexture;
}

GLuint waterBufferMaker::getRefractionDepthTexture() {
	return refractionDepthTexture;
}




void addVertex(GLfloat* buffer, int& index, float x, float y, float z, float u, float v) {
	buffer[index++] = x;
	buffer[index++] = y;
	buffer[index++] = z;
	buffer[index++] = 0.0f; // normal x
	buffer[index++] = 0.0f; // normal y
	buffer[index++] = 1.0f; // normal z
	buffer[index++] = u;
	buffer[index++] = v;
}

void createSquare(GLfloat* buffer, int& index, float x1, float y1, float x2, float y2, float z) {
	// 2 triangles
	addVertex(buffer, index, x1, y1, z, 0.0f, 0.0f);
	addVertex(buffer, index, x1, y2, z, 0.0f, 1.0f);
	addVertex(buffer, index, x2, y1, z, 1.0f, 0.0f);

	addVertex(buffer, index, x2, y2, z, 1.0f, 1.0f);
	addVertex(buffer, index, x1, y2, z, 0.0f, 1.0f);
	addVertex(buffer, index, x2, y1, z, 1.0f, 0.0f);
}

void generateWater(GLfloat* waterVertices) {
	int index = 0;

	// Nastav hranice plochy
	float x1 = -WATER_RES / 2.0f;
	float y1 = -WATER_RES / 2.0f;
	float x2 = WATER_RES / 2.0f;
	float y2 = WATER_RES / 2.0f;

	createSquare(waterVertices, index, x1, y1, x2, y2, WATER_Z);
}
