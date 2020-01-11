#include "KDTree.h"

KDTree::KDTree()
{
}

KDTree::KDTree(Scene* scene)
{
	vector<Vertex> vertices = scene->getAllVertices();
	root = new KDNode(vertices);
}

void KDTree::construct(KDNode* node, int depth)
{
	if (node->vertices.size() <= 50 || depth > 10) {
		return;
	}

	// Get Axis with maximum distance and then get the median vertex position in respect to the axis.
	node->axis = node->aabb->getMaxExtentAxis();
	node->axisPos = node->getAxisMedian();

	KDSplit split = node->getKDSplit(node->axisPos);
	KDNode* left = new KDNode(split.leftSplit);
	KDNode* right = new KDNode(split.rightSplit);

	++depth;
	node->left = left;
	node->right = right;
	return;
}
