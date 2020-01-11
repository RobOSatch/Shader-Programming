#include "KDNode.h"

KDNode::KDNode(vector<Vertex> vertices)
{
	this->vertices = vertices;
	this->getNodeAABB();
}

void KDNode::getNodeAABB()
{
	if (vertices.size() == 0) return;
	AABB* bb = new AABB(vertices[0].Position, vertices[0].Position);

	for (Vertex v : vertices) {
		bb->include(v.Position);
	}

	aabb = bb;
}

float KDNode::getAxisMedian()
{
	vector<float> axisCoordinates;

	for (Vertex v : vertices) {
		if (axis == 0) axisCoordinates.push_back(v.Position.x);
		else if (axis == 1) axisCoordinates.push_back(v.Position.y);
		else axisCoordinates.push_back(v.Position.z);
	}

	std::nth_element(axisCoordinates.begin(), axisCoordinates.begin() + axisCoordinates.size() / 2, axisCoordinates.end());
	return axisCoordinates[axisCoordinates.size() / 2];
}

KDSplit KDNode::getKDSplit(float splitPosition)
{
	KDSplit kdSplit;

	for (Vertex v : vertices) {
		switch (axis) {
		case 0:
			if (v.Position.x < splitPosition) kdSplit.leftSplit.push_back(v);
			else kdSplit.rightSplit.push_back(v);
			break;
		case 1:
			if (v.Position.y < splitPosition) kdSplit.leftSplit.push_back(v);
			else kdSplit.rightSplit.push_back(v);
			break;
		case 2:
			if (v.Position.z < splitPosition) kdSplit.leftSplit.push_back(v);
			else kdSplit.rightSplit.push_back(v);
			break;
		}
	}

	return kdSplit;
}
