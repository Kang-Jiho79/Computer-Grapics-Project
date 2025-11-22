#include "Map.h"
#include <gl/glew.h>

Map::Map() : frontGround(MAP_WIDTH, 5, BLOCK_SIZE), backGround(MAP_WIDTH, 5, BLOCK_SIZE)
{
    // 생성자에서는 기본 설정만 하고 OpenGL 초기화는 나중에
}

void Map::initialize()
{
    // 앞쪽 Ground 초기화 (z: 0~4)
    frontGround.initializeBlocks(0.0f, 0.0f, 0.0f, BLOCK_SIZE);
    
    // 뒤쪽 Ground 초기화 (z: 10~14)
    backGround.initializeBlocks(0.0f, 0.0f, 10.0f * BLOCK_SIZE, BLOCK_SIZE);
    
    // 둘러싸는 벽 초기화
    surroundingWall.clear();
    
    // 벽 높이 설정 (5칸)
    const int WALL_HEIGHT = 5;
    
    // 맵 경계 벽 생성 - 5층으로 쌓기 (Ground 위에)
    for (int height = 0; height < WALL_HEIGHT; ++height) {
        float wallY = (height + 1) * BLOCK_SIZE; // Ground 위에서 시작하여 위로 쌓기
        
        // 앞쪽 벽 (z = -1)
        surroundingWall.createHorizontalWall(-BLOCK_SIZE, wallY, -BLOCK_SIZE, MAP_WIDTH + 2, BLOCK_SIZE);
        
        // 뒤쪽 벽 (z = 15)
        surroundingWall.createHorizontalWall(-BLOCK_SIZE, wallY, MAP_DEPTH * BLOCK_SIZE, MAP_WIDTH + 2, BLOCK_SIZE);
        
        // 왼쪽 벽 (x = -1)
        surroundingWall.createVerticalWall(-BLOCK_SIZE, wallY, 0.0f, MAP_DEPTH, BLOCK_SIZE);
        
        // 오른쪽 벽 (x = 10)
        surroundingWall.createVerticalWall(MAP_WIDTH * BLOCK_SIZE, wallY, 0.0f, MAP_DEPTH, BLOCK_SIZE);
    }
}

void Map::render() const
{
    // Ground 렌더링
    frontGround.render();
    backGround.render();
    
    // Wall 렌더링  
    surroundingWall.render();
}
