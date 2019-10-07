#include "glad/glad.h"
#include "glfw/glfw3.h""

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.h"
#include "Camera.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

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

float speed = 0.01f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

/**
*
* CATMULL TIME
* Calculates the point for the Catmull-Rom Spline located between points p1 and p2 at t.
*
* PARAMS:
* p0 - The point before the curve
* p1 - The starting point of the curve
* p2 - The end point of the curve
* p3 - The point after the curve
* t - The interpolation parameter, going from 0 to 1, where 0 is the start and 1 is the end of the curve
*
*/
glm::vec3 catmullRomSpline(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t) {
	float a = 0.0f; // Tension
	float b = 0.0f; // Bias
	float c = 0.0f; // Continuity

	glm::vec3 sourceVec1 = p1 - p0;
	glm::vec3 destinationVec1 = p2 - p1;
	glm::vec3 sourceVec2 = p2 - p1;
	glm::vec3 destinationVec2 = p3 - p2;
	glm::vec3 sourceTangent = (1.0f - a) * (1.0f + b) * (1.0f - c) / 2.0f * sourceVec1 + (1.0f - a) * (1.0f - b) * (1.0f + c) * destinationVec1;
	glm::vec3 destinationTangent = (1.0f - a) * (1.0f + b) * (1.0f + c) / 2.0f * sourceVec2 + (1.0f - a) * (1.0f - b) * (1.0f - c) * destinationVec2;

	glm::vec3 point0 = (2.0f * pow(t, 3) - 3.0f * pow(t, 2) + 1.0f) * p1;
	glm::vec3 m0 = (pow(t, 3) - 2.0f * pow(t, 2) + t) * sourceTangent;
	glm::vec3 m1 = (pow(t, 3) - pow(t, 2)) * destinationTangent;
	glm::vec3 point1 = (-2.0f * pow(t, 3) + 3.0f * pow(t, 2)) * p2;

	return point0 + m0 + m1 + point1;
}

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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

	std::vector<glm::vec3> waypoints = {
		glm::vec3(8.0f,  0.5f,  8.0f),
		glm::vec3(5.0f,  0.5f, 3.0f),
		glm::vec3(-3.0f,  0.5f, -5.0f),
		glm::vec3(-7.0f,  0.5f, 4.0f),
		glm::vec3(6.0f, 0.5f, 4.0f),
		glm::vec3(8.0f, 0.5f, 5.0f),
		glm::vec3(6.0f, 0.5f, 6.0f),
		glm::vec3(8.0f, 0.5f, 7.0f)
	};

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
	
	camera.Position = waypoints[0];

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
		lightingShader.setVec3("objectColor", 0.5f, 1.0f, 0.31f);
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

		// Draw waypoints
		for (int i = 0; i < waypoints.size(); i++) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, waypoints[i]);
			model = glm::scale(model, glm::vec3(0.1f));
			lampShader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		std::cout << camera.Position.x << " " << camera.Position.y << " " << camera.Position.z << std::endl;
		// Draw splines
		glm::vec3 nextPosition;
		if (t <= 1.0f && currentWaypoint != waypoints.size() - 1) {
			float e = 0.0001;
			bool done = false;

			while (!done) {
				if (currentWaypoint == 0) {
					nextPosition = catmullRomSpline(waypoints[currentWaypoint], waypoints[currentWaypoint], waypoints[currentWaypoint + 1], waypoints[currentWaypoint + 2], t);
				}
				else if (currentWaypoint == waypoints.size() - 2) {
					nextPosition = catmullRomSpline(waypoints[currentWaypoint - 1], waypoints[currentWaypoint], waypoints[currentWaypoint + 1], waypoints[currentWaypoint + 1], t);
				}
				else {
					nextPosition = catmullRomSpline(waypoints[currentWaypoint - 1], waypoints[currentWaypoint], waypoints[currentWaypoint + 1], waypoints[currentWaypoint + 2], t);
				}

				glm::vec3 direction = nextPosition - camera.Position;
				float distance = glm::length(direction);
				std::cout << distance << std::endl;

				if (distance <= speed + 0.001 && distance >= speed - 0.001) {
					done = true;
					camera.Position = nextPosition;	
				}

				t += e;
			}
		}

		if (t >= 1.0f) {
			t = 0.0f;
			currentWaypoint++;
		}

		/**
		while (t <= 1.0f) {
			curvePoint = catmullRomSpline(waypoints[0], waypoints[0], waypoints[1], waypoints[2], t);
			model = glm::mat4(1.0f);
			model = glm::translate(model, curvePoint);
			model = glm::scale(model, glm::vec3(0.05f));
			lampShader.setMat4("model", model);

			//glDrawArrays(GL_TRIANGLES, 0, 36);
			camera.Position += glm::normalize(curvePoint - camera.Position) * 0.02f;

			curvePoint = catmullRomSpline(waypoints[0], waypoints[1], waypoints[2], waypoints[3], t);
			model = glm::mat4(1.0f);
			model = glm::translate(model, curvePoint);
			model = glm::scale(model, glm::vec3(0.05f));
			lampShader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);

			curvePoint = catmullRomSpline(waypoints[1], waypoints[2], waypoints[3], waypoints[3], t);
			model = glm::mat4(1.0f);
			model = glm::translate(model, curvePoint);
			model = glm::scale(model, glm::vec3(0.05f));
			lampShader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);

			t += 0.001f;
		}
		*/

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
		speed = fmin(0.5f, speed + 0.0001f);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		speed = fmax(0.0f, speed - 0.0001f);
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