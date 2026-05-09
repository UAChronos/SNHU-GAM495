// Renderer includes
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

// Math includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"

#include <iostream>

// Window resize callback function
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// Mouse movement callback function
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// Mouse scroll callback function
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// User input processing
void processInput(GLFWwindow* window);

// Manages texture loading
unsigned int loadTexture(char const* path);

// Settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

// Texture blending force
float mixValue = 0.2f;

// Camera object and variables
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouseInput = true;

// Camera movement speed
const float cameraSpeed = 2.5f;

// Time variables
// Time delta between current and last frame
float deltaTime = 0.0f;
// Last frame time
float lastFrame = 0.0f;

// Lighting variables
glm::vec3 lightSourcePos(1.2f, 1.0f, 2.0f);
glm::vec3 lightColor = glm::vec3(1.0f);

int main()
{
	// Initialize GLFW
	glfwInit();

	// Configure window, set to use OpenGL 3.3 Core
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window object
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "DHRenderer", NULL, NULL);

	// Report error
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Switch OpenGL context to window object
	glfwMakeContextCurrent(window);

	// Register framebuffer resize callback
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Register mouse movement callback
	glfwSetCursorPosCallback(window, mouse_callback);

	// Register mouse scroll callback
	glfwSetScrollCallback(window, scroll_callback);

	// Capture mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Check if GLAD is properly loaded
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Configure global OpenGL state
	// Wireframe rendering mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	// Build and compile shader programs
	Shader lightingShader("Shaders\\shader.vert", "Shaders\\shader.frag");
	Shader lightCubeShader("Shaders\\lightingShader.vert", "Shaders\\lightingShader.frag");

	// Set up vertex data, buffers, and configure vertex attributes
	// ------------------------------------------------------------------
	// Define vertices
	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	// Define translation vectors for cubes
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	// Generate vertex buffer object and assign its ID to VBO
	unsigned int VBO;
	glGenBuffers(1, &VBO);

	// Generate vertex array object and assign its ID to cubeVAO
	unsigned int cubeVAO;
	glGenVertexArrays(1, &cubeVAO);

	glBindVertexArray(cubeVAO);

	// Bind VBO and copy vertices to buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Configure vertex attribute 0 (position) to read from VBO and enable it
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Configure vertex attribute 1 (aNormal) to read from VBO and enable it
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Configure vertex attribute 2 (aTexCoords) to read from VBO and enable it
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Lighting VAO and VBO
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Configure vertex attribute 0 (position) to read from VBO and enable it
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Unbind VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Unbind cubeVAO
	glBindVertexArray(0);

	// Load and create texture maps
	unsigned int diffuseMap = loadTexture("Resources\\container2.png");
	unsigned int specularMap = loadTexture("Resources\\container2_specular.png");

	// Enable shader
	lightingShader.use();

	// Pass texture unit ids to frag shader
	lightingShader.setInt("material.diffuse", 0);
	lightingShader.setInt("material.specular", 1);

	lightingShader.setFloat("material.shininess", 32.0f);

	// Render loop, stops when GLFW is instructed to stop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate time delta
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Input
		processInput(window);

		// Set buffer clear color to dark green and clear current framebuffer
		glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Activate shader
		lightingShader.use();

		// Update light source position in fragment shader
		lightingShader.setVec3("lightPos", lightSourcePos);
		lightingShader.setVec3("viewPos", camera.Position);

		// Update light color in frag shader
		lightingShader.setVec3("light.ambient", lightColor * glm::vec3(0.2f));
		lightingShader.setVec3("light.diffuse", lightColor * glm::vec3(0.5f));
		lightingShader.setVec3("light.specular", lightColor);

		// Enable light cube shader and update light color parameters
		lightCubeShader.use();
		lightCubeShader.setVec3("lightColor", lightColor);

		// Switch back to lighting shader
		lightingShader.use();

		// Create transformation matrices
		// Projection matrix
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		lightingShader.setMat4("projection", projection);

		// View matrix
		glm::mat4 view = camera.GetViewMatrix();
		lightingShader.setMat4("view", view);

		// Bind and activate diffuse map texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);

		// Bind and activate specular map texture
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);

		// Render boxes' tris
		glBindVertexArray(cubeVAO);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
		lightingShader.setMat4("model", model);

		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Draw light source
		lightCubeShader.use();
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightSourcePos);
		model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
		lightCubeShader.setMat4("model", model);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Check and call events
		glfwPollEvents();
	}

	// Free resources
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &VBO);

	// Free allocated GLFW resources
	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xPosIn, double yPosIn)
{
	float xPos = static_cast<float>(xPosIn);
	float yPos = static_cast<float>(yPosIn);

	// Initially set to true
	if (firstMouseInput)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouseInput = false;
	}

	float xoffset = xPos - lastX;
	float yoffset = lastY - yPos;

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Process mouse scroll
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow* window)
{
	float camMovementSpeed = cameraSpeed * deltaTime;

	// Check if "Escape" key is was pressed
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		// Command GLFW to terminate execution
		glfwSetWindowShouldClose(window, true);
	}

	// Check if arrow keys are pressed
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		/*mixValue += 0.001f;
		if (mixValue >= 1.0f)
		{
			mixValue = 1.0f;
		}*/

		lightColor.x += 0.005f;
		if (lightColor.x >= 1.0f)
		{
			lightColor.x = 1.0f;
		}

		lightColor.y += 0.005f;
		if (lightColor.y >= 1.0f)
		{
			lightColor.y = 1.0f;
		}

		lightColor.z += 0.005f;
		if (lightColor.z >= 1.0f)
		{
			lightColor.z = 1.0f;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		/*mixValue -= 0.001f;
		if (mixValue <= 0.0f)
		{
			mixValue = 0.0f;
		}*/

		lightColor.x -= 0.005f;
		if (lightColor.x <= 0.0f)
		{
			lightColor.x = 0.0f;
		}

		lightColor.y -= 0.005f;
		if (lightColor.y <= 0.0f)
		{
			lightColor.y = 0.0f;
		}

		lightColor.z -= 0.005f;
		if (lightColor.z <= 0.0f)
		{
			lightColor.z = 0.0f;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		lightColor.z -= 0.005f;
		if (lightColor.z <= 0.0f)
		{
			lightColor.z = 0.0f;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		lightColor.z += 0.005f;
		if (lightColor.z >= 1.0f)
		{
			lightColor.z = 1.0f;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		// Move camera forward
		camera.ProcessDigitalInput(FORWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		// Move camera backwards
		camera.ProcessDigitalInput(BACKWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		// Move camera left
		camera.ProcessDigitalInput(LEFT, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		// Move camera right
		camera.ProcessDigitalInput(RIGHT, deltaTime);
	}
}

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