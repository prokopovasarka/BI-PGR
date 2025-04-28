//-----------------------------------------------------------------------------------------
/**
 * \file       spline.cpp
 * \author     Šárka Prokopová
 * \date       2025/4/28
 * \brief      Everything for computing spline - Catmull-Rom and Hermite
 *
*/
//-----------------------------------------------------------------------------------------
#include "spline.h"
#include <iostream>

// length of vector check
bool splineHandler::vectorLen(const glm::vec3& vect) {
    return !vect.x && !vect.y && !vect.z;
}

// align current coordinate system
glm::mat4 splineHandler::alignObject(const glm::vec3& position, const glm::vec3& front, const glm::vec3& up) {

    glm::vec3 z = -glm::normalize(front);

    if (vectorLen(z))
        z = glm::vec3(0.0, 0.0, 1.0);

    glm::vec3 x = glm::normalize(glm::cross(up, z));

    if (vectorLen(x))
        x = glm::vec3(1.0, 0.0, 0.0);

    glm::vec3 y = glm::cross(z, x);
    glm::mat4 matrix = glm::mat4(
        x.x, x.y, x.z, 0.0,
        y.x, y.y, y.z, 0.0,
        z.x, z.y, z.z, 0.0,
        position.x, position.y, position.z, 1.0
    );

    return matrix;
}

// evalucate one segment of the curve for duck movement
glm::vec3 splineHandler::evaluateCurveSegment(const glm::vec3 points[], const float t) {

	float t2 = t * t;
	float t3 = t2 * t;
	glm::vec4 ts(t3, t2, t, 1);
	glm::mat4 values(
		-1, 3, -3, 1,
		2, -5, 4, -1,
		-1, 0, 1, 0,
		0, 2, 0, 0
	);
	glm::vec4 polynom = ts * transpose(values);
	glm::vec3 res = points[0] * polynom[0]
		+ points[1] * polynom[1]
		+ points[2] * polynom[2]
		+ points[3] * polynom[3];

	res *= 0.5;
	return res;
}

// evaluate first derivative of one segment for duck movement
glm::vec3 splineHandler::evalCurveSegFirstDev(const glm::vec3 points[], const float t) {
	float t2 = t * t;
	glm::vec4 ts(3 * t2, 2 * t, 1, 0);
	glm::mat4 values(
		-1, 3, -3, 1,
		2, -5, 4, -1,
		-1, 0, 1, 0,
		0, 2, 0, 0
	);
	glm::vec4 polynom = ts * transpose(values);
	glm::vec3 res = points[0] * polynom[0]
		+ points[1] * polynom[1]
		+ points[2] * polynom[2]
		+ points[3] * polynom[3];

	res *= 0.5;
	return res;
}

// evaluate all parts of the curve for duck movemenet
glm::vec3 splineHandler::evaluateClosedCurve(const glm::vec3 points[], const size_t count, const float t) {
	glm::vec3 result(0.0, 0.0, 0.0);

	int i = (int)(t*count);
	glm::vec3 newPoints[4];

	newPoints[0] = points[(i - 1 + count) % count];
	newPoints[1] = points[i % count];
	newPoints[2] = points[(i + 1) % count];
	newPoints[3] = points[(i + 2) % count];

	result = evaluateCurveSegment(newPoints, t * count - i);

	return result;
}

 // evaluate first derivate of the whole curve for duck movement
glm::vec3 splineHandler::evalClosedCurveFirstDev(const glm::vec3 points[], const size_t count, const float t) {
	int i = (int)(t*count);
	glm::vec3 newPoints[4];
	newPoints[0] = points[(i - 1 + count) % count];
	newPoints[1] = points[i % count];
	newPoints[2] = points[(i + 1) % count];
	newPoints[3] = points[(i + 2) % count];
	glm::vec3 result = evalCurveSegFirstDev( newPoints, t - i );
	return -result;
}

// SPLINE FOR MOVEMENT OF CAM - HERMITE SPLINE

// Hermite interpolation between two points
static glm::vec3 hermite(const glm::vec3& p0, const glm::vec3& m0, const glm::vec3& p1, const glm::vec3& m1, float t) {
	float t2 = t * t;
	float t3 = t2 * t;

	float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
	float h10 = t3 - 2.0f * t2 + t;
	float h01 = -2.0f * t3 + 3.0f * t2;
	float h11 = t3 - t2;

	return h00 * p0 + h10 * m0 + h01 * p1 + h11 * m1;
}

// Derivative of Hermite interpolation
static glm::vec3 hermiteDerivative(const glm::vec3& p0, const glm::vec3& m0, const glm::vec3& p1, const glm::vec3& m1, float t) {
	float t2 = t * t;

	float dh00 = 6.0f * t2 - 6.0f * t;
	float dh10 = 3.0f * t2 - 4.0f * t + 1.0f;
	float dh01 = -6.0f * t2 + 6.0f * t;
	float dh11 = 3.0f * t2 - 2.0f * t;

	return dh00 * p0 + dh10 * m0 + dh01 * p1 + dh11 * m1;
}

// Calculate tangent as half vector between neighbors
static glm::vec3 calcTangent(const glm::vec3& prev, const glm::vec3& next) {
	return 0.5f * (next - prev);
}

glm::vec3 splineHandler::evaluateMovementCurve(const glm::vec3 points[], const size_t count, const float t) {
    if (count < 2) {
        return glm::vec3(0.0f);
    }

    float total = (float)count;
    float scaledT = t * total;
    int i = (int)scaledT;
    float localT = scaledT - i;

    int i0 = i % count;
    int i1 = (i + 1) % count;
    int iPrev = (i - 1 + count) % count;
    int iNext = (i + 2) % count;

    glm::vec3 p0 = points[i0];
    glm::vec3 p1 = points[i1];

    glm::vec3 m0 = calcTangent(points[iPrev], p1);
    glm::vec3 m1 = calcTangent(p0, points[iNext]);

    return hermite(p0, m0, p1, m1, localT);
}

glm::vec3 splineHandler::evalMovementCurveFirstDev(const glm::vec3 points[], const size_t count, const float t) {
	if (count < 2) {
		return glm::vec3(0.0f);
	}

	float total = (float)count;
	float scaledT = t * total;
	int i = (int)scaledT;
	float localT = scaledT - i;

	int i0 = i % count;
	int i1 = (i + 1) % count;
	int iPrev = (i - 1 + count) % count;
	int iNext = (i + 2) % count;

	glm::vec3 p0 = points[i0];
	glm::vec3 p1 = points[i1];

	glm::vec3 m0 = calcTangent(points[iPrev], p1);
	glm::vec3 m1 = calcTangent(p0, points[iNext]);

	return hermiteDerivative(p0, m0, p1, m1, localT);
}