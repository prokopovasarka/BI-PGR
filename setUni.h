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

#include "pgr.h"
#include "render_stuff.h"
#include "data.h"

void setMaterialUniforms(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess, GLuint texture, SCommonShaderProgram& shaderProgram, GameUniformVariables gameUni);
void setTransformUniforms(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram);

#endif