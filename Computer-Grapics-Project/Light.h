#pragma once
#include <gl/glew.h>
#include <gl/glm/glm.hpp>
#include <vector>

// 조명 타입 열거형
enum class LightType {
    DIRECTIONAL = 0,  // 방향광 (태양빛)
    POINT = 1,        // 점광 
    SPOT = 2          // 스포트라이트
};

// 단일 조명 구조체
struct Light {
    LightType type;
    glm::vec3 position;      // 위치 (점광, 스포트라이트용)
    glm::vec3 direction;     // 방향 (방향광, 스포트라이트용)
    glm::vec3 color;         // 조명 색상
    float intensity;         // 조명 강도
    
    // 감쇠 계수 (점광, 스포트라이트용)
    float constant;
    float linear;
    float quadratic;
    
    // 스포트라이트용 각도
    float cutOff;
    float outerCutOff;
    
    // 기본 생성자
    Light();
    
    // 방향광 생성자
    Light(const glm::vec3& direction, const glm::vec3& color, float intensity = 1.0f);
    
    // 점광 생성자
    Light(const glm::vec3& position, const glm::vec3& color, float intensity = 1.0f, 
          float constant = 1.0f, float linear = 0.09f, float quadratic = 0.032f);
    
    // 스포트라이트 생성자
    Light(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color, 
          float intensity = 1.0f, float cutOff = 12.5f, float outerCutOff = 17.5f);
};

// 조명 매니저 클래스
class LightManager {
private:
    std::vector<Light> lights;
    glm::vec3 ambientColor;
    float ambientStrength;
    
public:
    // 생성자
    LightManager();
    
    // 조명 추가/제거
    void addLight(const Light& light);
    void removeLight(size_t index);
    void clearLights();
    
    // 환경광 설정
    void setAmbientLight(const glm::vec3& color, float strength);
    
    // 조명 접근자
    Light& getLight(size_t index);
    const Light& getLight(size_t index) const;
    size_t getLightCount() const;
    
    // 조명 위치 업데이트 (카메라 따라다니기 등)
    void updateLightPosition(size_t index, const glm::vec3& newPos);
    void updateLightDirection(size_t index, const glm::vec3& newDir);
    
    // 셰이더에 조명 데이터 전송
    void applyLighting(GLuint shaderProgram, const glm::vec3& viewPos) const;
    
    // 기본 조명 설정
    void setupDefaultLighting();
    void setupCameraFollowLight(const glm::vec3& cameraPos);
    void setupSceneStaticLight();
};

