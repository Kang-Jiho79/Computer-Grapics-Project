#pragma once
#include <gl/glew.h>
#include <gl/glm/glm.hpp>
#include <vector>
#include <string>

class Block
{
private:
    float x, y, z;  // 블록의 위치
    float size;     // 블록의 크기 (1x1 기본)
    
    // OpenGL 렌더링을 위한 멤버
    mutable GLuint vao = 0;
    mutable GLuint vbo = 0;
    mutable GLuint nbo = 0;
    mutable GLuint tbo = 0;  // 텍스처 좌표 버퍼
    mutable GLuint texture = 0;  // 텍스처 ID
    mutable GLsizei count = 0;
    mutable bool isInitialized = false;

    // 텍스처 관련
    std::string texturePath;
    mutable bool textureLoaded = false;

public:
    // 생성자
    Block();
    Block(float x, float y, float z, float size = 1.0f, const std::string& texturePath = "oak_planks.png");
    
    // 복사 생성자와 대입 연산자 추가
    Block(const Block& other);
    Block& operator=(const Block& other);
    
    // 접근자 함수
    float getX() const { return x; }
    float getY() const { return y; }
    float getZ() const { return z; }
    float getSize() const { return size; }
    
    // 설정자 함수
    void setPosition(float x, float y, float z);
    void setSize(float size);
    void setTexture(const std::string& texturePath);
    
    // 텍스처 로드 함수
    void loadTexture() const;
    
    // 초기화 및 렌더링 함수
    void initialize() const;
    void render(GLuint shaderProgram, const glm::vec3& color = glm::vec3(0.5f, 0.5f, 0.5f)) const;
    
    // 소멸자
    ~Block();
};

