#pragma once
#include "SceneObject.h"

class Scene
{
public:
	vector<Triangle> triangles;


	Scene() {}

	vector<Triangle> getWorldTriangles()
	{
		vector<Triangle> triangles;

		for (SceneObject sceneObject : mSceneObjects) {
			for (Mesh mesh : sceneObject.model->meshes) {
				vector<Vertex> vertices = sceneObject.getVerticesInWorld();
				// TODO
			}
		}
	}

	vector<SceneObject> getSceneObjects()
	{
		return mSceneObjects;
	}
	
	void addSceneObject(SceneObject sceneObj)
	{
		mSceneObjects.emplace_back(sceneObj);
	}

	void addSceneObject(Model* model, glm::mat4 transMat, float normalLevel, Shader* usingShader)
	{
		auto* temp = new SceneObject(model, transMat, normalLevel, usingShader);
		mSceneObjects.emplace_back(*temp);
	}

	void draw() {
		for (SceneObject so : mSceneObjects)
		{
			so.draw();
		}
	}

	void draw(Shader* usingShader) {
		for (SceneObject so : mSceneObjects)
		{
			so.draw(usingShader);
		}
	}

	vector<AABB> getAllAABB()
	{
		vector<AABB> tempVec;
		for (SceneObject so : mSceneObjects)
		{
			tempVec.emplace_back(so.getAABB());
		}
		return tempVec;
	}

	vector<Vertex> getAllVertices()
	{
		vector<Vertex> vertices;

		for (SceneObject sO : mSceneObjects)
		{
			vector<Vertex> tempVertices = sO.getVerticesInWorld();
			vertices.reserve(vertices.size() + tempVertices.size()); // preallocate memory
			vertices.insert(vertices.end(), tempVertices.begin(), tempVertices.end());
		}
		return vertices;
	}


private:
	vector<SceneObject> mSceneObjects;
};
