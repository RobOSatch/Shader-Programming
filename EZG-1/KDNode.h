#pragma once
#include "Scene.h"

struct KDSplit
{
	vector<Vertex> leftSplit;
	vector<Vertex> rightSplit;
};

class KDNode
{
public:
	int axis;
	float axisPos;
	KDNode* left;
	KDNode* right;
	vector<Vertex> vertices;
	AABB* aabb;

	KDNode(vector<Vertex> vertices);
	
	void getNodeAABB();
	float getAxisMedian();
	KDSplit getKDSplit(float splitPosition);

private:

};

