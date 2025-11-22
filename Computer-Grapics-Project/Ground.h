#pragma once
#include "Block.h"
#include <vector>

class Ground
{
private:
    std::vector<std::vector<Block>> blocks;  // 2D 블록 배열
    int width, depth;  // Ground의 크기

public:
    // 생성자
    Ground(int width, int depth, float blockSize = 1.0f);
    
    // 접근자 함수
    int getWidth() const;
    int getDepth() const;
    Block& getBlock(int x, int z);
    const Block& getBlock(int x, int z) const;
    
    // 초기화 함수
    void initializeBlocks(float startX, float startY, float startZ, float blockSize);
    
    // 렌더링 함수
    void render() const;
};


