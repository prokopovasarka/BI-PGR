//-----------------------------------------------------------------------------------------
/**
 * \file       setUni.cpp
 * \author     Šárka Prokopová
 * \date       2022/4/28
 * \brief      File made to set all uniforms
 *
*/
//-----------------------------------------------------------------------------------------
#include "pgr.h"
#include "setUni.h"

// function to set material uniforms
void setUniforms::setMaterialUniforms(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess, GLuint texture, SCommonShaderProgram& shaderProgram, GameUniformVariables gameUni) {

	glUniform3fv(shaderProgram.diffuseLocation, 1, glm::value_ptr(diffuse));  // 2nd parameter must be 1 - it declares number of vectors in the vector array
	glUniform3fv(shaderProgram.ambientLocation, 1, glm::value_ptr(ambient));
	glUniform3fv(shaderProgram.specularLocation, 1, glm::value_ptr(specular));
	glUniform1f(shaderProgram.shininessLocation, shininess);
	glUniform1i(shaderProgram.isFogLocation, gameUni.isFog);
	glUniform1i(shaderProgram.spotLightLocation, gameUni.spotLight);
	glUniform1f(shaderProgram.pointLightIntensityLocation, gameUni.pointLightIntensity);
	glUniform1f(shaderProgram.lightIntensityLocation, gameUni.lightIntensity);
	//--- reflector ---
	glUniform3fv(shaderProgram.reflectorPositionLocation, 1, glm::value_ptr(gameUni.reflectorPositionLocation));
	glUniform3fv(shaderProgram.reflectorDirectionLocation, 1, glm::value_ptr(gameUni.reflectorDirectionLocation));

	if (texture != 0) {
		glUniform1i(shaderProgram.useTextureLocation, 1);
		glUniform1i(shaderProgram.texSamplerLocation, 0);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, texture);
	}
	else {
		glUniform1i(shaderProgram.useTextureLocation, 0);

	}
}

// function to set transform uniforms
void setUniforms::setTransformUniforms(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram) {

	glm::mat4 PVM = projectionMatrix * viewMatrix * modelMatrix;
	glUniformMatrix4fv(shaderProgram.PVMmatrixLocation, 1, GL_FALSE, glm::value_ptr(PVM));

	glUniformMatrix4fv(shaderProgram.VmatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(shaderProgram.MmatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	const glm::mat4 modelRotationMatrix = glm::mat4(
		modelMatrix[0],
		modelMatrix[1],
		modelMatrix[2],
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelRotationMatrix));
	glUniformMatrix4fv(shaderProgram.normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));  // correct matrix for non-rigid transform
}