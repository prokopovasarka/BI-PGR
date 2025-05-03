//-----------------------------------------------------------------------------------------
/**
 * \file       render_stuff.h
 * \author     Šárka Prokopová
 * \date       2025/4/28
 * \brief      Functions for rendering, initialization and drawing
 *
*/
//-----------------------------------------------------------------------------------------
#ifndef __RENDER_STUFF_H
#define __RENDER_STUFF_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include "pgr.h"
#include "utilStructures.h"
#include "setUni.h"
#include "spline.h"
#include "water.h"
#include "model.h"

class renderObjects {
public:
	renderObjects() {
		m_initHandler = initHandler();
		m_drawHandler = drawHandler();
	}

	class initHandler {
	public:
		void initSkybox(GLuint, MeshGeometry**);

		bool loadSingleMesh(const std::string& fileName, SCommonShaderProgram& shader, MeshGeometry** geometry);
		bool loadMesh(const std::string& fileName, SCommonShaderProgram& shader, std::vector<MeshGeometry*>* geometryFull);
		void initMaterial(MeshGeometry** geometry, Material material);
		void initBarGeometry(GLuint shader, MeshGeometry** geometry);
		void initSkyboxGeometry(skyboxFarPlaneShaderProgram  skyboxShader, MeshGeometry** geometry);
		void initWater(SCommonShaderProgram& shader, MeshGeometry** geometry, waterBufferMaker* waterFBOHandler);
		void initplatformGeometry(SCommonShaderProgram& shader, MeshGeometry** geometry);
		void initExplosion(ExplosionShaderProgram& explosionShader, MeshGeometry** geometry);

		void initializeShaderPrograms();

		void setLight(Light& sun, Light& camR, Light& sphere);

		void initializeModels(waterBufferMaker* waterFBOHandler);

		GLuint platformTexture;
		GLuint grassTexture;

	};


	class drawHandler {
	public:
		void drawSkybox(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
			skyboxFarPlaneShaderProgram& skyboxShader, MeshGeometry** geometry, GameUniformVariables gameUni);
		void drawTower(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
			SCommonShaderProgram& shaderProgram, MeshGeometry** geometry, GameUniformVariables gameUni,
			glm::vec3 towerPosition);
		void drawCube(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
			SCommonShaderProgram& shaderProgram, MeshGeometry** geometry, GameUniformVariables gameUni,
			glm::vec3 cubePosition, float angle);
		void drawSphere(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
			SCommonShaderProgram& shaderProgram, MeshGeometry** geometry, GameUniformVariables gameUni, glm::vec3 spherePosition);
		void drawObject(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram,
			std::vector<MeshGeometry*>* geometry, GameUniformVariables gameUni, ObjectProp param);
		void drawWater(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram, 
			MeshGeometry** geometry, GameUniformVariables gameUni, waterBufferMaker* waterFBOHandler);
		void drawEverything(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, bool drawWaterBool, std::map<std::string, ObjectProp> loadProps, waterBufferMaker* waterFBOHandler);
		void drawDuck(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram, GameUniformVariables gameUni, ObjectProp param, glm::vec3 position, glm::vec3 dir);
		void drawMaxwell(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram, GameUniformVariables gameUni, ObjectProp param, glm::vec3 position, glm::vec3 dir);
		void drawExplosion(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, Explosion* explosion);
		void drawBar(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, float loadingBarWidth);
		void drawHouse(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram, 
			MeshGeometry** geometry, GameUniformVariables gameUni, glm::vec3 houœePosition);
		void drawPlatform(ObjectProp platformProps, const glm::mat4& viewMatrix, SCommonShaderProgram& shaderProgram,
			MeshGeometry** geometry, GameUniformVariables gameUni, const glm::mat4& projectionMatrix);
		void drawExplosionMet(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, ExplosionShaderProgram& explosionShader, Explosion* explosion, MeshGeometry** geometry);
	};

	void cleanupShaderPrograms();
	void cleanupModels();

	initHandler& getInitHandler() { return m_initHandler; }
	drawHandler& getDrawHandler() { return m_drawHandler; }
	setUniforms& getUniSetter() { return uniSetter; }

private:
	initHandler m_initHandler;
	drawHandler m_drawHandler;
	static setUniforms uniSetter;

};

#endif
