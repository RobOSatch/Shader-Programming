#pragma once
#include "Scene.h";
#include "KDNode.h"

class KDTree
{
public:
	KDNode* root;
	AABB godBox;

	KDTree();
	KDTree(Scene* scene);
	
	void construct(KDNode* node, int depth);
	

private:

};

