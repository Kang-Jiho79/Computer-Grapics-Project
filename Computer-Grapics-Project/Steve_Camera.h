#pragma once
#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

class Steve_Camera
{
private:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    
    // 오일러 각
    float yaw;
    float pitch;
    
    // 카메라 옵션
    float movementSpeed;
    float mouseSensitivity;
    float zoom;
    
    // 카메라 벡터 업데이트
    void updateCameraVectors();

public:
    Steve_Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
                 glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                 float yaw = -90.0f, float pitch = 0.0f);
    
    // 스티브 캐릭터 위치에 맞춰 카메라 위치 업데이트
    void updateFromCharacterPosition(const glm::vec3& characterPosition);
    
    // 뷰 매트릭스 반환
    glm::mat4 getViewMatrix() const;
    
    // 투영 매트릭스 반환 (분할 화면용)
    glm::mat4 getProjectionMatrix(float screenWidth, float screenHeight) const;
    
    // 키보드 입력 처리 (WASD로 시점 회전)
    void processKeyboard(unsigned char key, float deltaTime);
    
    // 마우스 움직임 처리
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
    
    // 마우스 스크롤 처리
    void processMouseScroll(float yOffset);
    
    // Getter 함수들
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    glm::vec3 getUp() const { return up; }
    float getZoom() const { return zoom; }
};

