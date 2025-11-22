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
#include "Map.h"      // 맵 헤더
#include "Light.h"    // 조명 헤더 추가
//#include "stb_image.h"  // stb_image 헤더 추가
#include "Steve.h"      // Steve 헤더 추가
#include "Alex.h"       // Alex 헤더 추가
#include "Camera.h"     // Camera 헤더 추가

#define WinX 1280
#define WinY 720

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

// 전역 객체들
static Camera camera;  // 카메라 객체
static LightManager lightManager;
static Map gameMap;
static Steve::Character* steve = nullptr;
static Alex::Character* alex = nullptr;

// 캐릭터 위치 관리
static glm::vec3 stevePosition = glm::vec3(3.0f, 1.5f, 5.0f);
static glm::vec3 alexPosition = glm::vec3(7.0f, 1.5f, 5.0f);

// 기타 전역 변수들
bool cameraLightMode = true;  // 카메라 따라다니는 조명 모드
enum CharacterSelection { STEVE, ALEX };
static CharacterSelection activeCharacter = STEVE;

// 마우스 상태
bool firstMouse = true;
float lastX = WinX / 2.0f;
float lastY = WinY / 2.0f;

// 함수 선언
void Mouse(int button, int state, int x, int y);
void MouseMotion(int x, int y);
void Keyboard(unsigned char key, int x, int y);
void SpecialKeyboard(int key, int x, int y);
void TimerFunction(int value);

void make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);

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
	glutCreateWindow("Map, Steve & Alex - WASD: 카메라 이동, 화살표: 카메라 회전, IJKL: 캐릭터 이동");

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

	std::cout << "\n=== 조작법 ===" << std::endl;
	std::cout << "WASD: 카메라 이동" << std::endl;
	std::cout << "화살표 키: 카메라 회전" << std::endl;
	std::cout << "E/C: 위/아래 이동" << std::endl;
	std::cout << "TAB: 캐릭터 선택 (Steve/Alex)" << std::endl;
	std::cout << "IJKL: 선택된 캐릭터 이동 (앞/왼/뒤/오른)" << std::endl;
	std::cout << "U/O: 선택된 캐릭터 위/아래 이동" << std::endl;
	std::cout << "M: 선택된 캐릭터 위치 리셋" << std::endl;
	std::cout << "N: 모든 캐릭터 위치 리셋" << std::endl;
	std::cout << "T: 조명 모드 토글" << std::endl;
	std::cout << "R: 카메라 리셋" << std::endl;
	std::cout << "ESC: 종료" << std::endl;
	std::cout << "\n현재 선택된 캐릭터: Steve" << std::endl;

	//--- 콜백 등록
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard);
	glutMouseFunc(Mouse);
	glutPassiveMotionFunc(MouseMotion);
	glutTimerFunc(16, TimerFunction, 1);
	
	std::cout << "메인 루프 시작" << std::endl;
	glutMainLoop();
	
	return 0;
}

void Keyboard(unsigned char key, int x, int y)
{
	float deltaTime = 0.016f; // 16ms (60 FPS)

	switch (key) {
	case 27: // ESC 키
	case 'q':
	case 'Q':
		exit(0);
		break;
	
	// 카메라 이동 (Camera 클래스에서 처리)
	case 'w': case 'W':
	case 's': case 'S':
	case 'a': case 'A':
	case 'd': case 'D':
	case 'e': case 'E':
	case 'c': case 'C':
	case '+': case '=':
	case '-': case '_':
	case 'r': case 'R':
		camera.processKeyboard(key, deltaTime);
		// cameraPos 전역 변수도 업데이트 (다른 파일에서 사용하기 위해)
		cameraPos = camera.position;
		cameraFront = camera.front;
		cameraUp = camera.up;
		break;

	// 캐릭터 선택 토글
	case 9: // TAB 키
		activeCharacter = (activeCharacter == STEVE) ? ALEX : STEVE;
		std::cout << "현재 선택된 캐릭터: " << (activeCharacter == STEVE ? "Steve" : "Alex") << std::endl;
		break;

	// 선택된 캐릭터 이동
	case 'i': case 'I':
		if (activeCharacter == STEVE) {
			stevePosition += glm::vec3(0.0f, 0.0f, -0.1f);
			std::cout << "Steve 위치: " << stevePosition.x << ", " << stevePosition.y << ", " << stevePosition.z << std::endl;
		} else {
			alexPosition += glm::vec3(0.0f, 0.0f, -0.1f);
			std::cout << "Alex 위치: " << alexPosition.x << ", " << alexPosition.y << ", " << alexPosition.z << std::endl;
		}
		break;
	case 'k': case 'K':
		if (activeCharacter == STEVE) {
			stevePosition += glm::vec3(0.0f, 0.0f, 0.1f);
			std::cout << "Steve 위치: " << stevePosition.x << ", " << stevePosition.y << ", " << stevePosition.z << std::endl;
		} else {
			alexPosition += glm::vec3(0.0f, 0.0f, 0.1f);
			std::cout << "Alex 위치: " << alexPosition.x << ", " << alexPosition.y << ", " << alexPosition.z << std::endl;
		}
		break;
	case 'j': case 'J':
		if (activeCharacter == STEVE) {
			stevePosition += glm::vec3(-0.1f, 0.0f, 0.0f);
			std::cout << "Steve 위치: " << stevePosition.x << ", " << stevePosition.y << ", " << stevePosition.z << std::endl;
		} else {
			alexPosition += glm::vec3(-0.1f, 0.0f, 0.0f);
			std::cout << "Alex 위치: " << alexPosition.x << ", " << alexPosition.y << ", " << alexPosition.z << std::endl;
		}
		break;
	case 'l': case 'L':
		if (activeCharacter == STEVE) {
			stevePosition += glm::vec3(0.1f, 0.0f, 0.0f);
			std::cout << "Steve 위치: " << stevePosition.x << ", " << stevePosition.y << ", " << stevePosition.z << std::endl;
		} else {
			alexPosition += glm::vec3(0.1f, 0.0f, 0.0f);
			std::cout << "Alex 위치: " << alexPosition.x << ", " << alexPosition.y << ", " << alexPosition.z << std::endl;
		}
		break;
	case 'u': case 'U':
		if (activeCharacter == STEVE) {
			stevePosition += glm::vec3(0.0f, 0.1f, 0.0f);
			std::cout << "Steve 위치: " << stevePosition.x << ", " << stevePosition.y << ", " << stevePosition.z << std::endl;
		} else {
			alexPosition += glm::vec3(0.0f, 0.1f, 0.0f);
			std::cout << "Alex 위치: " << alexPosition.x << ", " << alexPosition.y << ", " << alexPosition.z << std::endl;
		}
		break;
	case 'o': case 'O':
		if (activeCharacter == STEVE) {
			stevePosition += glm::vec3(0.0f, -0.1f, 0.0f);
			std::cout << "Steve 위치: " << stevePosition.x << ", " << stevePosition.y << ", " << stevePosition.z << std::endl;
		} else {
			alexPosition += glm::vec3(0.0f, -0.1f, 0.0f);
			std::cout << "Alex 위치: " << alexPosition.x << ", " << alexPosition.y << ", " << alexPosition.z << std::endl;
		}
		break;
	case 'm': case 'M':
		// 선택된 캐릭터만 리セット
		if (activeCharacter == STEVE) {
			stevePosition = glm::vec3(3.0f, 1.5f, 5.0f);
			std::cout << "Steve 위치 리셋: " << stevePosition.x << ", " << stevePosition.y << ", " << stevePosition.z << std::endl;
		} else {
			alexPosition = glm::vec3(7.0f, 1.5f, 5.0f);
			std::cout << "Alex 위치 리셋: " << alexPosition.x << ", " << alexPosition.y << ", " << alexPosition.z << std::endl;
		}
		break;
	case 'n': case 'N':
		// 모든 캐릭터 위치 리셋
		stevePosition = glm::vec3(3.0f, 1.5f, 5.0f);
		alexPosition = glm::vec3(7.0f, 1.5f, 5.0f);
		std::cout << "모든 캐릭터 위치 리셋" << std::endl;
		std::cout << "Steve 위치: " << stevePosition.x << ", " << stevePosition.y << ", " << stevePosition.z << std::endl;
		std::cout << "Alex 위치: " << alexPosition.x << ", " << alexPosition.y << ", " << alexPosition.z << std::endl;
		break;
	case 't': case 'T':
		// 조명 모드 토글
		cameraLightMode = !cameraLightMode;
		if (cameraLightMode) {
			lightManager.setupDefaultLighting();
			std::cout << "카메라 따라다니는 조명 모드" << std::endl;
		} else {
			lightManager.setupSceneStaticLight();
			std::cout << "정적 조명 모드" << std::endl;
		}
		break;
	}
	
	glutPostRedisplay();
}

void SpecialKeyboard(int key, int x, int y)
{
	camera.processSpecialKeyboard(key);
	// cameraPos 전역 변수도 업데이트
	cameraPos = camera.position;
	cameraFront = camera.front;
	cameraUp = camera.up;
	glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y) 
{
	// 마우스 휠로 줌 인/아웃
	if (button == 3) { // 휠 위로
		camera.processMouseScroll(1);
		cameraPos = camera.position;
		glutPostRedisplay();
	}
	else if (button == 4) { // 휠 아래로
		camera.processMouseScroll(-1);
		cameraPos = camera.position;
		glutPostRedisplay();
	}
}

void MouseMotion(int x, int y)
{
	// 마우스 움직임으로 카메라 회전 (선택적 기능)
	// 현재는 비활성화되어 있음
}

void make_shaderProgram()
{
	// 텍스처 지원을 위한 버텍스 셰이더
	const char* vertexShaderSource = R"(
		#version 330 core
		layout (location = 0) in vec3 vPos;
		layout (location = 1) in vec3 vNormal;
		layout (location = 2) in vec2 aTexCoord;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;
		uniform mat3 normalMatrix;

		out vec4 FragPos;
		out vec3 Normal;
		out vec2 TexCoord;

		void main()
		{
			FragPos = model * vec4(vPos, 1.0);
			Normal = normalMatrix * vNormal;
			TexCoord = aTexCoord;
			
			gl_Position = projection * view * FragPos;
		}
	)";

	// 텍스처 지원을 위한 프래그먼트 셰이더
	const char* fragmentShaderSource = R"(
		#version 330 core
		out vec4 FragColor;

		in vec4 FragPos;
		in vec3 Normal;
		in vec2 TexCoord;

		uniform vec3 lightPos;
		uniform vec3 lightColor;
		uniform vec3 viewPos;
		uniform vec3 vColor;
		uniform bool lightingEnabled;
		uniform bool useTexture;
		uniform sampler2D texture1;

		void main()
		{
			vec3 objectColor;
			
			if (useTexture) {
				objectColor = texture(texture1, TexCoord).rgb;
			} else {
				objectColor = vColor;
			}

			if (!lightingEnabled)
			{
				FragColor = vec4(objectColor, 1.0f);
				return;
			}

			float ambientStrength = 0.5f;
			vec3 ambient = ambientStrength * lightColor;

			vec3 normalVector = normalize(Normal);
			vec3 lightDir = normalize(lightPos - FragPos.xyz);
			float diffuseLight = max(dot(normalVector, lightDir), 0.0);
			vec3 diffuse = diffuseLight * lightColor;

			int shininess = 128;
			vec3 viewDir = normalize(viewPos - FragPos.xyz);
			vec3 reflectDir = reflect(-lightDir, normalVector);
			float specularLight = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
			vec3 specular = specularLight * lightColor;

			vec3 result = (ambient + diffuse + specular) * objectColor;

			FragColor = vec4(result, 1.0f);
		}
	)";

	// 먼저 파일에서 읽기 시도
	vertexSource = filetobuf("vertex.glsl");
	fragmentSource = filetobuf("fragment.glsl");

	// 파일이 없으면 하드코딩된 셰이더 사용
	if (!vertexSource) {
		std::cout << "vertex.glsl 파일이 없습니다. 기본 셰이더를 사용합니다." << std::endl;
		vertexSource = (char*)vertexShaderSource;
	}
	if (!fragmentSource) {
		std::cout << "fragment.glsl 파일이 없습니다. 기본 셰이더를 사용합니다." << std::endl;
		fragmentSource = (char*)fragmentShaderSource;
	}

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
	glUseProgram(shaderProgramID);

	// 배경색을 회색으로 변경하여 조명 효과를 확인할 수 있게 함
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  // 더 어두운 배경
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);

	// 유니폼 위치 가져오기
	unsigned int modelLoc = glGetUniformLocation(shaderProgramID, "model");
	unsigned int viewLoc = glGetUniformLocation(shaderProgramID, "view");
	unsigned int projLoc = glGetUniformLocation(shaderProgramID, "projection");
	unsigned int normalMatrixLoc = glGetUniformLocation(shaderProgramID, "normalMatrix");

	// 조명 설정
	GLint lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
	GLint lightColorLoc = glGetUniformLocation(shaderProgramID, "lightColor");
	GLint viewPosLoc = glGetUniformLocation(shaderProgramID, "viewPos");
	GLint lightingEnabledLoc = glGetUniformLocation(shaderProgramID, "lightingEnabled");
	GLint useTextureLoc = glGetUniformLocation(shaderProgramID, "useTexture");

	// 조명 활성화
	if (lightingEnabledLoc != -1) {
		glUniform1i(lightingEnabledLoc, 1); // true
	}

	// 조명 위치와 색상 설정
	if (lightPosLoc != -1) {
		glm::vec3 lightPos(5.0f, 8.0f, 7.5f);
		glUniform3fv(lightPosLoc, 1, &lightPos[0]);
	}

	if (lightColorLoc != -1) {
		glm::vec3 lightColor(1.5f, 1.5f, 1.5f);
		glUniform3fv(lightColorLoc, 1, &lightColor[0]);
	}

	// 카메라 위치 설정
	if (viewPosLoc != -1) {
		glUniform3fv(viewPosLoc, 1, &camera.position[0]);
	}

	// 카메라/투영 행렬 설정 (Camera 클래스 사용)
	glm::mat4 view = camera.getViewMatrix();
	glm::mat4 proj = camera.getProjectionMatrix(WinX, WinY);

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &proj[0][0]);

	// 조명 시스템 적용
	if (cameraLightMode) {
		lightManager.setupCameraFollowLight(camera.position);
	}
	lightManager.applyLighting(shaderProgramID, camera.position);

	// 맵 렌더링 (텍스처 사용하지 않음)
	if (useTextureLoc != -1) {
		glUniform1i(useTextureLoc, 0); // false
	}
	gameMap.render();

	// 캐릭터 렌더링 (텍스처 사용)
	if (useTextureLoc != -1) {
		glUniform1i(useTextureLoc, 1); // true
	}

	// Steve 캐릭터 렌더링
	if (steve) {
		glm::mat4 steveTransform = glm::translate(glm::mat4(1.0f), stevePosition);
		steve->draw(modelLoc, steveTransform);
	}

	// Alex 캐릭터 렌더링
	if (alex) {
		glm::mat4 alexTransform = glm::translate(glm::mat4(1.0f), alexPosition);
		alex->draw(modelLoc, alexTransform);
	}

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

void TimerFunction(int value)
{
	glutPostRedisplay();
	glutTimerFunc(16, TimerFunction, 1);
}