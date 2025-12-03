#pragma once
#include <gl/glew.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <map>
#include <vector>
#include <string>

// 눈 애니메이션 정보 구조체
struct SnowAnimationData {
    float targetHeight;     // 최종 목표 높이
    float currentHeight;    // 현재 높이 (애니메이션 중)
    float animationTime;    // 애니메이션 경과 시간
    float animationDuration; // 애니메이션 지속 시간
    bool isAnimating;       // 애니메이션 중인지 여부
    float alphaValue;       // 투명도 (페이드 인 효과)

    SnowAnimationData() : targetHeight(0.0f), currentHeight(0.0f),
        animationTime(0.0f), animationDuration(1.0f),
        isAnimating(false), alphaValue(0.0f) {
    }
};

class Snow
{
private:
    // 눈 높이와 애니메이션 데이터를 저장하는 맵
    std::map<std::pair<int, int>, SnowAnimationData> snowData;

    // 눈 렌더링을 위한 OpenGL 리소스
    mutable GLuint vao = 0;
    mutable GLuint vbo = 0;
    mutable GLuint nbo = 0;
    mutable GLuint tbo = 0;
    mutable GLuint alphaVBO = 0; // 알파값을 위한 VBO 추가
    mutable GLsizei totalVertexCount = 0;
    mutable bool needsUpdate = true;

    // 텍스처 관련 멤버 변수
    mutable GLuint textureID = 0;
    mutable bool textureLoaded = false;
    std::string texturePath;

    // 애니메이션 설정
    float defaultAnimationDuration = 1.5f; // 기본 애니메이션 시간 (1.5초)
    float growthRate = 2.0f; // 성장 속도 배율

    // 좌표 변환 함수들
    std::pair<int, int> worldToGrid(float x, float z) const;
    glm::vec3 gridToWorld(int gridX, int gridZ, float height) const;

    // 버퍼 업데이트 및 렌더링 함수들
    void updateBuffers() const;
    void generateSnowBlock(float x, float y, float z, float width, float height, float depth,
        std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals,
        std::vector<glm::vec2>& texCoords, std::vector<float>& alphas,
        float alpha) const;

    // 텍스처 로드 함수
    void loadTexture() const;

    // 눈 생성 조건 검사 함수들
    bool canSnowBeGenerated(int gridX, int gridZ) const;
    bool isAdjacentToWall(int gridX, int gridZ) const;
    bool isAdjacentToExistingSnow(int gridX, int gridZ) const;
    bool isAdjacentToMaxHeightSnow(int gridX, int gridZ) const;

    // 애니메이션 헬퍼 함수들
    float easeOutQuart(float t) const; // 부드러운 성장 커브
    float easeInOut(float t) const;    // 페이드 인 커브

public:
    Snow(const std::string& texturePath = "snow.png");
    ~Snow();

    // 특정 위치에 눈 추가 (애니메이션과 함께)
    void addSnowAt(float x, float z);

    // 애니메이션 업데이트 (매 프레임 호출 필요)
    void updateAnimations(float deltaTime);

    // 특정 위치의 눈 높이 조회 (그리드 좌표)
    float getSnowHeightAt(int gridX, int gridZ) const;

    // 특정 위치의 눈 높이 조회 (월드 좌표)
    float getSnowHeightAtWorld(float x, float z) const;

    // 모든 눈 렌더링
    void render(GLuint shaderProgram) const;

    // 눈 초기화
    void clearAll();

    // Ground 영역 유효성 검사
    bool isValidGroundPosition(float x, float z) const;

    // 애니메이션 설정 함수들
    void setAnimationDuration(float duration) { defaultAnimationDuration = duration; }
    void setGrowthRate(float rate) { growthRate = rate; }
};