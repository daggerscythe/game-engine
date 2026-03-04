#include "Camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 up, float yaw, float pitch) : 
    front(glm::vec3(0.0f, 0.0f, -1.0f)), 
    speed(SPEED), 
    sensitivity(SENSITIVITY), 
    zoom(ZOOM)
{
    position = pos;
    worldUp = up;
    this->yaw = yaw;
    this->pitch = pitch;
    m_updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
{
    position = glm::vec3(posX, posY, posZ);
    worldUp = glm::vec3(upX, upY, upZ);
    this->yaw = yaw;
    this->pitch = pitch;
    m_updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(position, position + front, up);
}

void Camera::processKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = speed * deltaTime;
    if (direction == FORWARD) position += front * velocity;
    if (direction == BACKWARD) position -= front * velocity;
    if (direction == LEFT) position -= right * velocity;
    if (direction == RIGHT) position += right * velocity; 

    // cannot fly around
    //position.y = 0.0f;
}

void Camera::processMouseMovement(float xoffset, float yoffset, GLboolean constraintPitch)
{
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (constraintPitch)
    {
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }

    // update front, right, up vectors using new euler angles
    m_updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset)
{
    zoom -= (float)yoffset;
    if (zoom < 1.0f) zoom = 1.0f;
    if (zoom > 45.0f) zoom = 45.0f;
}

void Camera::m_updateCameraVectors()
{
    // calculate new front
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    // re-calculate front and up
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}
