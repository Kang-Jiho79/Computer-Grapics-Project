#include "Alex_Camera.h"
#include <gl/freeglut.h>
#include <algorithm>

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

Alex_Camera::Alex_Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM)
{
    this->position = position;
    this->worldUp = up;
    this->yaw = yaw;
    this->pitch = pitch;
    updateCameraVectors();
}

void Alex_Camera::updateFromCharacterPosition(const glm::vec3& characterPosition)
{
    // 캐릭터의 머리 부분에 카메라 위치 설정 (캐릭터보다 약간 위)
    position = characterPosition + glm::vec3(0.0f, 0.8f, 0.0f); // 눈 높이
}

glm::mat4 Alex_Camera::getViewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Alex_Camera::getProjectionMatrix(float screenWidth, float screenHeight) const
{
    // 분할 화면을 위해 종횡비를 절반 화면에 맞춤
    float aspectRatio = (screenWidth / 2.0f) / screenHeight;
    return glm::perspective(glm::radians(zoom), aspectRatio, 0.1f, 100.0f);
}

void Alex_Camera::processKeyboard(int key, float deltaTime)
{
    float velocity = movementSpeed * deltaTime;
    
    switch(key) {
        case GLUT_KEY_UP:
            // 위쪽 보기
            pitch += velocity * 10.0f;
            break;
        case GLUT_KEY_DOWN:
            // 아래쪽 보기
            pitch -= velocity * 10.0f;
            break;
        case GLUT_KEY_LEFT:
            // 왼쪽 보기
            yaw -= velocity * 10.0f;
            break;
        case GLUT_KEY_RIGHT:
            // 오른쪽 보기
            yaw += velocity * 10.0f;
            break;
    }
    
    // pitch 제한
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
    
    updateCameraVectors();
}

void Alex_Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch)
{
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yaw += xOffset;
    pitch += yOffset;

    if (constrainPitch) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    updateCameraVectors();
}

void Alex_Camera::processMouseScroll(float yOffset)
{
    zoom -= (float)yOffset;
    if (zoom < 1.0f)
        zoom = 1.0f;
    if (zoom > 45.0f)
        zoom = 45.0f;
}

void Alex_Camera::updateCameraVectors()
{
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}
