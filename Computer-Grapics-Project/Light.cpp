#include "Light.h"
#include <iostream>
#include <string>

// Light 구조체 구현
Light::Light() 
    : type(LightType::POINT), position(0.0f), direction(0.0f, -1.0f, 0.0f), 
      color(1.0f), intensity(1.0f), constant(1.0f), linear(0.09f), 
      quadratic(0.032f), cutOff(12.5f), outerCutOff(17.5f) {}

Light::Light(const glm::vec3& direction, const glm::vec3& color, float intensity)
    : type(LightType::DIRECTIONAL), position(0.0f), direction(direction), 
      color(color), intensity(intensity), constant(1.0f), linear(0.0f), 
      quadratic(0.0f), cutOff(0.0f), outerCutOff(0.0f) {}

Light::Light(const glm::vec3& position, const glm::vec3& color, float intensity, 
             float constant, float linear, float quadratic)
    : type(LightType::POINT), position(position), direction(0.0f), 
      color(color), intensity(intensity), constant(constant), linear(linear), 
      quadratic(quadratic), cutOff(0.0f), outerCutOff(0.0f) {}

Light::Light(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color, 
             float intensity, float cutOff, float outerCutOff)
    : type(LightType::SPOT), position(position), direction(direction), 
      color(color), intensity(intensity), constant(1.0f), linear(0.09f), 
      quadratic(0.032f), cutOff(cutOff), outerCutOff(outerCutOff) {}

// LightManager 클래스 구현
LightManager::LightManager() 
    : ambientColor(0.2f, 0.2f, 0.2f), ambientStrength(0.1f) {
    lights.reserve(8);
}

void LightManager::addLight(const Light& light) {
    if (lights.size() < 8) {
        lights.push_back(light);
        std::cout << "조명이 추가되었습니다. 현재 조명 수: " << lights.size() << std::endl;
    } else {
        std::cout << "최대 조명 수에 도달했습니다." << std::endl;
    }
}

void LightManager::removeLight(size_t index) {
    if (index < lights.size()) {
        lights.erase(lights.begin() + index);
        std::cout << "조명이 제거되었습니다. 현재 조명 수: " << lights.size() << std::endl;
    }
}

void LightManager::clearLights() {
    lights.clear();
    std::cout << "모든 조명이 제거되었습니다." << std::endl;
}

void LightManager::setAmbientLight(const glm::vec3& color, float strength) {
    ambientColor = color;
    ambientStrength = strength;
}

Light& LightManager::getLight(size_t index) {
    return lights[index];
}

const Light& LightManager::getLight(size_t index) const {
    return lights[index];
}

size_t LightManager::getLightCount() const {
    return lights.size();
}

void LightManager::updateLightPosition(size_t index, const glm::vec3& newPos) {
    if (index < lights.size()) {
        lights[index].position = newPos;
    }
}

void LightManager::updateLightDirection(size_t index, const glm::vec3& newDir) {
    if (index < lights.size()) {
        lights[index].direction = newDir;
    }
}

void LightManager::applyLighting(GLuint shaderProgram, const glm::vec3& viewPos) const {
    // 카메라 위치 설정
    GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    if (viewPosLoc != -1) {
        glUniform3fv(viewPosLoc, 1, &viewPos[0]);
    }
    
    // 기존 셰이더와 호환되는 단순 조명 적용
    if (!lights.empty()) {
        const Light& mainLight = lights[0];
        
        // 메인 조명 위치
        GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        if (lightPosLoc != -1) {
            glUniform3fv(lightPosLoc, 1, &mainLight.position[0]);
        }
        
        // 메인 조명 색상 (환경광 포함)
        GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
        if (lightColorLoc != -1) {
            // 환경광을 조명 색상에 섞어서 전달
            glm::vec3 finalColor = mainLight.color * mainLight.intensity + ambientColor * ambientStrength;
            glUniform3fv(lightColorLoc, 1, &finalColor[0]);
        }
        
    } else {
        // 조명이 없으면 기본값 설정
        GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
        
        if (lightPosLoc != -1) {
            glm::vec3 defaultPos(5.0f, 10.0f, 5.0f);
            glUniform3fv(lightPosLoc, 1, &defaultPos[0]);
        }
        
        if (lightColorLoc != -1) {
            glm::vec3 defaultColor(1.0f, 1.0f, 1.0f);
            glUniform3fv(lightColorLoc, 1, &defaultColor[0]);
        }
    }
}

void LightManager::setupDefaultLighting() {
    clearLights();
    
    // 기본 점광원 하나 추가 (맵 중앙 위)
    Light mainLight(glm::vec3(5.0f, 10.0f, 7.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 1.0f, 0.09f, 0.032f);
    addLight(mainLight);
    
    // 환경광 설정
    setAmbientLight(glm::vec3(0.2f, 0.2f, 0.2f), 0.1f);
    
    std::cout << "기본 조명 설정 완료" << std::endl;
}

void LightManager::setupCameraFollowLight(const glm::vec3& cameraPos) {
    if (!lights.empty() && lights[0].type == LightType::POINT) {
        // 카메라 근처에 조명 위치 업데이트
        glm::vec3 lightOffset(2.0f, 5.0f, 2.0f);
        updateLightPosition(0, cameraPos + lightOffset);
    }
}

void LightManager::setupSceneStaticLight() {
    clearLights();
    
    // 맵을 잘 비추는 메인 조명 설정 (더 밝게)
    Light mainLight(glm::vec3(5.0f, 15.0f, 7.5f), glm::vec3(1.2f, 1.2f, 1.2f), 1.2f, 1.0f, 0.05f, 0.02f);
    addLight(mainLight);
    
    // 환경광 설정 (더 밝게)
    setAmbientLight(glm::vec3(0.3f, 0.3f, 0.3f), 0.2f);
    
    std::cout << "정적 장면 조명 설정 완료" << std::endl;
}
