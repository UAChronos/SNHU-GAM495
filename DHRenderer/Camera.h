#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Abstract camera movement options
enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// Camera class that processes input and calculates camera Euler Angles, Vectors, and Matrices.
class Camera
{
public:
	// Camera attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	// Camera look direction variables
	float Yaw;
	float Pitch;

	// Camera settings
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// Camera constructor allowing to set attributes
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// Camera constructor allowing to set attributes using vector components
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	/* Processes digital input from input devices like keyboard.
	* @param direction - input parameter in the form of abstract camera movement defined by ENUM
	*/
	void ProcessDigitalInput(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;

		if (direction == FORWARD)
		{
			Position += Front * velocity;
		}

		if (direction == BACKWARD)
		{
			Position -= Front * velocity;
		}

		if (direction == LEFT)
		{
			Position -= Right * velocity;
		}

		if (direction == RIGHT)
		{
			Position += Right * velocity;
		}
	}

	/* Processes analog input received from mouse.
	* @param xDelta - mouse position change on X axis since last frame
	* @param yDelta - mouse position change on Y axis since last frame
	* @param constrainPitch - whether to constraint pitch from -90 to 90 degrees
	*/
	void ProcessMouseMovement(float xDelta, float yDelta, GLboolean constrainPitch = true)
	{
		xDelta *= MouseSensitivity;
		yDelta *= MouseSensitivity;

		// Update yaw and pitch
		Yaw += xDelta;
		Pitch += yDelta;

		// Constraint pitch movement
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
			{
				Pitch = 89.0f;
			}

			if (Pitch < -89.0f)
			{
				Pitch = -89.0f;
			}
		}

		// Update camera attributes using the updated Euler angles
		updateCameraVectors();
	}

	/* Processes input received from mouse scroll wheel.
	* @param yDelta - mouse scroll wheel Y axis change since last frame
	*/
	void ProcessMouseScroll(float yDelta)
	{
		Zoom -= (float)yDelta;

		// Clamp fov from 1 to 45
		if (Zoom < 1.0f)
		{
			Zoom = 1.0f;
		}

		if (Zoom > 45.0f)
		{
			Zoom = 45.0f;
		}
	}

private:
	// Calculates camera Front, Right, and Up vectors. Vectors are normalized to maintain constant speed of change
	void updateCameraVectors()
	{
		// Calculate the Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

		Front = glm::normalize(front);
		
		// Calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  
		Up = glm::normalize(glm::cross(Right, Front));
	}
};