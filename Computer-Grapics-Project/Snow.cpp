#include "Snow.h"
#include "Map.h"
#include "stb_image.h"
#include <iostream>
#include <cmath>
#include <algorithm>

Snow::Snow(const std::string& texturePath) : texturePath(texturePath)
{
}

Snow::~Snow()
{
    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &nbo);
        glDeleteBuffers(1, &tbo);
        if (alphaVBO != 0) glDeleteBuffers(1, &alphaVBO);
    }

    if (textureLoaded && textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
}

void Snow::loadTexture() const
{
    if (textureLoaded) return;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // 텍스처 파라미터 설정
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 이미지 로드
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);

    if (data) {
        GLenum format;
        if (channels == 1)
            format = GL_RED;
        else if (channels == 3)
            format = GL_RGB;
        else if (channels == 4)
            format = GL_RGBA;
        else
            format = GL_RGB; // 기본값

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        std::cout << "눈 텍스처 로드 성공: " << texturePath << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;
        textureLoaded = true;
    }
    else {
        std::cerr << "눈 텍스처 로드 실패: " << texturePath << std::endl;
        // 기본 텍스처 생성 (흰색)
        unsigned char whitePixel[3] = { 255, 255, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, whitePixel);
        textureLoaded = true;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool Snow::isValidGroundPosition(float x, float z) const
{
    // Ground 영역 체크: 앞쪽 Ground (x: 0~10, z: 0~5) 또는 뒤쪽 Ground (x: 0~10, z: 10~15)
    if (x < 0 || x >= MAP_WIDTH * BLOCK_SIZE) {
        return false;
    }

    // 앞쪽 Ground 영역 (z: 0~5)
    bool frontGroundArea = (z >= 0 && z < 5.0f * BLOCK_SIZE);

    // 뒤쪽 Ground 영역 (z: 10~15)  
    bool backGroundArea = (z >= 10.0f * BLOCK_SIZE && z < MAP_DEPTH * BLOCK_SIZE);

    return frontGroundArea || backGroundArea;
}

void Snow::addSnowAt(float x, float z)
{
    std::cout << "애니메이션과 함께 눈 생성 시도: (" << x << ", " << z << ")" << std::endl;

    // 유효한 Ground 영역인지 눈 생성 확인
    if (!isValidGroundPosition(x, z)) {
        std::cout << "Ground가 아닌 위치입니다: (" << x << ", " << z << ")" << std::endl;
        return;
    }

    auto gridPos = worldToGrid(x, z);
    int targetGridX = gridPos.first;
    int targetGridZ = gridPos.second;

    if (!canSnowBeGenerated(targetGridX, targetGridZ)) {
        std::cout << "눈 생성 불가: 조건을 만족하지 않음" << std::endl;
        return;
    }

    // 현재 눈 데이터 가져오기
    auto& snowAnimData = snowData[{targetGridX, targetGridZ}];
    
    // 기존 높이 저장 (애니메이션 시작점)
    float previousHeight = snowAnimData.targetHeight;
    
    // 새로운 높이 계산 (0.5씩 증가, 최대 3.0)
    float newTargetHeight = previousHeight + 0.5f;
    if (newTargetHeight > 3.0f) {
        newTargetHeight = 3.0f;
        std::cout << "이미 최대 높이에 도달했습니다." << std::endl;
        return;
    }

    // 애니메이션 데이터 설정
    snowAnimData.targetHeight = newTargetHeight;
    snowAnimData.currentHeight = previousHeight; // 기존 높이에서 시작
    snowAnimData.animationTime = 0.0f;
    snowAnimData.animationDuration = defaultAnimationDuration;
    snowAnimData.isAnimating = true;
    
    // 기존 높이가 있으면 불투명하게 시작, 없으면 투명하게 시작
    snowAnimData.alphaValue = (previousHeight > 0.0f) ? 1.0f : 0.0f;
    
    needsUpdate = true;

    std::cout << "눈 애니메이션 시작! - 그리드(" << targetGridX << ", " << targetGridZ
              << ") 시작 높이: " << previousHeight << " -> 목표 높이: " << newTargetHeight << std::endl;
}

void Snow::updateAnimations(float deltaTime)
{
    bool hasActiveAnimations = false;
    
    for (auto& pair : snowData) {
        auto& animData = pair.second;
        
        if (animData.isAnimating) {
            hasActiveAnimations = true;
            animData.animationTime += deltaTime;
            
            float progress = animData.animationTime / animData.animationDuration;
            progress = std::min(progress, 1.0f);
            
            // 애니메이션 시작 높이 계산 (목표에서 0.5를 뺀 값)
            float startHeight = animData.targetHeight - 0.5f;
            if (startHeight < 0.0f) startHeight = 0.0f;
            
            // 부드러운 성장 애니메이션 (ease-out)
            float easedProgress = easeOutQuart(progress);
            float heightDifference = animData.targetHeight - startHeight;
            animData.currentHeight = startHeight + (heightDifference * easedProgress);
            
            // 페이드 인 효과 (기존 높이가 있으면 스킵)
            if (startHeight <= 0.0f) {
                float fadeProgress = easeInOut(progress);
                animData.alphaValue = fadeProgress;
            } else {
                animData.alphaValue = 1.0f; // 기존 높이가 있으면 항상 불투명
            }
            
            // 애니메이션 완료 체크
            if (progress >= 1.0f) {
                animData.isAnimating = false;
                animData.currentHeight = animData.targetHeight;
                animData.alphaValue = 1.0f;
                std::cout << "눈 애니메이션 완료 - 시작: " << startHeight 
                         << " -> 최종 높이: " << animData.targetHeight << std::endl;
            }
            
            needsUpdate = true;
        }
    }
    
    // 활성화된 애니메이션이 있으면 버퍼 업데이트 플래그 설정
    if (hasActiveAnimations) {
        needsUpdate = true;
    }
}

float Snow::easeOutQuart(float t) const
{
    return 1.0f - std::pow(1.0f - t, 4.0f);
}

float Snow::easeInOut(float t) const
{
    if (t < 0.5f) {
        return 2.0f * t * t;
    }
    else {
        return 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
    }
}

float Snow::getSnowHeightAt(int gridX, int gridZ) const
{
    auto it = snowData.find({ gridX, gridZ });
    return (it != snowData.end()) ? it->second.currentHeight : 0.0f;
}

float Snow::getSnowHeightAtWorld(float x, float z) const
{
    auto gridPos = worldToGrid(x, z);
    return getSnowHeightAt(gridPos.first, gridPos.second);
}

std::pair<int, int> Snow::worldToGrid(float x, float z) const
{
    // 월드 좌표를 격자 좌표로 변환
    int gridX = static_cast<int>(std::floor(x / BLOCK_SIZE));
    int gridZ = static_cast<int>(std::floor(z / BLOCK_SIZE));
    return { gridX, gridZ };
}

glm::vec3 Snow::gridToWorld(int gridX, int gridZ, float height) const
{
    // Ground 블록과 동일한 방식으로 배치 (격자의 왼쪽 하단 모서리)
    float worldX = gridX * BLOCK_SIZE; // Ground와 동일: 0, 1, 2, 3, ...
    float worldZ = gridZ * BLOCK_SIZE; // Ground와 동일: 0, 1, 2, 3, ...

    // Ground 블록 상단(Y=0.5) 위에 눈 블록의 중심이 오도록 배치
    const float GROUND_TOP = 0.5f;
    float worldY = GROUND_TOP + height / 2.0f; // 눈 블록의 중심 Y 좌표

    return glm::vec3(worldX, worldY, worldZ);
}

void Snow::updateBuffers() const
{
    if (vao == 0) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &nbo);
        glGenBuffers(1, &tbo);
        glGenBuffers(1, &alphaVBO);
    }

    std::vector<glm::vec3> allVertices, allNormals;
    std::vector<glm::vec2> allTexCoords;
    std::vector<float> allAlphas;

    // 모든 눈 블록의 버텍스 생성
    for (const auto& pair : snowData) {
        int gridX = pair.first.first;
        int gridZ = pair.first.second;
        const auto& animData = pair.second;

        if (animData.currentHeight <= 0.0f) continue; // 높이가 0이면 스킵

        glm::vec3 worldPos = gridToWorld(gridX, gridZ, animData.currentHeight);

        generateSnowBlock(worldPos.x, worldPos.y, worldPos.z,
            BLOCK_SIZE, animData.currentHeight, BLOCK_SIZE,
            allVertices, allNormals, allTexCoords, allAlphas,
            animData.alphaValue);
    }

    totalVertexCount = static_cast<GLsizei>(allVertices.size());

    if (totalVertexCount > 0) {
        glBindVertexArray(vao);

        // 버텍스 위치 데이터
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(glm::vec3),
            allVertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        // 법선 데이터
        glBindBuffer(GL_ARRAY_BUFFER, nbo);
        glBufferData(GL_ARRAY_BUFFER, allNormals.size() * sizeof(glm::vec3),
            allNormals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        // 텍스처 좌표 데이터
        glBindBuffer(GL_ARRAY_BUFFER, tbo);
        glBufferData(GL_ARRAY_BUFFER, allTexCoords.size() * sizeof(glm::vec2),
            allTexCoords.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

        // 알파 값 데이터 (attribute location 3)
        glBindBuffer(GL_ARRAY_BUFFER, alphaVBO);
        glBufferData(GL_ARRAY_BUFFER, allAlphas.size() * sizeof(float),
            allAlphas.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void Snow::generateSnowBlock(float x, float y, float z, float width, float height, float depth,
    std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals,
    std::vector<glm::vec2>& texCoords, std::vector<float>& alphas,
    float alpha) const
{
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;
    float halfDepth = depth / 2.0f;

    // 큐브의 8개 꼭짓점
    glm::vec3 v[8] = {
        {x - halfWidth, y - halfHeight, z - halfDepth}, // 0
        {x + halfWidth, y - halfHeight, z - halfDepth}, // 1
        {x + halfWidth, y + halfHeight, z - halfDepth}, // 2
        {x - halfWidth, y + halfHeight, z - halfDepth}, // 3
        {x - halfWidth, y - halfHeight, z + halfDepth}, // 4
        {x + halfWidth, y - halfHeight, z + halfDepth}, // 5
        {x + halfWidth, y + halfHeight, z + halfDepth}, // 6
        {x - halfWidth, y + halfHeight, z + halfDepth}  // 7
    };

    // 6개 면 (CCW 순서)
    int faces[6][6] = {
        {0, 3, 2, 2, 1, 0}, // 전면
        {4, 5, 6, 6, 7, 4}, // 후면
        {0, 4, 7, 7, 3, 0}, // 좌측면
        {1, 2, 6, 6, 5, 1}, // 우측면
        {3, 7, 6, 6, 2, 3}, // 윗면
        {0, 1, 5, 5, 4, 0}  // 아랫면
    };

    glm::vec3 faceNormals[6] = {
        {0, 0, -1}, {0, 0, 1}, {-1, 0, 0},
        {1, 0, 0}, {0, 1, 0}, {0, -1, 0}
    };

    // 높이에 따른 텍스처 좌표 계산
    float heightRatio = std::min(height / 3.0f, 1.0f);
    float sideTexTop = heightRatio;

    // 각 면에 대해 삼각형 생성
    for (int face = 0; face < 6; ++face) {
        for (int vertex = 0; vertex < 6; ++vertex) {
            vertices.push_back(v[faces[face][vertex]]);
            normals.push_back(faceNormals[face]);
            alphas.push_back(alpha); // 각 버텍스에 알파값 추가

            glm::vec2 texCoord;
            if (face == 4 || face == 5) { // 윗면/아랫면
                switch (vertex % 4) {
                case 0: texCoord = { 0.0f, 0.0f }; break;
                case 1: texCoord = { 1.0f, 0.0f }; break;
                case 2: texCoord = { 1.0f, 1.0f }; break;
                case 3: texCoord = { 0.0f, 1.0f }; break;
                }
            }
            else { // 측면들
                switch (vertex % 4) {
                case 0: texCoord = { 0.0f, 0.0f }; break;
                case 1: texCoord = { 1.0f, 0.0f }; break;
                case 2: texCoord = { 1.0f, sideTexTop }; break;
                case 3: texCoord = { 0.0f, sideTexTop }; break;
                }
            }
            texCoords.push_back(texCoord);
        }
    }
}

void Snow::render(GLuint shaderProgram) const
{
    if (snowData.empty()) return;

    if (needsUpdate) {
        updateBuffers();
        needsUpdate = false;
    }

    if (!textureLoaded) {
        loadTexture();
    }

    if (totalVertexCount > 0) {
        // 블렌딩 활성화 (투명도 지원)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 텍스처 바인딩
        if (textureLoaded && textureID != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
        }

        // 유니폼 설정
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
        GLint vColorLoc = glGetUniformLocation(shaderProgram, "vColor");

        glm::mat4 model = glm::mat4(1.0f);
        if (modelLoc != -1) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        if (useTextureLoc != -1) glUniform1i(useTextureLoc, textureLoaded ? 1 : 0);
        if (vColorLoc != -1) glUniform3f(vColorLoc, 1.0f, 1.0f, 1.0f);

        // 렌더링
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, totalVertexCount);
        glBindVertexArray(0);

        // 블렌딩 비활성화
        glDisable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Snow::clearAll()
{
    std::cout << "모든 눈 제거: " << snowData.size() << "개 블록" << std::endl;
    snowData.clear();
    needsUpdate = true;
    totalVertexCount = 0;
}

bool Snow::canSnowBeGenerated(int gridX, int gridZ) const
{
    // 이미 이 위치에 눈이 최대 높이까지 쌓여있다면 생성 불가
    if (getSnowHeightAt(gridX, gridZ) >= 3.0f) {
        std::cout << "이미 최대 높이의 눈이 쌓여있음" << std::endl;
        return false;
    }

    // 1. 벽에 닿는지 확인
    if (isAdjacentToWall(gridX, gridZ)) {
        std::cout << "벽에 인접한 위치 - 눈 생성 가능" << std::endl;
        return true;
    }

    // 2. 기존 눈에 닿으면서, 그 눈이 최대 높이(3.0)인지 확인
    if (isAdjacentToMaxHeightSnow(gridX, gridZ)) {
        std::cout << "최대 높이 눈에 인접한 위치 - 눈 생성 가능" << std::endl;
        return true;
    }

    std::cout << "격자(" << gridX << ", " << gridZ << ")는 생성 조건을 만족하지 않음" << std::endl;
    return false;
}

bool Snow::isAdjacentToMaxHeightSnow(int gridX, int gridZ) const
{
    // 상하좌우 4방향 검사
    int directions[4][2] = {
        {0, 1},   // 위쪽
        {0, -1},  // 아래쪽
        {1, 0},   // 오른쪽
        {-1, 0}   // 왼쪽
    };

    for (int i = 0; i < 4; i++) {
        int checkX = gridX + directions[i][0];
        int checkZ = gridZ + directions[i][1];

        // 인접한 위치에 최대 높이(3.0)의 눈이 있는지 확인
        float adjacentHeight = getSnowHeightAt(checkX, checkZ);
        if (adjacentHeight >= 1.0f) {
            std::cout << "인접 위치(" << checkX << ", " << checkZ << ")에 최대 높이 눈 존재 (높이: "
                << adjacentHeight << ")" << std::endl;
            return true;
        }
    }

    return false;
}

bool Snow::isAdjacentToWall(int gridX, int gridZ) const
{
    // 앞쪽 Ground 영역 (z: 0~4)
    if (gridZ >= 0 && gridZ < 5) {
        // 앞쪽 벽 (z=0)에 닿는지 확인
        if (gridZ == 0) {
            std::cout << "앞쪽 벽(z=0)에 인접" << std::endl;
            return true;
        }

        // 왼쪽/오른쪽 벽에 닿는지 확인
        if (gridX == 0 || gridX == 9) { // MAP_WIDTH-1 = 9
            std::cout << "좌우 벽(x=" << gridX << ")에 인접" << std::endl;
            return true;
        }

        // 중간 공간과의 경계 (z=4)
        if (gridZ == 4) {
            std::cout << "중간 공간 경계(z=4)에 인접" << std::endl;
            return true;
        }
    }
    // 뒤쪽 Ground 영역 (z: 10~14)
    else if (gridZ >= 10 && gridZ < 15) {
        // 뒤쪽 벽 (z=14)에 닿는지 확인
        if (gridZ == 14) {
            std::cout << "뒤쪽 벽(z=14)에 인접" << std::endl;
            return true;
        }

        // 왼쪽/오른쪽 벽에 닿는지 확인
        if (gridX == 0 || gridX == 9) { // MAP_WIDTH-1 = 9
            std::cout << "좌우 벽(x=" << gridX << ")에 인접" << std::endl;
            return true;
        }

        // 중간 공간과의 경계 (z=10)
        if (gridZ == 10) {
            std::cout << "중간 공간 경계(z=10)에 인접" << std::endl;
            return true;
        }
    }

    return false;
}

bool Snow::isAdjacentToExistingSnow(int gridX, int gridZ) const
{
    // 이 함수는 isAdjacentToMaxHeightSnow로 대체됨
    return isAdjacentToMaxHeightSnow(gridX, gridZ);
}