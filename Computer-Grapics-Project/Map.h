#pragma once
#include "Ground.h"
#include "Wall.h"

// 맵 설정 상수 (쉽게 변경 가능)
constexpr int MAP_WIDTH = 10;
constexpr int MAP_DEPTH = 15;
constexpr float BLOCK_SIZE = 1.0f;  // 블록 크기 (나중에 쉽게 변경 가능)

class Map
{
private:
    Ground frontGround;     // 앞쪽 ground (10x5)
    Ground backGround;      // 뒤쪽 ground (10x5)
    Wall surroundingWall;   // 맵을 둘러싸는 벽
    
public:
    // 생성자
    Map();
    
    // 초기화 함수
    void initialize();
    
    // 접근자 함수
    Ground& getFrontGround() { return frontGround; }
    Ground& getBackGround() { return backGround; }
    Wall& getWall() { return surroundingWall; }
    
    const Ground& getFrontGround() const { return frontGround; }
    const Ground& getBackGround() const { return backGround; }
    const Wall& getWall() const { return surroundingWall; }
    
    // 렌더링 함수
    void render() const;
};

