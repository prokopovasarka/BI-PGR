//-----------------------------------------------------------------------------------------
/**
 * \file       water.cpp
 * \author     Šárka Prokopová
 * \date       2022/4/28
 * \brief      File for generating water texture
 *
*/
//-----------------------------------------------------------------------------------------
#include "water.h"

// create normal square
void createSquare(GLfloat waterVertices[], float x1, float y1, float x2, float y2, int index, float z) {

	const GLfloat square[] = {
		// x   y  z   nx    ny    nz    u     v
			x1, y1, z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			x1, y2, z, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			x2, y1, z, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,

			x2, y2, z, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
			x1, y2, z, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			x2, y1, z, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
	};
	for (int i = 0; i < SQUARE_SIZE; i++) {
		waterVertices[i + index] = square[i];
	}
}

// create random square to set water texture
void randomSquare(GLfloat floorVertices[], float x1, float y1, float x2,
	float y2, int index, float z1, float z2, float newZ1, float newZ2) {
	// vectors
	glm::vec3 a_b = glm::vec3(x1 - x1, y1 - y2, newZ1 - z1);
	glm::vec3 a_c = glm::vec3(x1 - x2, y1 - y1, newZ1 - newZ2);
	glm::vec3 d_c = glm::vec3(x2 - x2, y2 - y1, z2 - newZ2);
	glm::vec3 d_b = glm::vec3(x2 - x1, y2 - y2, z2 - z1);
	// cross products
	glm::vec3 aCross = normalize(glm::cross(-a_b, -a_c));
	glm::vec3 bCross = normalize(glm::cross(a_b, d_b));
	glm::vec3 cCross = normalize(glm::cross(d_c, a_c));
	glm::vec3 dCross = normalize(glm::cross(-d_c, -d_b));
	glm::vec3 normalCross = glm::vec3(0.0, 0.0, 0.1);
	// create face normal 
	glm::vec3 res = normalize(aCross + bCross + cCross + dCross + normalCross);
	const GLfloat square[] = {
		x1, y1, newZ1,   res.x,  res.y, res.z, 0.0f, 0.0f,      //a
			x1, y2, z1,      res.x,  res.y, res.z, 0.0f, 1.0f,      //b
			x2, y1, newZ2,   res.x,  res.y, res.z, 1.0f, 0.0f,      //c

			x2, y2, z2,      res.x,  res.y,  res.z, 1.0f, 1.0f,      //d
			x1, y2, z1,      res.x,  res.y,  res.z, 0.0f, 1.0f,      //b
			x2, y1,  newZ2,  res.x,  res.y,  res.z, 1.0f, 0.0f,      //c

	};
	for (int i = 0; i < SQUARE_SIZE; i++) {
		floorVertices[i + index] = square[i];
	}
}



// generates water 
void generateWater(GLfloat waterVertices[]) {
	// create plain 36 squares
	float size = 1.0f;
	int index = 0;
	for (int i = -3; i < 3; i += 1) {
		for (int j = -3; j < 3; j += 1) {
			createSquare(waterVertices, i, j, i + size, j + size, index, 0.5f);
			index += SQUARE_SIZE;
		}
	}
	// create 400-36 more squares
	float z1 = 0.5f;
	float z2 = 0.5f;
	float newZ1 = 0.5;
	float newZ2 = 0.5;

	const int startX = -10;
	const int endX = 10;
	const int startY = -10;
	const int endY = 10;
	float zValues[endX - startX + 1];
	for (int i = 0; i < endX - startX; i++) {
		zValues[i] = 0.5f;
	}
	for (int j = endY; j > startY; j -= 1) {
		for (int i = startX; i < endX; i += 1) {

			if (!(i<3 && j < 3 && i > -4 && j >-4)) {
				// conditions to fit the plain in the center
				if (j == -4 && i < 3 && i > -4) {
					randomSquare(waterVertices, i, j, i + size, j + size, index,
						0.5f, 0.5f, newZ1, newZ2);
				}
				else if (j == 3 && i < 3 && i > -4) {
					randomSquare(waterVertices, i, j, i + size, j + size, index,
						zValues[i - startX], zValues[i - startX + 1], 0.5f, 0.5f);

				}
				else if (i == 3 && j < 3 && j > -4) {
					randomSquare(waterVertices, i, j, i + size, j + size, index,
						-0.01f, zValues[i - startX + 1], 0.5f, newZ2);
				}
				else if (i == -4 && j < 3 && j > -4) {
					randomSquare(waterVertices, i, j, i + size, j + size, index,
						zValues[i - startX], 0.5f, newZ1, 0.5f);
				}
				// corners needs to change only one value
				else if (i == -4 && j == 3) {
					randomSquare(waterVertices, i, j, i + size, j + size, index,
						zValues[i - startX], zValues[i - startX + 1], newZ1, 0.5f);
				}
				else if (i == 3 && j == 3) {
					randomSquare(waterVertices, i, j, i + size, j + size, index,
						zValues[i - startX], zValues[i - startX + 1], 0.5f, newZ2);
				}
				else if (i == 3 && j == -4) {
					randomSquare(waterVertices, i, j, i + size, j + size, index,
						0.5f, zValues[i - startX + 1], newZ1, newZ2);
				}
				else if (i == -4 && j == -4) {
					randomSquare(waterVertices, i, j, i + size, j + size, index,
						zValues[i - startX], 0.5f, newZ1, newZ2);
				}
				else {
					randomSquare(waterVertices, i, j, i + size, j + size, index,
						zValues[i - startX], zValues[i - startX ], newZ1, newZ2);
				}

				zValues[i - startX] = newZ1;
				newZ1 = newZ2;
				newZ2 = 0.5;
				index += SQUARE_SIZE;
			}
		}
	}
}
