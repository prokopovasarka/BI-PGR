//-----------------------------------------------------------------------------------------
/**
 * \file       spline.h
 * \author     Šárka Prokopová
 * \date       2025/4/28
 * \brief      Everything for computing spline
 *
*/
//-----------------------------------------------------------------------------------------
#ifndef __SPLINE_H
#define __SPLINE_H

#include "pgr.h" 

class splineHandler {
public:
	static bool vectorLen(const glm::vec3& vect);

	static glm::mat4 alignObject(const glm::vec3& position, const glm::vec3& front, const glm::vec3& up);
	glm::vec3 evalCurveSegFirstDev(const glm::vec3 points[], const float t);
	glm::vec3 evaluateCurveSegment(const glm::vec3 points[], const float t);

	glm::vec3 evaluateClosedCurve(
		const glm::vec3 points[],
		const size_t    count,
		const float     t
	);

	glm::vec3 evalClosedCurveFirstDev(
		const glm::vec3 points[],
		const size_t    count,
		const float     t
	);

	glm::vec3 splineHandler::evalMovementCurveFirstDev(const glm::vec3 points[], const size_t count, const float t);
	glm::vec3 splineHandler::evaluateMovementCurve(const glm::vec3 points[], const size_t count, const float t);

};
#endif
