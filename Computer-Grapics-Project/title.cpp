#include "title.h"
#include <vector>

TitleScreen::TitleScreen() : vao(0), vbo(0), isInitialized(false) {}

TitleScreen::~TitleScreen() {
    cleanup();
}

void TitleScreen::initialize() {
    if (isInitialized) return;

    initializeBuffers();
    isInitialized = true;

    std::cout << "타이틀 화면 초기화 완료" << std::endl;
}

void TitleScreen::initializeBuffers() {
    // 전체 화면을 덮는 쿼드 생성
    std::vector<float> vertices = {
        // 위치          // 텍스처 좌표
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,

        -1.0f,  1.0f,   0.0f, 1.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 위치 속성
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // 텍스처 좌표 속성
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TitleScreen::render(GLuint shaderProgram) const {
    if (!isInitialized) return;

    // 깊이 테스트 비활성화 (2D 렌더링을 위해)
    glDisable(GL_DEPTH_TEST);

    glUseProgram(shaderProgram);

    // 단위 행렬들 설정
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
    GLint lightingEnabledLoc = glGetUniformLocation(shaderProgram, "lightingEnabled");
    GLint vColorLoc = glGetUniformLocation(shaderProgram, "vColor");

    if (modelLoc != -1) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    if (projLoc != -1) glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
    if (useTextureLoc != -1) glUniform1i(useTextureLoc, 0);
    if (lightingEnabledLoc != -1) glUniform1i(lightingEnabledLoc, 0);

    // 배경색 (어두운 파란색)
    if (vColorLoc != -1) glUniform3f(vColorLoc, 0.1f, 0.1f, 0.3f);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // 텍스트 렌더링 (간단한 버전)
    renderText("MINECRAFT SNOWBALL FIGHT", 0.0f, 0.3f, 2.0f);
    renderText("1 - First Person Mode", 0.0f, 0.0f, 1.0f);
    renderText("3 - Third Person Mode", 0.0f, -0.2f, 1.0f);
    renderText("ESC - Exit", 0.0f, -0.4f, 1.0f);

    // 깊이 테스트 다시 활성화
    glEnable(GL_DEPTH_TEST);
}

void TitleScreen::renderText(const std::string& text, float x, float y, float scale) const {
    // 간단한 콘솔 출력으로 대체 (실제 텍스트 렌더링은 복잡하므로)
    // 나중에 텍스처 기반 텍스트 렌더링으로 교체 가능
    static bool textDisplayed = false;
    if (!textDisplayed) {
        std::cout << "\n=== " << text << " ===" << std::endl;
        textDisplayed = true;
    }
}

GameState TitleScreen::handleKeyInput(unsigned char key) const {
    switch (key) {
    case '1':
        std::cout << "1인칭 모드로 게임 시작!" << std::endl;
        return GameState::FIRST_PERSON_MODE;
    case '3':
        std::cout << "3인칭 모드로 게임 시작!" << std::endl;
        return GameState::THIRD_PERSON_MODE;
    case 27: // ESC
    case 'q': case 'Q':
        std::cout << "게임 종료" << std::endl;
        exit(0);
        break;
    default:
        return GameState::TITLE_SCREEN;
    }
    return GameState::TITLE_SCREEN;
}

void TitleScreen::cleanup() {
    if (isInitialized) {
        if (vao != 0) {
            glDeleteVertexArrays(1, &vao);
            vao = 0;
        }
        if (vbo != 0) {
            glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
        isInitialized = false;
    }
}