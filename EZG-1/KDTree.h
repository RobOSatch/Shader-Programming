#pragma once
#include "Scene.h";
#include "KDNode.h"
#include "Ray.h"

class KDTree
{
public:
	KDNode* root;
	AABB godBox;

	KDTree();
	KDTree(Scene* scene);
	
	void construct(KDNode* node, int depth);
	void VisitNodes(KDNode* pNode, glm::vec3 a, glm::vec3 d, float tMax);
	void intersectWith(Ray ray);

private:

};

