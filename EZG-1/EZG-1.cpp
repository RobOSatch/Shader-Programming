#include "glad/glad.h"
#include "glfw/glfw3.h""

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"

#include "Shader.h"
#include "Camera.h"
#include "interpolation.h"

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

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(9.5f, 1.0f, 9.5f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float speed = 0.13f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

Game_Mode gameMode = CREATE;

std::vector<glm::vec3> waypoints;
std::vector<glm::quat> orientations;


int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader zprogram
	// ------------------------------------
	Shader lightingShader("lightShader.vert", "lightShader.frag");
	Shader lampShader("lampShader.vert", "lampShader.frag");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};
	
	float planeRadius = 10.0f;
	float plane[] = {
		-planeRadius,  -0.5f, -planeRadius,  0.0f,  1.0f,  0.0f,
		 planeRadius,  -0.5f,  planeRadius,  0.0f,  1.0f,  0.0f,
		-planeRadius,  -0.5f,  planeRadius,  0.0f,  1.0f,  0.0f,
		 planeRadius,  -0.5f, -planeRadius,  0.0f,  1.0f,  0.0f,

		-planeRadius,  -0.5f, -planeRadius,  0.0f,  1.0f,  0.0f,
		 planeRadius,  -0.5f,  planeRadius,  0.0f,  1.0f,  0.0f,
		-planeRadius,  -0.5f,  planeRadius,  0.0f,  1.0f,  0.0f,
		 planeRadius,  -0.5f, -planeRadius,  0.0f,  1.0f,  0.0f
	};

	std::vector<glm::vec3> curvePoints;

	int currentWaypoint = 0;

	// first, configure the cube's VAO (and VBO)
	unsigned int VBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	// we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int planeVBO, planeVAO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);
	glBindVertexArray(planeVAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	float t = 0.0f;
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// be sure to activate shader when setting uniforms/drawing objects
		lightingShader.use();
		lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.setVec3("lightPos", lightPos);
		lightingShader.setVec3("viewPos", camera.Position);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);

		// world transformation
		glm::mat4 model = glm::mat4(1.0f);
		lightingShader.setMat4("model", model);

		// render the cubes
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(5.0f, 0.0f, 5.0f));
		lightingShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-5.0f, 0.0f, -5.0f));
		lightingShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// draw the plane
		model = glm::mat4(1.0f);
		lightingShader.setVec3("objectColor", 0.5f, 0.31f, 1.0f);
		lightingShader.setMat4("model", model);

		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 8);

		// also draw the light
		lampShader.use();
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
		lampShader.setMat4("model", model);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//if (gameMode == CREATE) {
		//	// Draw waypoints
		//	for (int i = 0; i < waypoints.size(); i++) {
		//		model = glm::mat4(1.0f);
		//		model = glm::translate(model, waypoints[i]);
		//		model = glm::scale(model, glm::vec3(0.1f));
		//		lampShader.setMat4("model", model);

		//		glDrawArrays(GL_TRIANGLES, 0, 36);
		//	}
		//}

		if (gameMode == RIDE) {
			// Visualize curve
			bool visualize = false;
			glm::vec3 pos;
			if (visualize) {
				for (int i = 0; i < waypoints.size() - 1; i++) {
					for (float p = 0.0f; p <= 1.0f; p += 0.01f) {
						if (i == 0) {
							pos = interpolation::catmullRomSpline(waypoints[i], waypoints[i], waypoints[i + 1], waypoints[i + 2], p);
						}
						else if (i == waypoints.size() - 2) {
							pos = interpolation::catmullRomSpline(waypoints[i - 1], waypoints[i], waypoints[i + 1], waypoints[i + 1], p);
						}
						else {
							pos = interpolation::catmullRomSpline(waypoints[i - 1], waypoints[i], waypoints[i + 1], waypoints[i + 2], p);
						}

						pos.y -= 0.1f;
						model = glm::mat4(1.0f);
						model = glm::translate(model, pos);
						model = glm::scale(model, glm::vec3(0.05f));
						lampShader.setMat4("model", model);

						glDrawArrays(GL_TRIANGLES, 0, 36);
					}
				}
			}

			// Draw splines
			glm::vec3 nextPosition;
			glm::quat nextOrientation;
			if (t <= 1.0f && currentWaypoint != waypoints.size() - 1) {
				float e = 0.1;
				bool done = false;

				while (!done) {
					if (currentWaypoint == 0) {
						nextPosition = interpolation::catmullRomSpline(waypoints[currentWaypoint], waypoints[currentWaypoint], waypoints[currentWaypoint + 1], waypoints[currentWaypoint + 2], t);
						nextOrientation = interpolation::squad(orientations[currentWaypoint], orientations[currentWaypoint], orientations[currentWaypoint + 1], orientations[currentWaypoint + 2], t);
					}
					else if (currentWaypoint == waypoints.size() - 2) {
						nextPosition = interpolation::catmullRomSpline(waypoints[currentWaypoint - 1], waypoints[currentWaypoint], waypoints[currentWaypoint + 1], waypoints[currentWaypoint + 1], t);
						nextOrientation = interpolation::squad(orientations[currentWaypoint - 1], orientations[currentWaypoint], orientations[currentWaypoint + 1], orientations[currentWaypoint + 1], t);
					}
					else {
						nextPosition = interpolation::catmullRomSpline(waypoints[currentWaypoint - 1], waypoints[currentWaypoint], waypoints[currentWaypoint + 1], waypoints[currentWaypoint + 2], t);
						nextOrientation = interpolation::squad(orientations[currentWaypoint - 1], orientations[currentWaypoint], orientations[currentWaypoint + 1], orientations[currentWaypoint + 2], t);
					}

					glm::vec3 direction = nextPosition - camera.Position;
					float distance = glm::length(direction);

					if (distance <= speed + 0.1 && distance >= speed - 0.1) {
						done = true;
						camera.Position = nextPosition;
						camera.Orientation = nextOrientation;

						// SLERP
						//camera.Orientation = glm::slerp(firstQuat, secondQuat, t);
					}

					t += e * deltaTime * speed;

					if (distance >= 5.0f) {
						camera.Position = nextPosition;
						done = true;
					}
				}
			}

			if (t >= 1.0f) {
				t = 0.0f;
				currentWaypoint++;
			}

			if (currentWaypoint == waypoints.size() - 1) {
				waypoints.clear();
				orientations.clear();
				gameMode = CREATE;
				currentWaypoint = 0;
			}
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow * window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

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

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		speed = fmin(0.5f, speed + 0.001f);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		speed = fmax(0.0f, speed - 0.001f);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS && gameMode == CREATE) {
		gameMode = RIDE;
		camera.Position = waypoints[0];
		camera.Orientation = orientations[0];
	}
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
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}