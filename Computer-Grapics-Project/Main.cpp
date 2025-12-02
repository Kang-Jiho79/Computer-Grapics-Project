#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>
#include <algorithm>  // std::remove_if를 위해 추가
#include "Map.h"      // 맵 헤더
#include "Light.h"    // 조명 헤더 추가
//#include "stb_image.h"  // stb_image 헤더 추가
#include "Steve.h"      // Steve 헤더 추가
#include "Alex.h"       // Alex 헤더 추가
#include "Camera.h"     // Camera 헤더 추가
#include "Steve_Camera.h"  // Steve 1인칭 카메라
#include "Alex_Camera.h"   // Alex 1인칭 카메라
#include "Snowball.h"      // Snowball 헤더 추가
#include "Snow.h"       // Snow 헤더 추가
#include "KeyManager.h"
#include "title.h"       // 타이틀 헤더 추가

#define WinX 1280
#define WinY 720

// stb_image 포함 (텍스처 로딩을 위해)

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;

	fptr = fopen(file, "rb");
	if (!fptr)
		return NULL;
	fseek(fptr, 0, SEEK_END);
	length = ftell(fptr);
	buf = (char*)malloc(length + 1);
	fseek(fptr, 0, SEEK_SET);
	fread(buf, length, 1, fptr);
	fclose(fptr);
	buf[length] = 0;
	return buf;
}

// 전역 변수 정의 (다른 파일에서 extern으로 접근 가능)
glm::vec3 cameraPos = glm::vec3(5.0f, 8.0f, 12.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// 게임 상태 관리
static GameState currentGameState = GameState::TITLE_SCREEN;
static TitleScreen titleScreen;

// 전역 객체들
static Camera camera;  // 기본 3인칭 카메라 객체
static Steve_Camera steveCamera; // Steve 1인칭 카메라
static Alex_Camera alexCamera;   // Alex 1인칭 카메라
static LightManager lightManager;
static Map gameMap;
static Steve::Character* steve = nullptr;
static Alex::Character* alex = nullptr;
static Snow snowSystem("snow.png"); // 눈 시스템 - 텍스처 경로 지정
static KeyManager input;

static bool steveThrowFlag = false;
static bool alexThrowFlag = false;

// 캐릭터 위치 관리
static glm::vec3 stevePosition = glm::vec3(3.0f, 0.8f, 5.0f);
static glm::vec3 alexPosition = glm::vec3(7.0f, 0.8f, 5.0f);

// 분할 화면 모드 관련
static bool splitScreenMode = false; // 분할 화면 모드 토글

// 기타 전역 변수들
bool cameraLightMode = true;  // 카메라 따라다니는 조명 모드
enum CharacterSelection { STEVE, ALEX };
static CharacterSelection activeCharacter = STEVE;

// 마우스 상태
bool firstMouse = true;
float lastX = WinX / 2.0f;
float lastY = WinY / 2.0f;

// 활성화된 눈덩이들
static std::vector<Snowball> snowballs; // 활성화된 눈덩이들

// 눈덩이 차징 관련 변수들 (Steve용)
static bool steveCharging = false;
static float steveChargeStartTime = 0.0f;

// 눈덩이 차징 관련 변수들 (Alex용)
static bool alexCharging = false;
static float alexChargeStartTime = 0.0f;

static float maxChargeTime = 2.5f; // 최대 차징 시간 (3초에서 2.5초로 단축)
static float minSpeed = 5.0f;     // 최소 속도 (더욱 증가)
static float maxSpeed = 40.0f;     // 최대 속도 (더욱 증가)

// 2D 얼굴 렌더링을 위한 버퍼와 텍스처
static GLuint faceVAO = 0, faceVBO = 0;
static bool faceBuffersInitialized = false;
static GLuint steveFaceTextureID = 0;
static GLuint alexFaceTextureID = 0;
static bool faceTexturesLoaded = false;

// 함수 선언
void Mouse(int button, int state, int x, int y);
void MouseMotion(int x, int y);
void Keyboard(unsigned char key, int x, int y);
void KeyboardUp(unsigned char key, int x, int y); // 키보드 릴리즈 함수 추가
void SpecialKeyboard(int key, int x, int y);
void SpecialKeyboardUp(int key, int x, int y); // 추가: 특수키 릴리즈 선언
void TimerFunction(int value);

void make_shaderProgram();
GLvoid drawScene();
GLvoid drawSplitScreen(); // 분할 화면 렌더링 함수
GLvoid renderWorld(const glm::mat4& view, const glm::mat4& projection); // 월드 렌더링 함수
GLvoid Reshape(int w, int h);

// 게임 초기화 함수
void initializeGame();

// 눈덩이 발사 함수들
void fireSteveSnowball();
void fireAlexSnowball();

// 충돌 처리 함수들
bool checkSnowballCharacterCollision(const Snowball& snowball, const glm::vec3& characterPos, const glm::vec3& boundingBoxSize);
void checkAllSnowballCollisions();

// 얼굴 렌더링 함수들
void initializeFaceBuffers();
void loadFaceTextures();
GLuint loadTexture(const char* path);
void renderCharacterFace(GLuint textureID, float x, float y, float size, int screenWidth, int screenHeight);
void drawCharacterFaces(int winW, int winH);

//--- 셰이더 관련 변수들
GLint width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;
GLchar* vertexSource, * fragmentSource;

//--- 메인 함수 (반환 타입을 int로 변경)
int main(int argc, char** argv)
{
	std::cout << "프로그램 시작" << std::endl;

	//--- 윈도우생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WinX, WinY);
	glutCreateWindow("Minecraft Snowball Fight");

	std::cout << "GLUT 초기화 완료" << std::endl;

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	GLenum glewStatus = glewInit();
	if (glewStatus != GLEW_OK) {
		std::cerr << "GLEW 초기화 실패: " << glewGetErrorString(glewStatus) << std::endl;
		return 1;
	}

	// OpenGL 버전 확인
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	//--- 세이더읽어와서세이더프로그램만들기
	make_shaderProgram();

	// 셰이더 프로그램이 제대로 생성되었는지 확인
	if (shaderProgramID == 0) {
		std::cerr << "셰이더 프로그램 생성 실패" << std::endl;
		return 1;
	}

	std::cout << "셰이더 프로그램 생성 완료" << std::endl;

	// 타이틀 화면 초기화
	titleScreen.initialize();

	std::cout << "\n=== MINECRAFT SNOWBALL FIGHT ===" << std::endl;
	std::cout << "1: 1인칭 모드로 게임 시작" << std::endl;
	std::cout << "3: 3인칭 모드로 게임 시작" << std::endl;
	std::cout << "ESC: 종료" << std::endl;

	//--- 콜백 등록
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutKeyboardUpFunc(KeyboardUp); // 키보드 릴리즈 콜백 추가
	glutSpecialFunc(SpecialKeyboard);
	glutSpecialUpFunc(SpecialKeyboardUp); // ← 추가: 특수키 릴리즈 등록
	glutMouseFunc(Mouse);
	glutPassiveMotionFunc(MouseMotion);
	glutTimerFunc(16, TimerFunction, 1);

	std::cout << "메인 루프 시작" << std::endl;
	glutMainLoop();

	return 0;
}

GLuint loadTexture(const char* path)
{
	GLuint textureID;
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
	unsigned char* data = stbi_load(path, &width, &height, &channels, 0);

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

		std::cout << "텍스처 로드 성공: " << path << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;
	}
	else {
		std::cerr << "텍스처 로드 실패: " << path << std::endl;
		// 기본 텍스처 생성 (흰색)
		unsigned char whitePixel[3] = { 255, 255, 255 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, whitePixel);
	}

	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

void loadFaceTextures()
{
	if (faceTexturesLoaded) return;

	steveFaceTextureID = loadTexture("steve_face.png");
	alexFaceTextureID = loadTexture("alex_face.png");

	faceTexturesLoaded = true;
	std::cout << "얼굴 텍스처 로드 완료" << std::endl;
}

void initializeFaceBuffers()
{
	if (faceBuffersInitialized) return;

	// 2D 쿼드를 위한 버텍스 데이터 (NDC 좌표계)
	float quadVertices[] = {
		// 위치        // 텍스처 좌표
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	glGenVertexArrays(1, &faceVAO);
	glGenBuffers(1, &faceVBO);

	glBindVertexArray(faceVAO);

	glBindBuffer(GL_ARRAY_BUFFER, faceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	// 위치 속성
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	// 텍스처 좌표 속성
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);

	faceBuffersInitialized = true;
	std::cout << "얼굴 렌더링 버퍼 초기화 완료" << std::endl;
}

void renderCharacterFace(GLuint textureID, float x, float y, float size, int screenWidth, int screenHeight)
{
	if (!faceBuffersInitialized) {
		initializeFaceBuffers();
	}

	// 2D 렌더링을 위한 설정
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// 텍스처 바인딩
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// 2D 직교 투영 매트릭스 생성
	glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 model = glm::mat4(1.0f);

	// 얼굴 위치와 크기 설정
	model = glm::translate(model, glm::vec3(x, y, 0.0f));
	model = glm::scale(model, glm::vec3(size, size, 1.0f));

	// 셰이더 유니폼 설정
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "projection");
	GLint useTextureLoc = glGetUniformLocation(shaderProgramID, "useTexture");
	GLint lightingEnabledLoc = glGetUniformLocation(shaderProgramID, "lightingEnabled");
	GLint vColorLoc = glGetUniformLocation(shaderProgramID, "vColor");
	GLint textureLoc = glGetUniformLocation(shaderProgramID, "texture1");

	if (modelLoc != -1) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
	if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
	if (projLoc != -1) glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
	if (useTextureLoc != -1) glUniform1i(useTextureLoc, 1);
	if (lightingEnabledLoc != -1) glUniform1i(lightingEnabledLoc, 0);
	if (vColorLoc != -1) glUniform3f(vColorLoc, 1.0f, 1.0f, 1.0f);
	if (textureLoc != -1) glUniform1i(textureLoc, 0);

	// 쿼드 렌더링
	glBindVertexArray(faceVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	// 설정 복원
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void drawCharacterFaces(int winW, int winH)
{
	if (!splitScreenMode || currentGameState != GameState::FIRST_PERSON_MODE) return;
	if (!faceTexturesLoaded) return;

	const float faceSize = 40.0f; // 얼굴 크기 (픽셀)
	const float margin = 40.0f;   // 화면 가장자리로부터의 여백

	// Steve 얼굴 (왼쪽 화면의 좌상단)
	float steveX = margin;
	float steveY = winH - faceSize - margin; // OpenGL은 Y축이 아래쪽이 0이므로 조정

	// Alex 얼굴 (오른쪽 화면의 좌상단)
	float alexX = margin; // 오른쪽 뷰포트 내에서의 상대 위치
	float alexY = winH - faceSize - margin;

	// Steve 얼굴 렌더링 (왼쪽 화면)
	glViewport(0, 0, winW / 2, winH);
	renderCharacterFace(steveFaceTextureID, steveX, steveY, faceSize, winW / 2, winH);

	// Alex 얼굴 렌더링 (오른쪽 화면)
	glViewport(winW / 2, 0, winW - winW / 2, winH);
	renderCharacterFace(alexFaceTextureID, alexX, alexY, faceSize, winW - winW / 2, winH);

	// 뷰포트를 전체 화면으로 복원
	glViewport(0, 0, winW, winH);
}

void initializeGame() {
	// 조명 시스템 초기화
	lightManager.setupDefaultLighting();

	// Map 초기화 (셰이더 프로그램 생성 후)
	gameMap.initialize();
	std::cout << "맵 초기화 완료" << std::endl;

	// Steve 캐릭터 생성
	steve = new Steve::Character("steve.png");
	std::cout << "Steve 캐릭터 초기화 완료" << std::endl;

	// Alex 캐릭터 생성
	alex = new Alex::Character("alex.png");
	std::cout << "Alex 캐릭터 초기화 완료" << std::endl;

	// 눈 초기화
	snowSystem.clearAll();
	std::cout << "눈 초기화 완료" << std::endl;

	// 얼굴 렌더링 시스템 초기화
	initializeFaceBuffers();
	loadFaceTextures();

	// 카메라 모드에 따른 설정
	if (currentGameState == GameState::FIRST_PERSON_MODE) {
		splitScreenMode = true;
		std::cout << "\n=== 1인칭 모드 조작법 ===" << std::endl;
		std::cout << "V: 분할 화면 모드 토글" << std::endl;
		std::cout << "WASD: Steve 1인칭 시점 회전" << std::endl;
		std::cout << "화살표 키: Alex 1인칭 시점 회전" << std::endl;
		std::cout << "IJKL: 선택된 캐릭터 이동 (앞/왼/뒤/오른)" << std::endl;
		std::cout << "TAB: 캐릭터 선택 (Steve/Alex)" << std::endl;
	}
	else {
		splitScreenMode = false;
		std::cout << "\n=== 3인칭 모드 조작법 ===" << std::endl;
		std::cout << "V: 분할 화면 모드 토글" << std::endl;
		std::cout << "화살표 키: 카메라 회전" << std::endl;
		std::cout << "IJKL: 선택된 캐릭터 이동 (앞/왼/뒤/오른)" << std::endl;
		std::cout << "TAB: 캐릭터 선택 (Steve/Alex)" << std::endl;
	}
	std::cout << "E: Steve 눈덩이 발사" << std::endl;
	std::cout << "O: Alex 눈덩이 발사" << std::endl;
	std::cout << "X: 모든 눈 제거" << std::endl;
	std::cout << "ESC: 종료" << std::endl;
}

void Keyboard(unsigned char key, int x, int y)
{
	// 타이틀 화면에서 키 입력 처리
	if (currentGameState == GameState::TITLE_SCREEN) {
		GameState newState = titleScreen.handleKeyInput(key);
		if (newState != GameState::TITLE_SCREEN) {
			currentGameState = newState;
			initializeGame();
		}
		glutPostRedisplay();
		return;
	}

	// 게임 중 키 입력 처리
	input.pressKey(static_cast<int>(key));

	switch (key) {
	case 27: // ESC
	case 'q': case 'Q':
		exit(0);
		break;
	case 'v': case 'V':
		splitScreenMode = !splitScreenMode;
		std::cout << "분할 화면 모드: " << (splitScreenMode ? "ON" : "OFF") << std::endl;
		break;
	case 9: // TAB
		activeCharacter = (activeCharacter == STEVE) ? ALEX : STEVE;
		std::cout << "현재 선택된 캐릭터: " << (activeCharacter == STEVE ? "Steve" : "Alex") << std::endl;
		break;
	case 'e': case 'E': // Steve 눈덩이 차징 시작
		if (!steveCharging && steve && steve->armState < 2) {
			steveCharging = true;
			steveChargeStartTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
			steve->enterCharge(); // CHARGE 상태로 변경
			std::cout << "Steve 던지기 준비..." << std::endl;
		}
		break;
	case 'o': case 'O': // Alex 눈덩이 차징 시작
		if (!alexCharging && alex && alex->armState < 2) {
			alexCharging = true;
			alexChargeStartTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
			alex->enterCharge(); // CHARGE 상태로 변경
			std::cout << "Alex 던지기 준비..." << std::endl;
		}
		break;
	case 'x': case 'X':
		snowSystem.clearAll();
		std::cout << "모든 눈 제거됨" << std::endl;
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

// 키보드 릴리즈 함수 추가
void KeyboardUp(unsigned char key, int x, int y)
{
	// 타이틀 화면에서는 릴리즈 처리 안함
	if (currentGameState == GameState::TITLE_SCREEN) {
		return;
	}

	input.releaseKey(static_cast<int>(key));

	switch (key) {
	case 'e': case 'E': // Steve 눈덩이 발사
		if (steveCharging) {
			fireSteveSnowball();
		}
		break;
	case 'o': case 'O': // Alex 눈덩이 발사
		if (alexCharging) {
			fireAlexSnowball();
		}
		break;
	}

	glutPostRedisplay();
}

void fireSteveSnowball()
{
	if (!steve) return;

	float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	float chargeTime = currentTime - steveChargeStartTime;
	float chargeRatio = std::min(chargeTime / maxChargeTime, 1.0f);
	float speed = minSpeed + (maxSpeed - minSpeed) * chargeRatio;

	steve->enterThrow();
	speed *= steve->throwingSpeed; // Steve의 투사체 속도 적용

	glm::vec3 shootDirection = steveCamera.getFront();
	glm::vec3 startPos = steve->pos + shootDirection * 1.0f + glm::vec3(0, 1, 0) * 0.5f;
	glm::vec3 direction = shootDirection + glm::vec3(0, 1, 0) * 0.3f;

	Snowball newSnowball(startPos, direction, speed, 0.15f);
	snowballs.push_back(newSnowball);

	std::cout << "Steve 눈덩이 발사! 차징 시간: " << chargeTime << "초, 속도: " << speed
		<< " (현재 " << snowballs.size() << "개)" << std::endl;

	steveCharging = false;
}

void fireAlexSnowball()
{
	if (!alex) return;

	float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	float chargeTime = currentTime - alexChargeStartTime;
	float chargeRatio = std::min(chargeTime / maxChargeTime, 1.0f);
	float speed = minSpeed + (maxSpeed - minSpeed) * chargeRatio;

	alex->enterThrow();
	speed *= alex->throwingSpeed; // Alex의 투사체 속도 적용

	glm::vec3 shootDirection = alexCamera.getFront();
	glm::vec3 startPos = alex->pos + shootDirection * 1.0f + glm::vec3(0, 1, 0) * 0.5f;
	glm::vec3 direction = shootDirection + glm::vec3(0, 1, 0) * 0.3f;

	Snowball newSnowball(startPos, direction, speed, 0.15f);
	snowballs.push_back(newSnowball);

	std::cout << "Alex 눈덩이 발사! 차징 시간: " << chargeTime << "초, 속도: " << speed
		<< " (현재 " << snowballs.size() << "개)" << std::endl;

	alexCharging = false;
}

// 눈덩이와 캐릭터의 충돌 감지 함수
bool checkSnowballCharacterCollision(const Snowball& snowball, const glm::vec3& characterPos, const glm::vec3& boundingBoxSize)
{
	if (!snowball.getIsActive()) return false;

	glm::vec3 snowballPos = snowball.getPosition();
	float snowballRadius = snowball.getRadius();

	// 캐릭터의 바운딩 박스 계산 (중심점 기준)
	glm::vec3 characterMin = characterPos - boundingBoxSize / 2.0f;
	glm::vec3 characterMax = characterPos + boundingBoxSize / 2.0f;

	// 구와 AABB(축 정렬 바운딩 박스) 충돌 검사
	// 가장 가까운 점 찾기
	float closestX = std::max(characterMin.x, std::min(snowballPos.x, characterMax.x));
	float closestY = std::max(characterMin.y, std::min(snowballPos.y, characterMax.y));
	float closestZ = std::max(characterMin.z, std::min(snowballPos.z, characterMax.z));

	glm::vec3 closestPoint(closestX, closestY, closestZ);

	// 거리 계산
	glm::vec3 distance = snowballPos - closestPoint;
	float distanceSquared = glm::dot(distance, distance);

	// 충돌 검사
	return distanceSquared <= (snowballRadius * snowballRadius);
}

// 모든 눈덩이와 캐릭터 간의 충돌 검사
void checkAllSnowballCollisions()
{
	for (const auto& snowball : snowballs) {
		if (!snowball.getIsActive()) continue;

		// Steve와의 충돌 검사
		if (steve && checkSnowballCharacterCollision(snowball, steve->pos, steve->boundingBoxSize)) {
			std::cout << "\n=== 게임 종료 ===" << std::endl;
			std::cout << "Steve가 눈덩이에 맞았습니다!" << std::endl;
			std::cout << "눈덩이 위치: (" << snowball.getPosition().x << ", " << snowball.getPosition().y << ", " << snowball.getPosition().z << ")" << std::endl;
			std::cout << "Steve 위치: (" << steve->pos.x << ", " << steve->pos.y << ", " << steve->pos.z << ")" << std::endl;

			// 정리 작업
			if (steve) {
				delete steve;
				steve = nullptr;
			}
			if (alex) {
				delete alex;
				alex = nullptr;
			}

			exit(0);
		}

		// Alex와의 충돌 검사
		if (alex && checkSnowballCharacterCollision(snowball, alex->pos, alex->boundingBoxSize)) {
			std::cout << "\n=== 게임 종료 ===" << std::endl;
			std::cout << "Alex가 눈덩이에 맞았습니다!" << std::endl;
			std::cout << "눈덩이 위치: (" << snowball.getPosition().x << ", " << snowball.getPosition().y << ", " << snowball.getPosition().z << ")" << std::endl;
			std::cout << "Alex 위치: (" << alex->pos.x << ", " << alex->pos.y << ", " << alex->pos.z << ")" << std::endl;

			// 정리 작업
			if (steve) {
				delete steve;
				steve = nullptr;
			}
			if (alex) {
				delete alex;
				alex = nullptr;
			}

			exit(0);
		}
	}
}

void SpecialKeyboard(int key, int x, int y)
{
	// 타이틀 화면에서는 특수키 처리 안함
	if (currentGameState == GameState::TITLE_SCREEN) {
		return;
	}

	input.pressSpecial(key);

	float deltaTime = 0.016f;
	if (!splitScreenMode || currentGameState == GameState::THIRD_PERSON_MODE) {
		camera.processSpecialKeyboard(key);
		cameraPos = camera.position;
		cameraFront = camera.front;
		cameraUp = camera.up;
	}
	// 분할 모드면 화살표 입력은 Timer에서 input.isSpecialDown으로 처리 가능
	glutPostRedisplay();
}

void SpecialKeyboardUp(int key, int x, int y)
{
	// 타이틀 화면에서는 특수키 릴리즈 처리 안함
	if (currentGameState == GameState::TITLE_SCREEN) {
		return;
	}

	input.releaseSpecial(key);
	glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y)
{
	// 타이틀 화면에서는 마우스 처리 안함
	if (currentGameState == GameState::TITLE_SCREEN) {
		return;
	}

	// 마우스 휠로 줌 인/아웃
	if (button == 3) { // 휠 위로
		if (splitScreenMode) {
			steveCamera.processMouseScroll(1);
			alexCamera.processMouseScroll(1);
		}
		else {
			camera.processMouseScroll(1);
			cameraPos = camera.position;
		}
		glutPostRedisplay();
	}
	else if (button == 4) { // 휠 아래로
		if (splitScreenMode) {
			steveCamera.processMouseScroll(-1);
			alexCamera.processMouseScroll(-1);
		}
		else {
			camera.processMouseScroll(-1);
			cameraPos = camera.position;
		}
		glutPostRedisplay();
	}
}

void MouseMotion(int x, int y)
{
	// 타이틀 화면이나 게임 중에도 마우스 움직임으로 카메라 회전 (현재는 비활성화)
}

void make_shaderProgram()
{

	// 먼저 파일에서 읽기 시도
	vertexSource = filetobuf("vertex.glsl");
	fragmentSource = filetobuf("fragment.glsl");

	// 버텍스 셰이더
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
	glCompileShader(vertexShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}

	// 프래그먼트 셰이더
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}

	// 셰이더 프로그램 생성
	shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);
	glLinkProgram(shaderProgramID);

	// 링크 상태 확인
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderProgramID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program 링크 실패\n" << errorLog << std::endl;
		return;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(shaderProgramID);
	glUniform1i(glGetUniformLocation(shaderProgramID, "texture1"), 0);
	std::cout << "셰이더 프로그램이 성공적으로 생성되었습니다." << std::endl;
}

GLvoid drawScene()
{
	if (shaderProgramID == 0) return;

	glUseProgram(shaderProgramID);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 타이틀 화면 렌더링
	if (currentGameState == GameState::TITLE_SCREEN) {
		titleScreen.render(shaderProgramID);
		glutSwapBuffers();
		return;
	}

	// 게임 화면 렌더링
	// 현재 윈도우 크기 사용 (사용자가 창 크기 조절해도 반영)
	int winW = glutGet(GLUT_WINDOW_WIDTH);
	int winH = glutGet(GLUT_WINDOW_HEIGHT);
	if (winW <= 0) winW = WinX;
	if (winH <= 0) winH = WinY;

	if (splitScreenMode && currentGameState == GameState::FIRST_PERSON_MODE) {
		// 분할 화면 렌더링 (1인칭 모드)
		// 카메라 위치를 캐릭터 기준으로 업데이트
		if (steve) steveCamera.updateFromCharacterPosition(steve->pos);
		if (alex) alexCamera.updateFromCharacterPosition(alex->pos);

		// 왼쪽: Steve
		glViewport(0, 0, winW / 2, winH);
		glm::mat4 steveView = steveCamera.getViewMatrix();
		glm::mat4 steveProj = steveCamera.getProjectionMatrix(winW / 2, winH);
		renderWorld(steveView, steveProj);

		// 깊이 버퍼만 지워서 오른쪽 렌더가 겹치지 않게 함
		glClear(GL_DEPTH_BUFFER_BIT);

		// 오른쪽: Alex
		glViewport(winW / 2, 0, winW - winW / 2, winH);
		glm::mat4 alexView = alexCamera.getViewMatrix();
		glm::mat4 alexProj = alexCamera.getProjectionMatrix(winW - winW / 2, winH);
		renderWorld(alexView, alexProj);

		// 캐릭터 얼굴 오버레이 렌더링
		drawCharacterFaces(winW, winH);
	}
	else {
		// 단일 화면(기본 3인칭)
		glViewport(0, 0, winW, winH);
		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 proj = camera.getProjectionMatrix(winW, winH);
		renderWorld(view, proj);
	}

	glutSwapBuffers();
}

GLvoid drawSplitScreen() {
	// 이제 drawScene이 분할/단일을 모두 다루므로 이 함수는 호출 경로 필요시 비워두거나 drawScene에 통합된 상태 유지.
	// 기존 코드와 중복되지 않도록 drawScene이 사용되도록 유지하세요.
	// (함수 정의는 남겨두되 내부는 비워두어도 무방)
}

GLvoid renderWorld(const glm::mat4& view, const glm::mat4& projection)
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);

	unsigned int modelLoc = glGetUniformLocation(shaderProgramID, "model");
	unsigned int viewLoc = glGetUniformLocation(shaderProgramID, "view");
	unsigned int projLoc = glGetUniformLocation(shaderProgramID, "projection");

	GLint lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
	GLint lightColorLoc = glGetUniformLocation(shaderProgramID, "lightColor");
	GLint viewPosLoc = glGetUniformLocation(shaderProgramID, "viewPos");
	GLint lightingEnabledLoc = glGetUniformLocation(shaderProgramID, "lightingEnabled");
	GLint useTextureLoc = glGetUniformLocation(shaderProgramID, "useTexture");

	if (lightingEnabledLoc != -1) glUniform1i(lightingEnabledLoc, 1);

	if (lightPosLoc != -1) {
		glm::vec3 lightPos(5.0f, 8.0f, 7.5f);
		glUniform3fv(lightPosLoc, 1, &lightPos[0]);
	}
	if (lightColorLoc != -1) {
		glm::vec3 lightColor(1.5f, 1.5f, 1.5f);
		glUniform3fv(lightColorLoc, 1, &lightColor[0]);
	}

	if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
	if (projLoc != -1) glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

	if (useTextureLoc != -1) glUniform1i(useTextureLoc, 0);
	gameMap.render();

	if (useTextureLoc != -1) glUniform1i(useTextureLoc, 1);
	snowSystem.render(shaderProgramID);

	if (useTextureLoc != -1) glUniform1i(useTextureLoc, 1);

	// Steve / Alex: 클래스 내부 pos를 사용해서 그리기
	if (steve) {
		// steve->draw 내부에서 pos 기반으로 변환하도록 구현되어 있으므로 model transform 전달 불필요
		steve->draw(modelLoc);
	}
	if (alex) {
		alex->draw(modelLoc);
	}

	if (useTextureLoc != -1) glUniform1i(useTextureLoc, 0);
	for (const auto& snowball : snowballs) {
		snowball.render(shaderProgramID, glm::vec3(1.0f));
	}
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

// per-frame 이동 업데이트 (KeyManager 폴링)
void updateStevePosition(float deltaTime)
{
	steve->moveDir = glm::vec2(0.0f, 0.0f);

	if (splitScreenMode) {
		if (input.isKeyDown('w') || input.isKeyDown('W')) steve->moveDir.y += 1.0f;
		if (input.isKeyDown('s') || input.isKeyDown('S')) steve->moveDir.y += -1.0f;
		if (input.isKeyDown('a') || input.isKeyDown('A')) steve->moveDir.x += 1.0f;
		if (input.isKeyDown('d') || input.isKeyDown('D')) steve->moveDir.x += -1.0f;
	}
	else {
		if (input.isKeyDown('w') || input.isKeyDown('W')) steve->moveDir.x += 1.0f;
		if (input.isKeyDown('s') || input.isKeyDown('S')) steve->moveDir.x += -1.0f;
		if (input.isKeyDown('a') || input.isKeyDown('A')) steve->moveDir.y += -1.0f;
		if (input.isKeyDown('d') || input.isKeyDown('D')) steve->moveDir.y += 1.0f;
	}
}

void updateAlexPosition(float deltaTime)
{
	alex->moveDir = glm::vec2(0.0f, 0.0f);
	if (splitScreenMode) {
		if (input.isKeyDown('i') || input.isKeyDown('I')) alex->moveDir.y += -1.0f;
		if (input.isKeyDown('k') || input.isKeyDown('K')) alex->moveDir.y += 1.0f;
		if (input.isKeyDown('j') || input.isKeyDown('J')) alex->moveDir.x += -1.0f;
		if (input.isKeyDown('l') || input.isKeyDown('L')) alex->moveDir.x += 1.0f;
	}
	else {
		if (input.isKeyDown('i') || input.isKeyDown('I')) alex->moveDir.x += 1.0f;
		if (input.isKeyDown('k') || input.isKeyDown('K')) alex->moveDir.x += -1.0f;
		if (input.isKeyDown('j') || input.isKeyDown('J')) alex->moveDir.y += -1.0f;
		if (input.isKeyDown('l') || input.isKeyDown('L')) alex->moveDir.y += 1.0f;
	}
}

// 이동 상태 확인은 그대로 유지 (내부 상태가 바뀌면 Character.changeState로 처리됨)
void checkSteveMoving()
{
	bool moving = input.isKeyDown('w') || input.isKeyDown('W') ||
		input.isKeyDown('s') || input.isKeyDown('S') ||
		input.isKeyDown('a') || input.isKeyDown('A') ||
		input.isKeyDown('d') || input.isKeyDown('D');
	if (!moving && steve) steve->changeState(1, 0); // leg idle
}


void checkAlexMoving()
{
	bool moving = input.isKeyDown('i') || input.isKeyDown('I') ||
		input.isKeyDown('j') || input.isKeyDown('J') ||
		input.isKeyDown('k') || input.isKeyDown('K') ||
		input.isKeyDown('l') || input.isKeyDown('L');
	if (!moving && alex) alex->changeState(1, 0); // leg idle
}

// TimerFunction: moveDir 설정 후 Character::update(map) 호출 — 클래스 내부 충돌 로직 수행
void TimerFunction(int value)
{
	// 타이틀 화면에서는 게임 로직 업데이트 안함
	if (currentGameState == GameState::TITLE_SCREEN) {
		glutPostRedisplay();
		glutTimerFunc(16, TimerFunction, 1);
		return;
	}

	float deltaTime = 0.016f;

	// 눈덩이 업데이트
	for (auto& sb : snowballs) sb.update(deltaTime, snowSystem, gameMap);
	snowballs.erase(std::remove_if(snowballs.begin(), snowballs.end(),
		[](const Snowball& s) { return !s.getIsActive(); }), snowballs.end());

	// 눈덩이와 캐릭터 간의 충돌 검사
	checkAllSnowballCollisions();

	// 키 기반 moveDir 설정 (KeyManager 폴링)
	updateStevePosition(deltaTime);
	updateAlexPosition(deltaTime);

	// Character에서 충돌검사 포함한 이동 수행
	if (steve) steve->update(gameMap, snowSystem);
	if (alex) alex->update(gameMap, snowSystem);

	// 이동 상태 점검(정지시 IDLE)
	checkSteveMoving();
	checkAlexMoving();

	// 카메라 위치 동기화
	cameraPos = camera.position;
	cameraFront = camera.front;
	cameraUp = camera.up;

	// 애니메이션(내부) 업데이트 이미 Character::update에서 처리됨

	glutPostRedisplay();
	glutTimerFunc(16, TimerFunction, 1);
}