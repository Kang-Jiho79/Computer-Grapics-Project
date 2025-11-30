#pragma once
#include <gl/glew.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cmath>

// 전방 선언
class Snow;
class Map;

class Snowball
{
private:
    // 렌더링 관련
    GLuint vao, vbo, nbo, tbo;
    GLsizei vertexCount;
    bool isInitialized;

    // 물리 관련
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float gravity;
    float radius;
    float lifeTime;
    float maxLifeTime;
    bool isActive;

    // 구 생성 함수
    void generateSphere(float radius, int segments, std::vector<glm::vec3>& vertices,
        std::vector<glm::vec3>& normals, std::vector<glm::vec2>& texCoords);

    void initializeBuffers();

    // 충돌 검사 함수들
    bool checkWallCollision(const Map& gameMap) const;
    bool checkSnowCollision(const Snow& snowSystem) const;

public:
    Snowball();
    Snowball(const glm::vec3& startPos, const glm::vec3& direction, float speed = 10.0f, float radius = 0.1f);
    ~Snowball();

    // 복사 생성자와 대입 연산자 (OpenGL 리소스 관리)
    Snowball(const Snowball& other);
    Snowball& operator=(const Snowball& other);

    // 물리 업데이트 (Snow 시스템과 Map 추가)
    void update(float deltaTime, Snow& snowSystem, const Map& gameMap);

    // 렌더링
    void render(GLuint shaderProgram, const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f)) const;

    // 상태 확인
    bool getIsActive() const { return isActive; }
    glm::vec3 getPosition() const { return position; }
    float getRadius() const { return radius; }

    // 충돌 검사용
    bool checkCollision(const glm::vec3& point, float distance) const;

    // 눈덩이 제거
    void destroy() { isActive = false; }

    // 플레이어로부터 눈덩이 발사
    static Snowball createFromPlayer(const glm::vec3& playerPos, const glm::vec3& playerFront,
        const glm::vec3& playerUp = glm::vec3(0, 1, 0));
};
