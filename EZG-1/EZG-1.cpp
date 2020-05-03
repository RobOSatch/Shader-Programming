#include "glad/glad.h"
#include "glfw/glfw3.h"

#include "ImageLoader.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"

#include "Shader.h"
#include "Camera.h"

#include "interpolation.h"
#include "Timer.h"

#include "triangulation.h"

#include <time.h>
#include <valarray>
#include <random>
#include<algorithm>

#include <iostream>

enum Game_Mode {
	CREATE,
	RIDE
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
unsigned int loadTexture(const char* path);
void renderScene(const Shader& shader);
void renderCube();
void renderQuad();
void renderWalls();

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(-50.0, 50.0, -50.0));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// meshes
unsigned int VAO, VBO, EBO;

// textures
unsigned int diffuseX, diffuseY, diffuseZ;
unsigned int displacementX, displacementY, displacementZ;
unsigned int normalZ;

float speed = 0.11f;

bool visualize = false;

Game_Mode gameMode = CREATE;

std::vector<glm::vec3> waypoints;
std::vector<glm::quat> orientations;

// Textures
unsigned int boxTexture;
unsigned int wallTexture;
unsigned int normalMap;
unsigned int secondTexture;
unsigned int secondNormalMap;

glm::vec3 lightPos(0.0f, 5.0f, 6.0f);

float bumpiness = 1.0f;

// MSAA
int samples = 4;
int previousSamples = 4;
bool msaa = true;
bool shouldQuit = false;
glm::vec2 mousePos = glm::vec2(0.0f, 0.0f);
int AAMode = 0;
int previousAAMode = 0;

bool isFirstRender = true;

Shader *lämpShader;

// Displacement constants
float heightScale = 0.05;
unsigned int primaryLayers = 10;
unsigned int secondaryLayers = 5;
float normalLevel = 0.3f;

// Marching Cubes stuff
Shader *marchingCubesShader;
Shader *densityComputeShader;
Shader* displacementShader;
GLuint densityTextureA;
GLuint densityTextureB;
GLuint noiseTexture;
unsigned int textureWidth = 96;
unsigned int textureHeight = 96;
unsigned int textureDepth = 256;

unsigned int quadVAO, quadVBO;
GLenum* DrawBuffers = new GLenum[2];
bool wireframeMode = false;

std::vector<float> points;
std::vector<float> noiseFloats;
float pointScale = 1;

GLuint mcTableTexture;
GLuint mcTableBuffer;

int cameraSector = 0;
int previousCameraSector = 0;
int reloadUpperSectorBound = 150;
int reloadLowerSectorBound = -150;
int cameraSectorHeight = 255;
bool reload = false;

float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	   // positions   // texCoords
	   -1.0f,  1.0f,  0.0f, 1.0f,
	   -1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

	   -1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
};

// Return random float
float getRandomNumber(float min, float max)
{
	return ((float(rand()) / float(RAND_MAX)) * (max - min)) + min;
}

// Load Shaders
void loadShaders()
{
	reload = true;
	marchingCubesShader = new Shader("Shaders/vertexShader.glsl", "Shaders/fragmentShader.glsl", "Shaders/geometryShader.glsl");
	densityComputeShader = new Shader("Shaders/densityCS.glsl");
	displacementShader = new Shader("Shaders/displacementVS.glsl", "Shaders/displacementPS.glsl");
}

void renderQuad()
{
	if (quadVAO == 0)
	{
		// positions
		glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
		glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
		glm::vec3 pos3(1.0f, -1.0f, 0.0f);
		glm::vec3 pos4(1.0f, 1.0f, 0.0f);
		// texture coordinates
		glm::vec2 uv1(0.0f, 1.0f);
		glm::vec2 uv2(0.0f, 0.0f);
		glm::vec2 uv3(1.0f, 0.0f);
		glm::vec2 uv4(1.0f, 1.0f);
		// normal vector
		glm::vec3 nm(0.0f, 0.0f, 1.0f);

		// calculate tangent/bitangent vectors of both triangles
		glm::vec3 tangent1, bitangent1;
		glm::vec3 tangent2, bitangent2;
		// triangle 1
		// ----------
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent1 = glm::normalize(tangent1);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent1 = glm::normalize(bitangent1);

		// triangle 2
		// ----------
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;

		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent2 = glm::normalize(tangent2);


		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent2 = glm::normalize(bitangent2);


		float quadVertices[] = {
			// positions            // normal         // texcoords  // tangent                          // bitangent
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
		};
		// configure plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

int main()
{
	srand(time(NULL));

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Camera Ride", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glClearColor(0.0, 0.0, 0.0, 0.0);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// Initialize points cloud
	for (int i = 0; i < textureWidth - 1; ++i)
	{
		for (int j = 0; j < textureHeight - 1; ++j)
		{
			int index = i * textureWidth + j;
			points.push_back(float(i) * pointScale);
			points.push_back(float(j) * pointScale);
		}
	}
	
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), &points[0], GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	// Creates the two density textures
	loadShaders();
	glEnable(GL_TEXTURE_3D);
	glGenTextures(1, &densityTextureA);
	glBindTexture(GL_TEXTURE_3D, densityTextureA);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, textureWidth, textureHeight, textureDepth, 0, GL_RED, GL_UNSIGNED_SHORT, NULL);
	glBindImageTexture(0, densityTextureA, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R16F);

	glGenTextures(1, &densityTextureB);
	glBindTexture(GL_TEXTURE_3D, densityTextureB);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, textureWidth, textureHeight, textureDepth, 0, GL_RED, GL_UNSIGNED_SHORT, NULL);
	glBindImageTexture(0, densityTextureB, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R16F);

	// Noise generation
	for (int i = 0; i < (16 * 16 * 16); i++) {
		noiseFloats.push_back(0.0); // R
		noiseFloats.push_back(0.0); // G
		noiseFloats.push_back(0.0); // B
		noiseFloats.push_back(0.0); // A
	}

	// Generates noise 3D texture
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_3D, noiseTexture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, 16, 16, 16, 0, GL_RGBA, GL_HALF_FLOAT, &noiseFloats[0]);
	glBindImageTexture(1, noiseTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA16F);

	//Create Table Buffer
	glGenBuffers(1, &mcTableBuffer);
	glBindBuffer(GL_TEXTURE_BUFFER, mcTableBuffer);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(triTable), triTable, GL_STATIC_DRAW);
	//glBufferStorage(GL_TEXTURE_BUFFER, 4096 * sizeof(GLuint), &triTable, GL_MAP_READ_BIT);
	//
	//Create Table Texture
	glGenTextures(1, &mcTableTexture);
	glBindTexture(GL_TEXTURE_BUFFER, mcTableTexture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, mcTableBuffer);
	
	//glBindTexture(GL_TEXTURE_3D, 0);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//____________________TEXTURES__________________

	int width, height, nrChannels;

	unsigned char* data = ImageLoader::loadImageData("textures/rock_1_Albedo.jpg", &width, &height, &nrChannels, 0);
	glGenTextures(1, &diffuseX);
	ImageLoader::setDefault2DTextureFromData(diffuseX, width, height, data);
	ImageLoader::freeImage((data));


	data = ImageLoader::loadImageData("textures/rock_2_Albedo.jpg", &width, &height, &nrChannels, 0);
	glGenTextures(1, &diffuseY);
	ImageLoader::setDefault2DTextureFromData(diffuseY, width, height, data);
	ImageLoader::freeImage((data));

	data = ImageLoader::loadImageData("textures/rock_3_Albedo.jpg", &width, &height, &nrChannels, 0);
	glGenTextures(1, &diffuseZ);
	ImageLoader::setDefault2DTextureFromData(diffuseZ, width, height, data);
	ImageLoader::freeImage((data));

	data = ImageLoader::loadImageData("textures/rock_1_Displacement.jpg", &width, &height, &nrChannels, 0);
	glGenTextures(1, &displacementX);
	ImageLoader::setDefault2DTextureFromData(displacementX, width, height, data);
	ImageLoader::freeImage((data));

	data = ImageLoader::loadImageData("textures/rock_2_Displacement.jpg", &width, &height, &nrChannels, 0);
	glGenTextures(1, &displacementY);
	ImageLoader::setDefault2DTextureFromData(displacementY, width, height, data);
	ImageLoader::freeImage((data));

	data = ImageLoader::loadImageData("textures/rock_3_Displacement.jpg", &width, &height, &nrChannels, 0);
	glGenTextures(1, &displacementZ);
	ImageLoader::setDefault2DTextureFromData(displacementZ, width, height, data);
	ImageLoader::freeImage((data));

	data = ImageLoader::loadImageData("textures/rock_3_Normal.jpg", &width, &height, &nrChannels, 0);
	glGenTextures(1, &normalZ);
	ImageLoader::setDefault2DTextureFromData(normalZ, width, height, data);
	ImageLoader::freeImage((data));


	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		if (wireframeMode)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		// Adjusts the camera sector, depending on the position
		if (camera.Position.z < (cameraSector * cameraSectorHeight) + reloadLowerSectorBound)
		{
			cameraSector--;
		}
		if (camera.Position.z > (cameraSector * cameraSectorHeight) + reloadUpperSectorBound)
		{
			cameraSector++;
		}

		if (reload || previousCameraSector != cameraSector)
		{
			reload = false;
			previousCameraSector = cameraSector;
			densityComputeShader->use();

			// First pass for density texture A
			glActiveTexture(GL_TEXTURE0);
			densityComputeShader->setInt("texturePosition", cameraSector);
			glBindTexture(GL_TEXTURE_3D, densityTextureA);
			glBindImageTexture(0, densityTextureA, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R16F);

			glDispatchCompute(textureWidth, textureHeight, textureDepth);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			
			// Second pass for density texture B
			densityComputeShader->setInt("texturePosition", cameraSector - 1);
			glBindTexture(GL_TEXTURE_3D, densityTextureB);
			glBindImageTexture(0, densityTextureB, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R16F);

			glDispatchCompute(textureWidth, textureHeight, textureDepth);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		// render
		// ------
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 300.0f);
		glm::mat4 view = camera.GetViewMatrix();

		// Configuring of marching cube shader
		marchingCubesShader->use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, densityTextureA);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, diffuseX);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, diffuseY);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, diffuseZ);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, displacementX);

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, displacementY);

		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, displacementZ);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, mcTableTexture);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, mcTableBuffer);

		marchingCubesShader->setVec3("densityTextureDimensions", textureWidth, textureHeight, textureDepth);
		marchingCubesShader->setMat4("projection", projection);
		marchingCubesShader->setMat4("view", view);
		marchingCubesShader->setVec3("viewPos", camera.Position);
		glm::mat4 model = glm::mat4(1.0f);
		marchingCubesShader->setMat4("model", model);
		glBindVertexArray(VAO);

		// First pass for density texture A
		marchingCubesShader->setInt("cameraSector", cameraSector);
		glBindTexture(GL_TEXTURE_3D, densityTextureA);
		glDrawArraysInstanced(GL_POINTS, 0, (textureWidth - 1) * (textureHeight - 1), textureDepth);

		// Second pass for density texture B
		marchingCubesShader->setInt("cameraSector", cameraSector - 1);
		glBindTexture(GL_TEXTURE_3D, densityTextureB);
		glDrawArraysInstanced(GL_POINTS, 0, (textureWidth - 1) * (textureHeight - 1), textureDepth - 1);

		// Displacement mapping
		displacementShader->use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseZ);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, displacementZ);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, normalZ);
		displacementShader->setInt("diffuseMap", 0);
		displacementShader->setInt("displacementMap", 1);
		displacementShader->setInt("normalMap", 2);
		displacementShader->setInt("cameraSector", cameraSector);
		displacementShader->setFloat("heightScale", heightScale);
		displacementShader->setFloat("normalLevel", normalLevel);
		displacementShader->setInt("primaryLayers", primaryLayers);
		displacementShader->setInt("secondaryLayers", secondaryLayers);
		displacementShader->setMat4("projection", projection);
		displacementShader->setMat4("view", view);
		model = glm::translate(model, glm::vec3(0, 0, -1));
		model = glm::scale(model, glm::vec3(10));
		displacementShader->setMat4("model", model);
		displacementShader->setVec3("viewPos", camera.Position);

		renderQuad();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();

	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow * window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		shouldQuit = true;
		glfwSetWindowShouldClose(window, true);
	}

	if (gameMode == CREATE) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			camera.ProcessKeyboard(UP, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			camera.ProcessKeyboard(DOWN, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		speed = fmin(0.5f, speed + 0.0001f);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		speed = fmax(0.0f, speed - 0.0001f);

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		bumpiness = fmin(1.0f, bumpiness + 0.02f);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		bumpiness = fmax(0.0f, bumpiness - 0.02f);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS && gameMode == CREATE) {
		gameMode = RIDE;
		camera.Position = waypoints[0];
		camera.Orientation = orientations[0];
	}

	if (key == GLFW_KEY_V && action == GLFW_PRESS) {
		visualize = !visualize;
	}

	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
		msaa = !msaa;
	}

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) wireframeMode = !wireframeMode;
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) loadShaders(); // Shader hot reloading
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && gameMode == CREATE) {
		waypoints.push_back(camera.Position);
		orientations.push_back(camera.Orientation);
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow * window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;

		xpos = mousePos.x;
		ypos = mousePos.y;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	if (gameMode == CREATE) {
		camera.ProcessMouseMovement(xoffset, yoffset);
	}

	mousePos = glm::vec2(xpos, ypos);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
	return 0;
}