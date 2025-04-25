//-----------------------------------------------------------------------------------------
/**
 * \file       setUni.h
 * \author     Šárka Prokopová
 * \date       2022/4/28
 * \brief      File made to set all uniforms
 *
*/
//-----------------------------------------------------------------------------------------
#ifndef __SETUNI_H
#define __SETUNI_H

#include "utilStructures.h"

class setUniforms {
public:
	setUniforms() = default;
	void setMaterialUniforms( MeshGeometry* geometry, SCommonShaderProgram& shaderProgram, GameUniformVariables gameUni);
	void setTransformUniforms(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram);
	void setWaterUni(SCommonShaderProgram& shaderProgram, GLfloat factor);
};


#endif