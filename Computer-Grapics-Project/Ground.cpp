#include "Ground.h"
#include <gl/glew.h>

Ground::Ground(int width, int depth, float blockSize) 
    : width(width), depth(depth)
{
    // 2D 벡터 초기화
    blocks.resize(width);
    for (int i = 0; i < width; ++i) {
        blocks[i].resize(depth);
    }
}

int Ground::getWidth() const 
{
    return width;
}

int Ground::getDepth() const 
{
    return depth;
}

Block& Ground::getBlock(int x, int z) 
{
    return blocks[x][z];
}

const Block& Ground::getBlock(int x, int z) const 
{
    return blocks[x][z];
}

void Ground::initializeBlocks(float startX, float startY, float startZ, float blockSize)
{
    for (int x = 0; x < width; ++x) {
        for (int z = 0; z < depth; ++z) {
            float posX = startX + x * blockSize;
            float posZ = startZ + z * blockSize;
            blocks[x][z] = Block(posX, startY, posZ, blockSize);
        }
    }
}

void Ground::render() const
{
        extern GLuint shaderProgramID;
    extern glm::vec3 cameraPos;
    
    // 외부 셰이더 파일의 유니폼 변수명에 맞춤
    GLint lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
    GLint lightColorLoc = glGetUniformLocation(shaderProgramID, "lightColor");
    GLint viewPosLoc = glGetUniformLocation(shaderProgramID, "viewPos");
    GLint vColorLoc = glGetUniformLocation(shaderProgramID, "vColor");
    GLint lightingEnabledLoc = glGetUniformLocation(shaderProgramID, "lightingEnabled");
    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    GLint normalMatrixLoc = glGetUniformLocation(shaderProgramID, "normalMatrix");
    
    // 조명 활성화
    if (lightingEnabledLoc != -1) {
        glUniform1i(lightingEnabledLoc, 1); // true
    }
    
    // 조명 위치 설정
    if (lightPosLoc != -1) {
        glm::vec3 lightPos(5.0f, 8.0f, 7.5f);
        glUniform3fv(lightPosLoc, 1, &lightPos[0]);
    }
    
    // 조명 색상 설정
    if (lightColorLoc != -1) {
        glm::vec3 lightColor(1.5f, 1.5f, 1.5f);
        glUniform3fv(lightColorLoc, 1, &lightColor[0]);
    }
    
    // 카메라 위치 설정
    if (viewPosLoc != -1) {
        glUniform3fv(viewPosLoc, 1, &cameraPos[0]);
    }
    
    glm::vec3 groundColor(0.2f, 0.8f, 0.2f); // 초록색
    
    for (int x = 0; x < width; ++x) {
        for (int z = 0; z < depth; ++z) {
            blocks[x][z].render(shaderProgramID, groundColor);
        }
    }
}
