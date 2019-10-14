#include "glm/gtx/quaternion.hpp"

namespace interpolation {
	/**
	*
	* CATMULL TIME
	* Calculates the point for the Catmull-Rom Spline located between points p1 and p2 at t.
	*
	* PARAMS:
	* p0 - The point before the curve
	* p1 - The starting point of the curve
	* p2 - The end point of the curve
	* p3 - The point after the curve
	* t - The interpolation parameter, ranging from 0 to 1, where 0 is the start and 1 is the end of the curve
	*
	*/
	glm::vec3 catmullRomSpline(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t) {
		float a = 0.0f; // Tension
		float b = 0.0f; // Bias
		float c = 0.0f; // Continuity

		glm::vec3 sourceVec1 = p1 - p0;
		glm::vec3 destinationVec1 = p2 - p1;
		glm::vec3 sourceVec2 = p2 - p1;
		glm::vec3 destinationVec2 = p3 - p2;
		glm::vec3 sourceTangent = (1.0f - a) * (1.0f + b) * (1.0f - c) / 2.0f * sourceVec1 + (1.0f - a) * (1.0f - b) * (1.0f + c) * destinationVec1;
		glm::vec3 destinationTangent = (1.0f - a) * (1.0f + b) * (1.0f + c) / 2.0f * sourceVec2 + (1.0f - a) * (1.0f - b) * (1.0f - c) * destinationVec2;

		glm::vec3 point0 = (2.0f * pow(t, 3) - 3.0f * pow(t, 2) + 1.0f) * p1;
		glm::vec3 m0 = (pow(t, 3) - 2.0f * pow(t, 2) + t) * sourceTangent;
		glm::vec3 m1 = (pow(t, 3) - pow(t, 2)) * destinationTangent;
		glm::vec3 point1 = (-2.0f * pow(t, 3) + 3.0f * pow(t, 2)) * p2;

		return point0 + m0 + m1 + point1;
	}

	/**
	*
	* SQUAD TIME
	* Calculates the SQUAD-interpolated orientation for points q1 and q2 at t.
	*
	* PARAMS:
	* q0 - The orientation before the starting one
	* q1 - The starting orientation
	* q2 - The final orientation
	* q3 - The orientation after the final one
	* t - The interpolation parameter, ranging from 0 to 1, where 0 is the start and 1 is the end of the interpolation
	*/
	glm::quat squad(glm::quat q0, glm::quat q1, glm::quat q2, glm::quat q3, float t) {
		glm::quat intermediate1 = glm::intermediate(q0, q1, q2);
		glm::quat intermediate2 = glm::intermediate(q1, q2, q3);
		return glm::squad(q1, q2, intermediate1, intermediate2, t);
	}
}