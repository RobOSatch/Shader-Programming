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
	if (node->vertices.size() <= 10 || depth > 50) {
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

void KDTree::VisitNodes(KDNode* pNode, glm::vec3 a, glm::vec3 d, float tMax)
{
	// Check if arrived at leaf node. If leaf node is not empty -> hit
	if (pNode->left == nullptr && pNode->right == nullptr) {
		if (pNode->vertices.size() != 0) pNode->hit = true;
		
		return;
	}

	int dim = pNode->axis;
	bool first = a[dim] > pNode->axisPos;

	if (d[dim] == 0.0f) {
		// line segment parallel to splitting plane, visit near side only
		if (first) VisitNodes(pNode->left, a, d, tMax);
		else VisitNodes(pNode->right, a, d, tMax);
	}
	else {
		// find t value for intersection
		float t = (pNode->axisPos - a[dim]) / d[dim];
		if (0.0f <= t && t < tMax) {
			if (first) {
				VisitNodes(pNode->left, a, d, t);
				VisitNodes(pNode->right, a, d, t);
			}
			else {
				VisitNodes(pNode->right, a, d, t);
				VisitNodes(pNode->left, a, d, t);
			}
		}
		else {
			if (first) VisitNodes(pNode->left, a, d, tMax);
			else VisitNodes(pNode->right, a, d, tMax);
		}
	}
}

void KDTree::intersectWith(Ray ray)
{
	VisitNodes(root, ray.origin, ray.direction, ray.tMax);
}
