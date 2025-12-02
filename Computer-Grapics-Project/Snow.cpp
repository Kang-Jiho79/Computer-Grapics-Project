#include "Snow.h"
#include "Map.h" // MAP_WIDTH, MAP_DEPTH, BLOCK_SIZE 상수를 위해
#include "stb_image.h" // 텍스처 로딩을 위해
#include <iostream>
#include <cmath> // std::floor를 위해

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
    std::cout << "눈 생성 시도: (" << x << ", " << z << ")" << std::endl;

    // 실제 Ground 영역에만 눈 생성 가능
    if (!isValidGroundPosition(x, z)) {
        std::cout << "Ground가 없는 위치입니다: (" << x << ", " << z << ")" << std::endl;
        std::cout << "앞쪽 Ground: z 0~5, 뒤쪽 Ground: z 10~15" << std::endl;
        return;
    }

    // 월드 좌표를 격자 좌표로 변환
    auto gridPos = worldToGrid(x, z);
    int targetGridX = gridPos.first;
    int targetGridZ = gridPos.second;

    std::cout << "유효한 Ground 위치: (" << x << ", " << z << ") -> 격자: (" << targetGridX << ", " << targetGridZ << ")" << std::endl;

    // 눈 생성 조건 검사: 벽에 닿거나 기존 눈에 닿아야 하고, 인접한 눈이 최대 높이여야 함
    if (!canSnowBeGenerated(targetGridX, targetGridZ)) {
        std::cout << "눈 생성 불가: 조건을 만족하지 않음" << std::endl;
        return;
    }

    // 현재 눈 높이 확인
    float currentHeight = getSnowHeightAt(targetGridX, targetGridZ);

    // 눈 높이 증가 (0.5씩, 최대 3.0)
    float newHeight = currentHeight + 0.5f;
    if (newHeight > 3.0f) {
        newHeight = 3.0f;
        std::cout << "이미 최대 높이의 눈이 쌓여있습니다." << std::endl;
        return;
    }

    // 눈 높이 업데이트
    snowHeights[{targetGridX, targetGridZ}] = newHeight;
    needsUpdate = true;

    std::cout << "눈 추가 성공! - 격자(" << targetGridX << ", " << targetGridZ
        << ") 높이: " << currentHeight << " -> " << newHeight << std::endl;
    std::cout << "현재 총 눈 블록 수: " << snowHeights.size() << std::endl;
}

float Snow::getSnowHeightAt(int gridX, int gridZ) const
{
    auto it = snowHeights.find({ gridX, gridZ });
    return (it != snowHeights.end()) ? it->second : 0.0f;
}

float Snow::getSnowHeightAtWorld(float x, float z) const
{
    auto gridPos = worldToGrid(x, z);
    return getSnowHeightAt(gridPos.first, gridPos.second);
}

void Snow::render(GLuint shaderProgram) const
{
    if (snowHeights.empty()) {
        return;
    }

    if (needsUpdate) {
        std::cout << "눈 버퍼 업데이트 중... (눈 블록 수: " << snowHeights.size() << ")" << std::endl;
        updateBuffers();
        needsUpdate = false;
    }

    // 텍스처 로드 (lazy loading)
    if (!textureLoaded) {
        loadTexture();
    }

    if (totalVertexCount > 0) {
        // 텍스처 바인딩
        if (textureLoaded && textureID != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
        }

        // 모델 행렬을 단위행렬로 설정 (눈은 월드 무대에 직접 배치)
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint normalMatrixLoc = glGetUniformLocation(shaderProgram, "normalMatrix");
        GLint useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
        GLint vColorLoc = glGetUniformLocation(shaderProgram, "vColor");
        GLint textureLoc = glGetUniformLocation(shaderProgram, "texture1");

        glm::mat4 model = glm::mat4(1.0f); // 단위행렬
        glm::mat3 normalMatrix = glm::mat3(1.0f); // 단위행렬

        if (modelLoc != -1) {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        }
        if (normalMatrixLoc != -1) {
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);
        }
        if (useTextureLoc != -1) {
            glUniform1i(useTextureLoc, textureLoaded ? 1 : 0); // 텍스처 사용
        }
        if (vColorLoc != -1) {
            glUniform3f(vColorLoc, 1.0f, 1.0f, 1.0f); // 흰색
        }
        if (textureLoc != -1) {
            glUniform1i(textureLoc, 0); // texture unit 0
        }

        // 렌더링
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, totalVertexCount);
        glBindVertexArray(0);

        // 텍스처 언바인딩
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Snow::clearAll()
{
    std::cout << "모든 눈 제거: " << snowHeights.size() << "개 블록" << std::endl;
    snowHeights.clear();
    needsUpdate = true;
    totalVertexCount = 0;
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
    }

    std::vector<glm::vec3> allVertices, allNormals;
    std::vector<glm::vec2> allTexCoords;

    // 모든 눈 블록의 버텍스 생성
    for (const auto& pair : snowHeights) {
        int gridX = pair.first.first;
        int gridZ = pair.first.second;
        float height = pair.second;

        glm::vec3 worldPos = gridToWorld(gridX, gridZ, height);

        std::cout << "눈 블록 생성 - 격자(" << gridX << ", " << gridZ << ") 높이: " << height
            << " 월드 위치: (" << worldPos.x << ", " << worldPos.y << ", " << worldPos.z << ")" << std::endl;
        std::cout << "  -> 블록 범위 Y: " << (worldPos.y - height / 2.0f) << " ~ " << (worldPos.y + height / 2.0f) << std::endl;
        std::cout << "  -> Ground 상단 Y=0.5, 눈 하단 Y=" << (worldPos.y - height / 2.0f) << std::endl;

        // gridToWorld에서 이미 중앙 좌표를 계산했으므로 그대로 사용
        generateSnowBlock(worldPos.x, worldPos.y, worldPos.z,
            BLOCK_SIZE, height, BLOCK_SIZE,
            allVertices, allNormals, allTexCoords);
    }

    totalVertexCount = static_cast<GLsizei>(allVertices.size());

    std::cout << "총 버텍스 수: " << totalVertexCount << std::endl;

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

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        std::cout << "눈 버퍼 업데이트 완료" << std::endl;
    }
}

void Snow::generateSnowBlock(float x, float y, float z, float width, float height, float depth,
    std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals,
    std::vector<glm::vec2>& texCoords) const
{
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;
    float halfDepth = depth / 2.0f;

    // 큐브의 8개 꼭짓점
    glm::vec3 v[8] = {
        {x - halfWidth, y - halfHeight, z - halfDepth}, // 0 (하단)
        {x + halfWidth, y - halfHeight, z - halfDepth}, // 1 (하단)
        {x + halfWidth, y + halfHeight, z - halfDepth}, // 2 (상단)
        {x - halfWidth, y + halfHeight, z - halfDepth}, // 3 (상단)
        {x - halfWidth, y - halfHeight, z + halfDepth}, // 4 (하단)
        {x + halfWidth, y - halfHeight, z + halfDepth}, // 5 (하단)
        {x + halfWidth, y + halfHeight, z + halfDepth}, // 6 (상단)
        {x - halfWidth, y + halfHeight, z + halfDepth}  // 7 (상단)
    };

    // 면 정의 (CCW 방향)
    int faces[6][6] = {
        {0, 3, 2, 2, 1, 0}, // 앞면 (z-)
        {4, 5, 6, 6, 7, 4}, // 뒷면 (z+)
        {0, 4, 7, 7, 3, 0}, // 왼쪽면 (x-)
        {1, 2, 6, 6, 5, 1}, // 오른쪽면 (x+)
        {3, 7, 6, 6, 2, 3}, // 윗면 (y+)
        {0, 1, 5, 5, 4, 0}  // 아랫면 (y-)
    };

    glm::vec3 faceNormals[6] = {
        {0, 0, -1}, // 앞면
        {0, 0, 1},  // 뒷면
        {-1, 0, 0}, // 왼쪽면
        {1, 0, 0},  // 오른쪽면
        {0, 1, 0},  // 윗면
        {0, -1, 0}  // 아랫면
    };

    // 높이에 따른 텍스처 매핑 계산
    // height에 따라 옆면 텍스처 비율 조정 (최대 3.0까지)
    float heightRatio = std::min(height / 3.0f, 1.0f); // 3.0f 기준으로 비율 계산
    float sideTexTop = heightRatio; // 옆면 상단 텍스처 좌표

    // 각 면에 대해 삼각형 생성
    for (int face = 0; face < 6; ++face) {
        for (int vertex = 0; vertex < 6; ++vertex) {
            vertices.push_back(v[faces[face][vertex]]);
            normals.push_back(faceNormals[face]);

            glm::vec2 texCoord;

            // 면에 따라 다른 텍스처 좌표 설정
            if (face == 4) { // 윗면 (y+) - 항상 전체 텍스처 사용
                switch (vertex % 4) {
                case 0: texCoord = { 0.0f, 0.0f }; break;
                case 1: texCoord = { 1.0f, 0.0f }; break;
                case 2: texCoord = { 1.0f, 1.0f }; break;
                case 3: texCoord = { 0.0f, 1.0f }; break;
                }
            }
            else if (face == 5) { // 아랫면 (y-) - 항상 전체 텍스처 사용
                switch (vertex % 4) {
                case 0: texCoord = { 0.0f, 0.0f }; break;
                case 1: texCoord = { 1.0f, 0.0f }; break;
                case 2: texCoord = { 1.0f, 1.0f }; break;
                case 3: texCoord = { 0.0f, 1.0f }; break;
                }
            }
            else { // 옆면들 (앞, 뒤, 좌, 우) - 높이에 따라 텍스처 조정
                switch (vertex % 4) {
                case 0: texCoord = { 0.0f, 0.0f }; break;        // 하단 왼쪽
                case 1: texCoord = { 1.0f, 0.0f }; break;        // 하단 오른쪽
                case 2: texCoord = { 1.0f, sideTexTop }; break;  // 상단 오른쪽 (높이에 따라)
                case 3: texCoord = { 0.0f, sideTexTop }; break;  // 상단 왼쪽 (높이에 따라)
                }
            }

            texCoords.push_back(texCoord);
        }
    }
}

// 눈이 생성될 수 있는지 확인하는 함수
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

// 기존 isAdjacentToExistingSnow 함수를 수정
bool Snow::isAdjacentToMaxHeightSnow(int gridX, int gridZ) const
{
    // 상하좌우 4방향 검사
    int directions[2][2] = {
        {0, 1},   // 위쪽
        {0, -1},  // 아래쪽
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