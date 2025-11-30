#include "Snowball.h"
#include "Snow.h"
#include "Map.h" // BLOCK_SIZE, MAP_WIDTH, MAP_DEPTH를 위해
#include <iostream>

// BLOCK_SIZE 상수 사용을 위해 Map.h의 상수들을 참조
// 또는 직접 상수값 사용 (BLOCK_SIZE = 1.0f)
const float SNOWBALL_BLOCK_SIZE = 1.0f;  // Map.h의 BLOCK_SIZE와 동일한 값
const int SNOWBALL_MAP_WIDTH = 10;       // Map.h의 MAP_WIDTH와 동일한 값  
const int SNOWBALL_MAP_DEPTH = 15;       // Map.h의 MAP_DEPTH와 동일한 값

Snowball::Snowball() 
    : vao(0), vbo(0), nbo(0), tbo(0), vertexCount(0), isInitialized(false),
      position(0.0f), velocity(0.0f), acceleration(0.0f), gravity(-9.8f),
      radius(0.1f), lifeTime(0.0f), maxLifeTime(5.0f), isActive(false)
{
}

Snowball::Snowball(const glm::vec3& startPos, const glm::vec3& direction, float speed, float radius)
    : vao(0), vbo(0), nbo(0), tbo(0), vertexCount(0), isInitialized(false),
      position(startPos), gravity(-9.8f), radius(radius), lifeTime(0.0f), 
      maxLifeTime(5.0f), isActive(true)
{
    // 초기 속도 설정 (방향 * 속도)
    velocity = glm::normalize(direction) * speed;
    acceleration = glm::vec3(0.0f, gravity, 0.0f);
    
    initializeBuffers();
}

Snowball::~Snowball()
{
    if (isInitialized) {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &nbo);
        glDeleteBuffers(1, &tbo);
    }
}

Snowball::Snowball(const Snowball& other)
    : vao(0), vbo(0), nbo(0), tbo(0), vertexCount(0), isInitialized(false),
      position(other.position), velocity(other.velocity), acceleration(other.acceleration),
      gravity(other.gravity), radius(other.radius), lifeTime(other.lifeTime),
      maxLifeTime(other.maxLifeTime), isActive(other.isActive)
{
    if (other.isInitialized) {
        initializeBuffers();
    }
}

Snowball& Snowball::operator=(const Snowball& other)
{
    if (this != &other) {
        // 기존 리소스 해제
        if (isInitialized) {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
            glDeleteBuffers(1, &nbo);
            glDeleteBuffers(1, &tbo);
            isInitialized = false;
        }
        
        // 값 복사
        position = other.position;
        velocity = other.velocity;
        acceleration = other.acceleration;
        gravity = other.gravity;
        radius = other.radius;
        lifeTime = other.lifeTime;
        maxLifeTime = other.maxLifeTime;
        isActive = other.isActive;
        
        vao = vbo = nbo = tbo = 0;
        vertexCount = 0;
        
        if (other.isInitialized) {
            initializeBuffers();
        }
    }
    return *this;
}

void Snowball::generateSphere(float radius, int segments, std::vector<glm::vec3>& vertices, 
                             std::vector<glm::vec3>& normals, std::vector<glm::vec2>& texCoords)
{
    vertices.clear();
    normals.clear();
    texCoords.clear();

    const float PI = 3.14159265359f; // M_PI 대신 직접 정의

    for (int i = 0; i <= segments; ++i) {
        float lat = static_cast<float>(i) / segments * PI - PI / 2.0f; // -π/2 to π/2
        float cosLat = cos(lat);
        float sinLat = sin(lat);

        for (int j = 0; j <= segments; ++j) {
            float lon = static_cast<float>(j) / segments * 2.0f * PI; // 0 to 2π
            float cosLon = cos(lon);
            float sinLon = sin(lon);

            glm::vec3 vertex(radius * cosLat * cosLon, radius * sinLat, radius * cosLat * sinLon);
            glm::vec3 normal = glm::normalize(vertex);
            glm::vec2 texCoord(static_cast<float>(j) / segments, static_cast<float>(i) / segments);

            vertices.push_back(vertex);
            normals.push_back(normal);
            texCoords.push_back(texCoord);
        }
    }

    // 삼각형 인덱스 생성을 위해 정점을 재배열
    std::vector<glm::vec3> finalVertices, finalNormals;
    std::vector<glm::vec2> finalTexCoords;

    for (int i = 0; i < segments; ++i) {
        for (int j = 0; j < segments; ++j) {
            int current = i * (segments + 1) + j;
            int next = current + segments + 1;

            // 첫 번째 삼각형
            finalVertices.push_back(vertices[current]);
            finalVertices.push_back(vertices[next]);
            finalVertices.push_back(vertices[current + 1]);
            
            finalNormals.push_back(normals[current]);
            finalNormals.push_back(normals[next]);
            finalNormals.push_back(normals[current + 1]);
            
            finalTexCoords.push_back(texCoords[current]);
            finalTexCoords.push_back(texCoords[next]);
            finalTexCoords.push_back(texCoords[current + 1]);

            // 두 번째 삼각형
            finalVertices.push_back(vertices[next]);
            finalVertices.push_back(vertices[next + 1]);
            finalVertices.push_back(vertices[current + 1]);
            
            finalNormals.push_back(normals[next]);
            finalNormals.push_back(normals[next + 1]);
            finalNormals.push_back(normals[current + 1]);
            
            finalTexCoords.push_back(texCoords[next]);
            finalTexCoords.push_back(texCoords[next + 1]);
            finalTexCoords.push_back(texCoords[current + 1]);
        }
    }

    vertices = finalVertices;
    normals = finalNormals;
    texCoords = finalTexCoords;
}

void Snowball::initializeBuffers()
{
    std::vector<glm::vec3> vertices, normals;
    std::vector<glm::vec2> texCoords;
    
    generateSphere(radius, 16, vertices, normals, texCoords); // 16 segments for smooth sphere
    vertexCount = static_cast<GLsizei>(vertices.size());

    // OpenGL 버퍼 생성
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &nbo);
    glGenBuffers(1, &tbo);

    glBindVertexArray(vao);

    // 버텍스 위치 데이터
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    // 법선 데이터
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    // 텍스처 좌표 데이터
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    isInitialized = true;
}

// 기존 눈과의 충돌 검사
bool Snowball::checkSnowCollision(const Snow& snowSystem) const
{
    // Snow 클래스와 동일한 방식으로 격자 좌표 계산
    int gridX = static_cast<int>(std::floor(position.x / SNOWBALL_BLOCK_SIZE));
    int gridZ = static_cast<int>(std::floor(position.z / SNOWBALL_BLOCK_SIZE));
    
    // 주변 격자들도 확인 (눈덩이의 반지름 고려)
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dz = -1; dz <= 1; ++dz) {
            int checkGridX = gridX + dx;
            int checkGridZ = gridZ + dz;
            
            float snowHeight = snowSystem.getSnowHeightAt(checkGridX, checkGridZ);
            if (snowHeight > 0.0f) {
                // Snow.cpp의 gridToWorld와 동일한 방식으로 월드 좌표 계산
                float snowWorldX = checkGridX * SNOWBALL_BLOCK_SIZE; // 격자 왼쪽 하단 모서리
                float snowWorldZ = checkGridZ * SNOWBALL_BLOCK_SIZE; // 격자 왼쪽 하단 모서리
                
                // 눈 블록의 중심 위치 계산
                float snowCenterX = snowWorldX + SNOWBALL_BLOCK_SIZE / 2.0f;
                float snowCenterZ = snowWorldZ + SNOWBALL_BLOCK_SIZE / 2.0f;
                
                // Ground 상단(Y=0.5) 위의 눈 블록 정보
                const float GROUND_TOP = 0.5f;
                float snowBottomY = GROUND_TOP;
                float snowTopY = GROUND_TOP + snowHeight;
                float snowCenterY = GROUND_TOP + snowHeight / 2.0f;
                
                // 눈덩이와 눈 블록 중심 사이의 거리 계산
                float distanceX = abs(position.x - snowCenterX);
                float distanceZ = abs(position.z - snowCenterZ);
                float distanceY = abs(position.y - snowCenterY);
                
                // AABB 충돌 검사 (더 정확한 3D 충돌 검사)
                float blockHalfWidth = SNOWBALL_BLOCK_SIZE / 2.0f;
                float blockHalfHeight = snowHeight / 2.0f;
                
                bool collisionX = distanceX <= (blockHalfWidth + radius);
                bool collisionZ = distanceZ <= (blockHalfWidth + radius);
                bool collisionY = distanceY <= (blockHalfHeight + radius);
                
                if (collisionX && collisionZ && collisionY) {
                    std::cout << "=== 눈 충돌 감지 ===" << std::endl;
                    std::cout << "눈덩이 위치: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
                    std::cout << "격자(" << checkGridX << ", " << checkGridZ << ") 눈 높이: " << snowHeight << std::endl;
                    std::cout << "눈 블록 중심: (" << snowCenterX << ", " << snowCenterY << ", " << snowCenterZ << ")" << std::endl;
                    std::cout << "눈 블록 범위 Y: " << snowBottomY << " ~ " << snowTopY << std::endl;
                    std::cout << "거리 X/Z/Y: " << distanceX << "/" << distanceZ << "/" << distanceY << std::endl;
                    return true;
                }
            }
        }
    }
    return false;
}

void Snowball::update(float deltaTime, Snow& snowSystem, const Map& gameMap)
{
    if (!isActive) return;

    // 수명 업데이트
    lifeTime += deltaTime;
    if (lifeTime >= maxLifeTime) {
        isActive = false;
        std::cout << "눈덩이 수명 만료" << std::endl;
        return;
    }

    // 이전 위치 저장
    glm::vec3 prevPosition = position;

    // 물리 업데이트 (포물선 운동)
    velocity += acceleration * deltaTime;
    position += velocity * deltaTime;

    // 디버그: 눈덩이 위치 출력 (가끔씩)
    static int debugCounter = 0;
    if (++debugCounter % 30 == 0) { // 30프레임마다
        std::cout << "눈덩이 위치: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    }

    // 1. 벽 충돌 검사 (벽에 맞으면 눈 생성)
    if (checkWallCollision(gameMap)) {
        // 벽에 가까운 Ground 위치에 눈 생성
        glm::vec3 collisionPoint = position; // 현재 위치 사용
        
        // 벽 근처이지만 Ground 영역 내에 눈 생성
        float snowX = collisionPoint.x;
        float snowZ = collisionPoint.z;
        
        // Ground 영역 내로 조정 (SNOWBALL_BLOCK_SIZE 고려)
        if (snowX < 0) snowX = 0.5f * SNOWBALL_BLOCK_SIZE;
        if (snowX >= SNOWBALL_MAP_WIDTH * SNOWBALL_BLOCK_SIZE) snowX = (SNOWBALL_MAP_WIDTH - 0.5f) * SNOWBALL_BLOCK_SIZE;
        
        // 앞쪽 Ground(z: 0~5) 또는 뒤쪽 Ground(z: 10~15) 영역으로 조정
        if (snowZ < 0) {
            snowZ = 0.5f * SNOWBALL_BLOCK_SIZE; // 앞쪽 Ground로
        } else if (snowZ >= 5.0f * SNOWBALL_BLOCK_SIZE && snowZ < 10.0f * SNOWBALL_BLOCK_SIZE) {
            // 중간 빈 공간에 있다면 가까운 쪽으로
            if (snowZ < 7.5f * SNOWBALL_BLOCK_SIZE) {
                snowZ = (5.0f - 0.5f) * SNOWBALL_BLOCK_SIZE; // 앞쪽 Ground로
            } else {
                snowZ = 10.5f * SNOWBALL_BLOCK_SIZE; // 뒤쪽 Ground로
            }
        } else if (snowZ >= SNOWBALL_MAP_DEPTH * SNOWBALL_BLOCK_SIZE) {
            snowZ = (SNOWBALL_MAP_DEPTH - 0.5f) * SNOWBALL_BLOCK_SIZE; // 뒤쪽 Ground로
        }
        
        snowSystem.addSnowAt(snowX, snowZ);
        
        // 눈덩이 비활성화
        isActive = false;
        std::cout << "눈덩이가 벽에 충돌! 조정된 위치(" << snowX << ", " << snowZ << ")에 눈 생성" << std::endl;
        return;
    }

    // 2. 기존 눈과의 충돌 검사 (다른 눈에 맞으면 눈 높이 증가)
    if (checkSnowCollision(snowSystem)) {
        // 현재 위치에 눈 추가 (기존 눈 위에 쌓임)
        snowSystem.addSnowAt(position.x, position.z);
        
        // 눈덩이 비활성화
        isActive = false;
        std::cout << "눈덩이가 기존 눈에 충돌! 위치(" << position.x << ", " << position.z << ")에 눈 추가" << std::endl;
        return;
    }

    // 3. Ground 충돌 검사 (Ground에 닿으면 눈덩이만 사라짐, 눈 생성 안함)
    const float GROUND_TOP = 0.5f;
    if (position.y - radius <= GROUND_TOP) {
        // 눈덩이 비활성화 (눈 생성하지 않음)
        isActive = false;
        std::cout << "눈덩이가 Ground에 착지하여 사라짐 (눈 생성 안함)" << std::endl;
        return;
    }

    // 4. 맵 영역을 벗어나면 사라짐
    if (position.x < -2.0f || position.x > (SNOWBALL_MAP_WIDTH + 2) * SNOWBALL_BLOCK_SIZE || 
        position.z < -2.0f || position.z > (SNOWBALL_MAP_DEPTH + 2) * SNOWBALL_BLOCK_SIZE) {
        isActive = false;
        std::cout << "눈덩이가 맵 영역을 벗어나서 사라짐" << std::endl;
        return;
    }
}

bool Snowball::checkWallCollision(const Map& gameMap) const
{
    const Wall& wall = gameMap.getWall();
    
    // 디버그: 벽 블록 개수 확인
    static bool debugPrinted = false;
    if (!debugPrinted) {
        std::cout << "총 벽 블록 개수: " << wall.getBlockCount() << std::endl;
        debugPrinted = true;
    }
    
    for (size_t i = 0; i < wall.getBlockCount(); ++i) {
        const Block& block = wall.getBlock(i);
        
        // 블록의 위치와 크기
        glm::vec3 blockPos(block.getX(), block.getY(), block.getZ());
        float blockSize = block.getSize();
        
        // 확장된 충돌 영역 (더 관대하게)
        float collisionMargin = radius + 0.1f; // 여유 공간 추가
        
        // AABB 충돌 검사 (더 관대하게)
        glm::vec3 blockMin = blockPos - glm::vec3(blockSize / 2.0f + collisionMargin);
        glm::vec3 blockMax = blockPos + glm::vec3(blockSize / 2.0f + collisionMargin);
        
        // 눈덩이가 블록 영역에 있는지 확인
        if (position.x >= blockMin.x && position.x <= blockMax.x &&
            position.y >= blockMin.y && position.y <= blockMax.y &&
            position.z >= blockMin.z && position.z <= blockMax.z) {
            
            std::cout << "벽 충돌 감지!" << std::endl;
            std::cout << "눈덩이 위치: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
            std::cout << "블록 위치: (" << blockPos.x << ", " << blockPos.y << ", " << blockPos.z << ")" << std::endl;
            std::cout << "블록 크기: " << blockSize << std::endl;
            return true;
        }
    }
    
    return false;
}

void Snowball::render(GLuint shaderProgram, const glm::vec3& color) const
{
    if (!isActive || !isInitialized) return;

    // 모델 행렬 생성 (눈덩이의 위치로 이동)
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    
    // 법선 행렬 계산
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    
    // 셰이더에 유니폼 전달
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint normalMatrixLoc = glGetUniformLocation(shaderProgram, "normalMatrix");
    GLint vColorLoc = glGetUniformLocation(shaderProgram, "vColor");
    GLint useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
    
    if (modelLoc != -1) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    }
    if (normalMatrixLoc != -1) {
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);
    }
    if (vColorLoc != -1) {
        glUniform3f(vColorLoc, color.r, color.g, color.b);
    }
    if (useTextureLoc != -1) {
        glUniform1i(useTextureLoc, 0); // 텍스처 사용하지 않음
    }

    // 렌더링
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}

bool Snowball::checkCollision(const glm::vec3& point, float distance) const
{
    if (!isActive) return false;
    
    float dist = glm::distance(position, point);
    return dist <= (radius + distance);
}

Snowball Snowball::createFromPlayer(const glm::vec3& playerPos, const glm::vec3& playerFront, const glm::vec3& playerUp)
{
    // 플레이어 앞쪽으로 약간 올린 위치에서 시작
    glm::vec3 startPos = playerPos + playerFront * 1.0f + playerUp * 0.5f;
    
    // 약간 위쪽으로 향하는 방향 (포물선을 위해)
    glm::vec3 direction = playerFront + playerUp * 0.3f;
    
    std::cout << "눈덩이 생성 - 시작 위치: (" << startPos.x << ", " << startPos.y << ", " << startPos.z 
              << ") 방향: (" << direction.x << ", " << direction.y << ", " << direction.z << ")" << std::endl;
    
    return Snowball(startPos, direction, 12.0f, 0.15f); // 속도 12, 반지름 0.15
}
