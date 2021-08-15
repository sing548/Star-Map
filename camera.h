#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

enum Camera_Input {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN,
	SPRINT,
	JOG,
	WALK
};

const float YAW			= -90.0f;
const float PITCH		= 0.0f;
const float SPEED		= 5.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM		= 45.0f;
const bool SPRINTING	= false;
const bool JOGGING		= false;

class Camera
{
public:
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	float Yaw;
	float Pitch;
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	bool Sprinting;
	bool Jogging;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM), Sprinting(SPRINTING), Jogging(JOGGING)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM), Sprinting(SPRINTING), Jogging(JOGGING)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	void ProcessKeyboard(Camera_Input input, float deltaTime)
	{
		
		glm::vec3 oldPos = Position;

		if (input == SPRINT)
		{
			Sprinting = true;
			Jogging = false;
		}

		if (input == JOG)
		{
			Sprinting = false;
			Jogging = true;
		}
		if (input == WALK)
		{
			Sprinting = false;
			Jogging = false;
		}

		float velocity;
		if (Sprinting)
			velocity = MovementSpeed * deltaTime * 50.0f;
		else if (Jogging)
			velocity = MovementSpeed * deltaTime * 10.0f;
		else
			velocity = MovementSpeed * deltaTime;
		if (input == FORWARD)
			Position += Front * velocity;
		if (input == BACKWARD)
			Position -= Front * velocity;
		if (input == LEFT)
			Position -= Right * velocity;
		if (input == RIGHT)
			Position += Right * velocity;
		
		Position.y = oldPos.y;

		if (input == UP)
			Position += Up * velocity;
		if (input == DOWN)
			Position -= Up * velocity;
	}

	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw   += xoffset;
		Pitch += yoffset;

		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		updateCameraVectors();
	}

	void ProcessMouseScroll(float yoffset)
	{
		Zoom -= (float)yoffset;

		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > 45.0f)
			Zoom = 45.0f;
	}

private:
	void updateCameraVectors()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);

		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up	  = glm::normalize(glm::cross(Right, Front));
	}
};

#endif