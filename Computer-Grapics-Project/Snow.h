#pragma once
#include <gl/glew.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <map>
#include <vector>
#include <string>

class Snow
{
private:
    // 눈 블록들을 저장하는 맵 (격자 좌표 -> 눈 높이)
    std::map<std::pair<int, int>, float> snowHeights;
    
    // 눈 렌더링을 위한 OpenGL 리소스
    mutable GLuint vao = 0;
    mutable GLuint vbo = 0;
    mutable GLuint nbo = 0;
    mutable GLuint tbo = 0;
    mutable GLsizei totalVertexCount = 0;
    mutable bool needsUpdate = true;
    
    // 텍스처 관련 멤버 변수
    mutable GLuint textureID = 0;
    mutable bool textureLoaded = false;
    std::string texturePath;

    // 월드 좌표를 격자 좌표로 변환
    std::pair<int, int> worldToGrid(float x, float z) const;
    
    // 격자 좌표를 월드 좌표로 변환
    glm::vec3 gridToWorld(int gridX, int gridZ, float height) const;
    
    // 눈 블록들을 위한 버퍼 업데이트
    void updateBuffers() const;
    
    // 단일 눈 블록의 버텍스 생성 (높이에 따른 텍스처 매핑 포함)
    void generateSnowBlock(float x, float y, float z, float width, float height, float depth,
                          std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, 
                          std::vector<glm::vec2>& texCoords) const;

    // 텍스처 로딩 함수
    void loadTexture() const;

    // 눈 생성 가능성 검사 함수들
    bool canSnowBeGenerated(int gridX, int gridZ) const;
    bool isAdjacentToWall(int gridX, int gridZ) const;
    bool isAdjacentToExistingSnow(int gridX, int gridZ) const;
    bool isAdjacentToMaxHeightSnow(int gridX, int gridZ) const;

public:
    Snow(const std::string& texturePath = "snow.png");
    ~Snow();
    
    // 특정 위치에 눈 추가 (0.5씩 증가, 최대 1.0)
    void addSnowAt(float x, float z);
    
    // 특정 위치의 눈 높이 조회 (격자 좌표)
    float getSnowHeightAt(int gridX, int gridZ) const;
    
    // 특정 위치의 눈 높이 조회 (월드 좌표)
    float getSnowHeightAtWorld(float x, float z) const;
    
    // 모든 눈 렌더링
    void render(GLuint shaderProgram) const;
    
    // 눈 초기화
    void clearAll();
    
    // Ground 영역 유효성 검사
    bool isValidGroundPosition(float x, float z) const;
};  

