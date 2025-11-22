#pragma once
#include "Block.h"
#include <vector>

class Wall
{
private:
    std::vector<Block> blocks;  // 벽을 구성하는 블록들
    
public:
    // 생성자
    Wall();
    
    // 블록 추가 함수
    void addBlock(const Block& block);
    void addBlock(float x, float y, float z, float size = 1.0f);
    
    // 벽 생성 함수들
    void createHorizontalWall(float startX, float y, float z, int length, float blockSize);
    void createVerticalWall(float x, float y, float startZ, int length, float blockSize);
    
    // 접근자 함수
    size_t getBlockCount() const { return blocks.size(); }
    const Block& getBlock(size_t index) const { return blocks[index]; }
    
    // 렌더링 함수
    void render() const;
    
    // 초기화 함수
    void clear() { blocks.clear(); }
};

