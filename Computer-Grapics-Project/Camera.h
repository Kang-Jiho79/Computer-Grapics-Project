#pragma once

#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

class Camera {
public:
    // 카메라 위치와 방향
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // 오일러 각도
    float yaw;
    float pitch;

    // 카메라 옵션
    float movementSpeed;
    float mouseSensitivity;
    float fov;

    // 생성자
    Camera(glm::vec3 pos = glm::vec3(5.0f, 8.0f, 12.0f), 
           glm::vec3 worldUpVec = glm::vec3(0.0f, 1.0f, 0.0f),
           float yawAngle = -90.0f, 
           float pitchAngle = -20.0f);

    // 뷰 매트릭스 반환
    glm::mat4 getViewMatrix() const;

    // 프로젝션 매트릭스 반환
    glm::mat4 getProjectionMatrix(float screenWidth, float screenHeight) const;

    // 키보드 입력 처리
    void processKeyboard(char key, float deltaTime);

    // 특수 키 입력 처리
    void processSpecialKeyboard(int key);

    // 마우스 휠 입력 처리
    void processMouseScroll(int direction);

    // 마우스 움직임 처리
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);

    // 카메라 속도 조정
    void adjustSpeed(float speedDelta);

    // 카메라 위치 리셋
    void reset();

    // 카메라 위치와 각도 설정
    void setPosition(const glm::vec3& pos);
    void setAngles(float yawAngle, float pitchAngle);

private:
    // 카메라 벡터들 업데이트
    void updateCameraVectors();
};

