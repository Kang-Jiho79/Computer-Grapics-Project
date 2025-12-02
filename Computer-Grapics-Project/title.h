#pragma once
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <iostream>

enum class GameState {
    TITLE_SCREEN,
    FIRST_PERSON_MODE,
    THIRD_PERSON_MODE
};

class TitleScreen {
private:
    GLuint vao, vbo;
    bool isInitialized;

    void initializeBuffers();
    void renderText(const std::string& text, float x, float y, float scale = 1.0f) const;

public:
    TitleScreen();
    ~TitleScreen();

    void initialize();
    void render(GLuint shaderProgram) const;
    void cleanup();

    // 키 입력 처리 - 게임 상태 반환
    GameState handleKeyInput(unsigned char key) const;
};