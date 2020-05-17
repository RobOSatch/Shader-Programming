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

#include "stb/stb_image.h"

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
void SetupParticles();

const unsigned int MAX_PARTICLES = 100000;
const unsigned int EMITTER_COUNT = 10;

Shader* particleRenderShader;
Shader* particleTransformShader;
unsigned int particleVBO[2];
unsigned int particleTFB[2];
//unsigned int particleVAO;
unsigned int randomTexture;

bool spawnParticles = false;
glm::vec3 spawnParticlePosition = glm::vec3(0.f, 0.f, 0.f);

int currVB = 0;
int currTFB = 1;

struct particlestruct
{
	glm::vec3 position;
	glm::vec3 velocity;
	float lifeTime;
	float type;
};

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 0.0f));
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

// Basic shaders
Shader* basicShader;

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

GLenum err;

// meshes
unsigned int planeVAO;

// Variance Shadow Mapping
Shader* storeDepthShader;
Shader* VSMShader;
Shader* simpleDepthShader;
Shader* debugShader;
unsigned int depthMap;
unsigned int depthMapFBO;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);

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

	if (marchingCubesShader != nullptr)
	{
		delete marchingCubesShader;
		delete displacementShader;
		delete particleRenderShader;
		delete particleTransformShader;
		delete densityComputeShader;
	}

	marchingCubesShader = new Shader("Shaders/vertexShader.glsl", "Shaders/fragmentShader.glsl", "Shaders/geometryShader.glsl");
	displacementShader = new Shader("Shaders/displacementVS.glsl", "Shaders/displacementPS.glsl");

	// Particle shader
	particleRenderShader = new Shader("Shaders/particleRenderVS.glsl", "Shaders/particleRenderPS.glsl", "Shaders/particleRenderGS.glsl");

	const GLchar* varyings[4];
	varyings[0] = "outPosition";
	varyings[1] = "outVelocity";
	varyings[2] = "outLifeTime";
	varyings[3] = "outType";

	particleTransformShader = new Shader("Shaders/particleTransformVS.glsl", "Shaders/particleTransformPS.glsl", "Shaders/particleTransformGS.glsl", varyings, 4);

	densityComputeShader = new Shader("Shaders/densityCS.glsl");

	// Shadow Mapping
	storeDepthShader = new Shader("Shaders/storeDepthVS.glsl", "Shaders/storeDepthPS.glsl");
	VSMShader = new Shader("Shaders/varianceShadowMapVS.glsl", "Shaders/varianceShadowMapPS.glsl");
	basicShader = new Shader("Shaders/basicVS.glsl", "Shaders/basicPS.glsl");
	simpleDepthShader = new Shader("Shaders/loglVS.glsl", "Shaders/loglPS.glsl");
	
	debugShader = new Shader("Shaders/debugVS.glsl", "Shaders/debugPS.glsl");

	VSMShader->use();
	VSMShader->setInt("diffuseTexture", 0);
	VSMShader->setInt("shadowMap", 1);
	debugShader->use();
	debugShader->setInt("depthMap", 0);
}

void SetupParticles()
{
	auto* particles = new particlestruct[MAX_PARTICLES];
	for (int i = 0; i < EMITTER_COUNT; i++)
	{
		//position
		particles[i].position = glm::vec3(i / 10.f, 1.f, 0.f);

		//velocity
		particles[i].velocity = glm::vec3(0.f, 0.f, 0.f);

		//lifetime
		particles[i].lifeTime = float(i) / 10.f;

		//type
		particles[i].type = 0.f;
	}



	glGenBuffers(2, particleVBO);
	glGenTransformFeedbacks(2, particleTFB);
	for (unsigned int i = 0; i < 2; i++) {
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, particleTFB[i]);
		glBindBuffer(GL_ARRAY_BUFFER, particleVBO[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(particlestruct) * MAX_PARTICLES, particles, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particleVBO[i]);
	}

	delete[] particles;

	/*glGenVertexArrays(1, &particleVAO);
	glBindVertexArray(particleVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8, (void*)(sizeof(float) * 6));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8, (void*)(sizeof(float) * 7));
	glBindVertexArray(0);*/
}

void SetupFBOs()
{
	glGenFramebuffers(1, &depthMapFBO);

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glFrontFace(GL_CW);

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

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float planeVertices[] = {
		// positions            // normals         // texcoords
		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		 25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};
	// plane VAO
	unsigned int planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	/*glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), &points[0], GL_STATIC_DRAW);*/

	// position attribute
	/*glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);*/

	// Creates the two density textures
	SetupFBOs();
	loadShaders();
	//glEnable(GL_TEXTURE_3D);
	//glGenTextures(1, &densityTextureA);
	//glBindTexture(GL_TEXTURE_3D, densityTextureA);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, textureWidth, textureHeight, textureDepth, 0, GL_RED, GL_UNSIGNED_SHORT, NULL);
	//glBindImageTexture(0, densityTextureA, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R16F);

	//glGenTextures(1, &densityTextureB);
	//glBindTexture(GL_TEXTURE_3D, densityTextureB);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, textureWidth, textureHeight, textureDepth, 0, GL_RED, GL_UNSIGNED_SHORT, NULL);
	//glBindImageTexture(0, densityTextureB, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R16F);

	//// Noise generation
	//for (int i = 0; i < (16 * 16 * 16); i++) {
	//	noiseFloats.push_back(0.0); // R
	//	noiseFloats.push_back(0.0); // G
	//	noiseFloats.push_back(0.0); // B
	//	noiseFloats.push_back(0.0); // A
	//}

	//// Generates noise 3D texture
	//glGenTextures(1, &noiseTexture);
	//glBindTexture(GL_TEXTURE_3D, noiseTexture);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, 16, 16, 16, 0, GL_RGBA, GL_HALF_FLOAT, &noiseFloats[0]);
	//glBindImageTexture(1, noiseTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA16F);

	////Create Table Buffer
	//glGenBuffers(1, &mcTableBuffer);
	//glBindBuffer(GL_TEXTURE_BUFFER, mcTableBuffer);
	//glBufferData(GL_TEXTURE_BUFFER, sizeof(triTable), triTable, GL_STATIC_DRAW);
	////glBufferStorage(GL_TEXTURE_BUFFER, 4096 * sizeof(GLuint), &triTable, GL_MAP_READ_BIT);
	////
	////Create Table Texture
	//glGenTextures(1, &mcTableTexture);
	//glBindTexture(GL_TEXTURE_BUFFER, mcTableTexture);
	//glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, mcTableBuffer);

	//glBindTexture(GL_TEXTURE_3D, 0);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//____________________TEXTURES__________________
	unsigned int woodTexture = loadTexture("textures/wood.png");

	/*int width, height, nrChannels;

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
	glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

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

		//if (reload || previousCameraSector != cameraSector)
		//{
		//	reload = false;
		//	previousCameraSector = cameraSector;
		//	densityComputeShader->use();

		//	// First pass for density texture A
		//	glActiveTexture(GL_TEXTURE0);
		//	densityComputeShader->setInt("texturePosition", cameraSector);
		//	glBindTexture(GL_TEXTURE_3D, densityTextureA);
		//	glBindImageTexture(0, densityTextureA, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R16F);

		//	glDispatchCompute(textureWidth, textureHeight, textureDepth);
		//	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		//	// Second pass for density texture B
		//	densityComputeShader->setInt("texturePosition", cameraSector - 1);
		//	glBindTexture(GL_TEXTURE_3D, densityTextureB);
		//	glBindImageTexture(0, densityTextureB, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R16F);

		//	glDispatchCompute(textureWidth, textureHeight, textureDepth);
		//	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		//}

		// render
		// ------
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 300.0f);
		glm::mat4 view = camera.GetViewMatrix();

		// Configuring of marching cube shader
		//marchingCubesShader->use();

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_3D, densityTextureA);

		//glActiveTexture(GL_TEXTURE2);
		//glBindTexture(GL_TEXTURE_2D, diffuseX);

		//glActiveTexture(GL_TEXTURE3);
		//glBindTexture(GL_TEXTURE_2D, diffuseY);

		//glActiveTexture(GL_TEXTURE4);
		//glBindTexture(GL_TEXTURE_2D, diffuseZ);

		//glActiveTexture(GL_TEXTURE5);
		//glBindTexture(GL_TEXTURE_2D, displacementX);

		//glActiveTexture(GL_TEXTURE6);
		//glBindTexture(GL_TEXTURE_2D, displacementY);

		//glActiveTexture(GL_TEXTURE7);
		//glBindTexture(GL_TEXTURE_2D, displacementZ);

		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_BUFFER, mcTableTexture);
		//glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, mcTableBuffer);

		//marchingCubesShader->setVec3("densityTextureDimensions", textureWidth, textureHeight, textureDepth);
		//marchingCubesShader->setMat4("projection", projection);
		//marchingCubesShader->setMat4("view", view);
		//marchingCubesShader->setVec3("viewPos", camera.Position);
		//glm::mat4 model = glm::mat4(1.0f);
		//marchingCubesShader->setMat4("model", model);
		//glBindVertexArray(VAO);

		//// First pass for density texture A
		//marchingCubesShader->setInt("cameraSector", cameraSector);
		//glBindTexture(GL_TEXTURE_3D, densityTextureA);
		//glDrawArraysInstanced(GL_POINTS, 0, (textureWidth - 1) * (textureHeight - 1), textureDepth);

		//// Second pass for density texture B
		//marchingCubesShader->setInt("cameraSector", cameraSector - 1);
		//glBindTexture(GL_TEXTURE_3D, densityTextureB);
		//glDrawArraysInstanced(GL_POINTS, 0, (textureWidth - 1) * (textureHeight - 1), textureDepth - 1);

		//// Displacement mapping
		//displacementShader->use();
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, diffuseZ);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, displacementZ);

		//glActiveTexture(GL_TEXTURE2);
		//glBindTexture(GL_TEXTURE_2D, normalZ);
		//displacementShader->setInt("diffuseMap", 0);
		//displacementShader->setInt("displacementMap", 1);
		//displacementShader->setInt("normalMap", 2);
		//displacementShader->setInt("cameraSector", cameraSector);
		//displacementShader->setFloat("heightScale", heightScale);
		//displacementShader->setFloat("normalLevel", normalLevel);
		//displacementShader->setInt("primaryLayers", primaryLayers);
		//displacementShader->setInt("secondaryLayers", secondaryLayers);
		//displacementShader->setMat4("projection", projection);
		//displacementShader->setMat4("view", view);
		//model = glm::translate(model, glm::vec3(0, 0, -1));
		//model = glm::scale(model, glm::vec3(10));
		//displacementShader->setMat4("model", model);
		//displacementShader->setVec3("viewPos", camera.Position);

		// 1. render depth of scene to texture (light POV)
		glm::mat4 lightProjection, lightView, lightSpaceMatrix;
		float near_plane = 1.0f, far_plane = 7.5f;
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		lightSpaceMatrix = lightProjection * lightView;
		// render scene from light pov
		storeDepthShader->use();
		storeDepthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		renderScene(*storeDepthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 2. render scene normally
		// -------------------------------------------------------
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		VSMShader->use();
		VSMShader->setMat4("projection", projection);
		VSMShader->setMat4("view", view);
		// set light uniforms
		VSMShader->setVec3("viewPos", camera.Position);
		VSMShader->setVec3("lightPos", lightPos);
		VSMShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		renderScene(*VSMShader);
		/*basicShader->use();
		basicShader->setMat4("projection", projection);
		basicShader->setMat4("view", view);
		basicShader->setInt("diffuseTexture", 0);
		basicShader->setVec3("viewPos", camera.Position);
		basicShader->setVec3("lightPos", lightPos);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		renderScene(*basicShader);*/

		// render Depth map to quad for visual debugging
		// ---------------------------------------------
		debugShader->use();
		debugShader->setFloat("near_plane", near_plane);
		debugShader->setFloat("far_plane", far_plane);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		//renderQuad();
		

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

// renders the 3D scene
// --------------------
void renderScene(const Shader& shader)
{
	// floor
	glm::mat4 model = glm::mat4(1.0f);
	shader.setMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// cubes
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMat4("model", model);
	renderCube();
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
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

	if (button == GLFW_MOUSE_BUTTON_MIDDLE)
	{
		std::cout << "PARTICLES" << std::endl;
		spawnParticles = true;
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
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}