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
	if (node->vertices.size() <= 10 || depth > 10) {
		return;
	}

	node->axis = node->aabb->getMaxExtentAxis();
	node->axisPos = node->getAxisMedian();

	KDSplit split = node->getKDSplit(node->axisPos);
	KDNode* left = new KDNode(split.leftSplit);
	KDNode* right = new KDNode(split.rightSplit);

	depth++;
	node->left = left;
	node->right = right;

	node->aabb->splitAABB(node->left->aabb, node->right->aabb, node->axis, node->axisPos);

	construct(left, depth);
	construct(right, depth);

	return;
}
