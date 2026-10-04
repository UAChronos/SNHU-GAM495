// User interface includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Renderer includes
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Math includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "stb_image.h"

#include <iostream>

// Window resize callback function
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// Mouse movement callback function
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// Mouse scroll callback function
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// User input processing
void processInput(GLFWwindow* window);

// Config window visibility toggle button callback
void toggleConfigKey_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// Creates ImGui window and binds actions to interactive elements
void createImGuiWindow();

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

// Define point lights positions
glm::vec3 pointLightPositions[] = {
	glm::vec3(0.0f,  1.2f,  5.0f),
	glm::vec3(0.0f, -3.3f, -4.0f),
	glm::vec3(-4.0f,  2.0f, 0.0f),
	glm::vec3(8.0f,  0.0f, 0.0f)
};

// User interface variables
bool showSceneConfig = true;

// Rendering variables
bool wireframeMode = false;
bool rotateModel = true;

int main()
{
	// Initialize GLFW
	glfwInit();

	// Configure window, set to use OpenGL 3.3 Core
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window object
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "DHRenderer (Press \"O\" to toggle scene config window)", NULL, NULL);

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

	// Register config window visibility toggle callback
	glfwSetKeyCallback(window, toggleConfigKey_callback);

	// Capture mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Check if GLAD is properly loaded
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& guiIO = ImGui::GetIO();
	ImGui::StyleColorsDark();

	// Setup ImGui renderer backend
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Setup STBI
	stbi_set_flip_vertically_on_load(true);

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	// Build and compile shader programs
	Shader lightingShader("Shaders\\shader.vert", "Shaders\\shader.frag");
	Shader lightEmitterShader("Shaders\\lightEmitterShader.vert", "Shaders\\lightEmitterShader.frag");

	// Load models
	Model guitarModel("Resources/backpack/backpack.obj");

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

	// Lighting VAO and VBO
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Configure vertex attribute 0 (position) to read from VBO and enable it
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Unbind VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Unbind cubeVAO
	glBindVertexArray(0);

	// Enable shader
	lightingShader.use();

	lightingShader.setFloat("material.shininess", 32.0f);

	// Setup lights
	// Directional light
	lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);

	// Point light 1
	lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
	lightingShader.setFloat("pointLights[0].constant", 1.0f);
	lightingShader.setFloat("pointLights[0].linear", 0.022f);
	lightingShader.setFloat("pointLights[0].quadratic", 0.0019f);

	// Point light 2
	lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
	lightingShader.setFloat("pointLights[1].constant", 1.0f);
	lightingShader.setFloat("pointLights[1].linear", 0.022f);
	lightingShader.setFloat("pointLights[1].quadratic", 0.0019f);

	// Point light 3
	lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
	lightingShader.setFloat("pointLights[2].constant", 1.0f);
	lightingShader.setFloat("pointLights[2].linear", 0.022f);
	lightingShader.setFloat("pointLights[2].quadratic", 0.0019f);

	// Point light 4
	lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
	lightingShader.setFloat("pointLights[3].constant", 1.0f);
	lightingShader.setFloat("pointLights[3].linear", 0.022f);
	lightingShader.setFloat("pointLights[3].quadratic", 0.0019f);

	// Render loop, stops when GLFW is instructed to stop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate time delta
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Input
		processInput(window);

		if (wireframeMode)
		{
			// Enable wireframe rendering mode
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			// Disable wireframe rendering mode
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// Set buffer clear color to dark green and clear current framebuffer
		glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Activate shader
		lightingShader.use();

		// Update viewer position in fragment shader
		lightingShader.setVec3("viewPos", camera.Position);

		// Update lights' color in frag shader
		// Directional light
		lightingShader.setVec3("dirLight.ambient", lightColor * glm::vec3(0.05f));
		lightingShader.setVec3("dirLight.diffuse", lightColor * glm::vec3(0.4f));
		lightingShader.setVec3("dirLight.specular", lightColor * glm::vec3(0.5f));
		// Point light 1
		lightingShader.setVec3("pointLights[0].ambient", lightColor * glm::vec3(0.05f));
		lightingShader.setVec3("pointLights[0].diffuse", lightColor * glm::vec3(0.8f));
		lightingShader.setVec3("pointLights[0].specular", lightColor);
		// Point light 2
		lightingShader.setVec3("pointLights[1].ambient", lightColor * glm::vec3(0.05f));
		lightingShader.setVec3("pointLights[1].diffuse", lightColor * glm::vec3(0.8f));
		lightingShader.setVec3("pointLights[1].specular", lightColor);
		// Point light 3
		lightingShader.setVec3("pointLights[2].ambient", lightColor * glm::vec3(0.05f));
		lightingShader.setVec3("pointLights[2].diffuse", lightColor * glm::vec3(0.8f));
		lightingShader.setVec3("pointLights[2].specular", lightColor);
		// Point light 4
		lightingShader.setVec3("pointLights[3].ambient", lightColor * glm::vec3(0.05f));
		lightingShader.setVec3("pointLights[3].diffuse", lightColor * glm::vec3(0.8f));
		lightingShader.setVec3("pointLights[3].specular", lightColor);

		// Enable light cube shader and update light color parameters
		lightEmitterShader.use();
		lightEmitterShader.setVec3("lightColor", lightColor);

		// Switch back to lighting shader
		lightingShader.use();

		// Create transformation matrices
		// Projection matrix
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		lightingShader.setMat4("projection", projection);

		// View matrix
		glm::mat4 view = camera.GetViewMatrix();
		lightingShader.setMat4("view", view);

		// Model matrix
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		if (rotateModel)
		{
			model = glm::rotate(model, (float)glfwGetTime() / 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setMat4("model", model);

		// Draw model
		guitarModel.Draw(lightingShader);

		// Draw light source
		glBindVertexArray(lightVAO);

		lightEmitterShader.use();

		model = glm::mat4(1.0f);

		for (unsigned int i = 0; i < 4; i++)
		{
			lightEmitterShader.setMat4("projection", projection);
			lightEmitterShader.setMat4("view", view);
			model = glm::mat4(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
			lightEmitterShader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// Create ImGui window
		if (showSceneConfig)
		{
			createImGuiWindow();

			// Show mouse cursor
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else
		{
			// Disable mouse cursor
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}

		// Render GUI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Check and call events
		glfwPollEvents();
	}

	// Destroy ImGui context
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Free resources
	glDeleteVertexArrays(1, &lightVAO);
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

	if (!showSceneConfig)
	{
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
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Process mouse scroll
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void toggleConfigKey_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Check if options "O" key is was pressed
	if (key == GLFW_KEY_O && action == GLFW_PRESS)
	{
		// Change visibility of scene config window
		if (!showSceneConfig)
		{
			showSceneConfig = !showSceneConfig;
		}
		else
		{
			showSceneConfig = !showSceneConfig;
			firstMouseInput = true;
		}		
	}
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

void createImGuiWindow()
{
	//ImGui::ShowDemoWindow();

	ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Scene Configuration (Press \"O\" to toggle)", &showSceneConfig))
	{
		ImGui::Checkbox("Wireframe rendering mode", &wireframeMode);
		ImGui::Checkbox("Rotate model", &rotateModel);

		// Light sources config
		if (ImGui::CollapsingHeader("Light Sources"))
		{
			float colorSelectorVec[3] = { lightColor.x, lightColor.y, lightColor.z };
			if (ImGui::ColorEdit3("Light sources color", colorSelectorVec))
			{
				lightColor.x = colorSelectorVec[0];
				lightColor.y = colorSelectorVec[1];
				lightColor.z = colorSelectorVec[2];
			}

			ImGui::SeparatorText("Point lights coordinates");
			static float pl1Pos[3] = { pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z };

			if (ImGui::InputFloat3("Point light 1", pl1Pos))
			{
				pointLightPositions[0].x = pl1Pos[0];
				pointLightPositions[0].y = pl1Pos[1];
				pointLightPositions[0].z = pl1Pos[2];
			}

			static float pl2Pos[3] = { pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z };

			if (ImGui::InputFloat3("Point light 2", pl2Pos))
			{
				pointLightPositions[1].x = pl2Pos[0];
				pointLightPositions[1].y = pl2Pos[1];
				pointLightPositions[1].z = pl2Pos[2];
			}

			static float pl3Pos[3] = { pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z };

			if (ImGui::InputFloat3("Point light 3", pl3Pos))
			{
				pointLightPositions[2].x = pl3Pos[0];
				pointLightPositions[2].y = pl3Pos[1];
				pointLightPositions[2].z = pl3Pos[2];
			}

			static float pl4Pos[3] = { pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z };

			if (ImGui::InputFloat3("Point light 4", pl4Pos))
			{
				pointLightPositions[3].x = pl4Pos[0];
				pointLightPositions[3].y = pl4Pos[1];
				pointLightPositions[3].z = pl4Pos[2];
			}
		}

		ImGui::End();
	}
	else
	{
		ImGui::End();
	}

	// If window was closed reset camera movement
	if (!showSceneConfig)
	{
		firstMouseInput = true;
	}
}