#pragma once
#include "glm/glm.hpp"
#include <array>
#include <vector>
#include <cmath>

class AABB
{
public:
	glm::vec3 mMinPoint;
	glm::vec3 mMaxPoint;
	AABB()
	{
		mMinPoint = glm::vec3(0);
		mMaxPoint = glm::vec3(0);
	}
	AABB(glm::vec3 mMinPoint, glm::vec3 mMaxPoint) : mMinPoint(mMinPoint), mMaxPoint(mMaxPoint) { }

	~AABB() { }
	void include(glm::vec3 p)
	{
		if (mMaxPoint.x < p.x)
		{
			mMaxPoint.x = p.x;
		}

		if (mMaxPoint.y < p.y)
		{
			mMaxPoint.y = p.y;
		}

		if (mMaxPoint.z < p.z)
		{
			mMaxPoint.z = p.z;
		}

		if (mMinPoint.x > p.x)
		{
			mMinPoint.x = p.x;
		}

		if (mMinPoint.y > p.y)
		{
			mMinPoint.y = p.y;
		}

		if (mMinPoint.z > p.z)
		{
			mMinPoint.z = p.z;
		}
	}

	bool isOverlappingAABB(AABB* other)
	{
		return (other->mMaxPoint.x > mMinPoint.x&& other->mMaxPoint.y > mMinPoint.y&& other->mMaxPoint.z > mMinPoint.z) ||
			(other->mMinPoint.x <= mMaxPoint.x && other->mMinPoint.y <= mMaxPoint.y && other->mMinPoint.z <= mMaxPoint.z);
	}

	void include(const AABB& b)
	{
		include(b.mMinPoint);
		include(b.mMaxPoint);
	}

	int getMaxExtentAxis()
	{
		glm::vec3 tempVec = mMaxPoint - mMinPoint;
		if (tempVec.x >= tempVec.y && tempVec.x >= tempVec.z)
		{
			return 0;
		}

		if (tempVec.y >= tempVec.z)
		{
			return 1;
		}

		return 2;
	}

	void splitAABB(AABB* lower, AABB* higher, int axis, float pos)
	{
		glm::vec3 lowCutPlanePos = mMinPoint;
		glm::vec3 highCutPlanePos = mMaxPoint;

		switch (axis) {
		case 0:
			lowCutPlanePos.x = pos;
			highCutPlanePos.x = pos;
			break;
		case 1:
			lowCutPlanePos.y = pos;
			highCutPlanePos.y = pos;
			break;
		case 2:
			lowCutPlanePos.z = pos;
			highCutPlanePos.z = pos;
			break;
		default:
			//std::cout << "Wrong Dimension";
			break;
		}
		lower->mMinPoint = mMinPoint;
		lower->mMaxPoint = highCutPlanePos;
		higher->mMinPoint = lowCutPlanePos;
		higher->mMaxPoint = mMaxPoint;

	}


	//bool intersect(const Ray& r, float& tmin, float& tmax) const;
};