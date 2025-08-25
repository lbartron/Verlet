#pragma once
#include <vector>
#include <SFML/System/Vector2.hpp>

#include "MovingObject.hpp"

class GridCollision{
private:
struct Cell {
    std::vector<uint32_t> indices;
    //static constexpr size_t MAX_PARTICLES_PER_CELL = 16;
    //uint32_t indices[8];
    bool dirty = false;
};
float coefficientOfRestitution = 0.7f;
int CELL_SIZE = 0;
int GRID_WIDTH = 0;
int GRID_HEIGHT = 0;
float WORLD_WIDTH = 0;
float WORLD_HEIGHT = 0;

std::vector<Cell> grid;
std::vector<int> dirtyCells;

inline int cellIndex(int x, int y){
    return y * GRID_WIDTH + x;
}

public:

void init(int width, int height, int cellSize){
    GRID_WIDTH = width / cellSize;
    GRID_HEIGHT = height / cellSize;
    CELL_SIZE = cellSize;
    WORLD_WIDTH = width;
    WORLD_HEIGHT = height;
    grid.resize(GRID_WIDTH * GRID_HEIGHT);
    for (auto& c : grid) c.indices.reserve(6);
    dirtyCells.reserve(GRID_WIDTH * GRID_HEIGHT / 2);
}

inline void addToCell(int x, int y, uint32_t idx){
    const int cellIdx = cellIndex(x, y);
    if (!grid[cellIdx].dirty) {
        grid[cellIdx].dirty = true;
        dirtyCells.push_back(cellIdx);
    }
    grid[cellIdx].indices.push_back(idx);
}

void buildGrid(std::vector<MovingObject>& objectList){
    //Clear the dirty cells in the grid
    for(int index : dirtyCells){
        grid[index].indices.clear();
        grid[index].dirty = false;
    }
    dirtyCells.clear();

    const float invCell = 1.0f / (float)(CELL_SIZE);
    for(uint32_t i = 0; i < objectList.size(); i++){
        const MovingObject& obj = objectList[i];
        const sf::Vector2f pos = obj.getPosition();
        
        int x = (int)(pos.x * invCell);
        int y = (int)(pos.y * invCell);

        if(x < 0){x = 0;}
        else if(x >= GRID_WIDTH) {x = GRID_WIDTH - 1;}

        if(y < 0) {y = 0;}
        else if(y >= GRID_HEIGHT){y = GRID_HEIGHT -1;}

        addToCell(x, y, i);
    }
}

inline void solveCollision(std::vector<MovingObject>& objectList, uint32_t idxA, uint32_t idxB){
    if(idxA == idxB) return;
    MovingObject& a = objectList[idxA];
    MovingObject& b = objectList[idxB];

    const float rA = a.getRadius();
    const float rB = b.getRadius();
    sf::Vector2f posA = a.getPosition();
    sf::Vector2f posB = b.getPosition();

    float collisionDistance = rA + rB;
    float dx = posA.x - posB.x; 
    float dy = posA.y - posB.y;
    float totalDistance = (dx * dx) + (dy * dy);

    //squared euclidian distance that doesn't use square root function
    //square the collisionDistance instead of finding square root of totalDistance

    if(totalDistance >= collisionDistance * collisionDistance ) return; 
    if(totalDistance  < 0.00001f){
        const float small = 1e-3f;
        posA.x += small; posB.x -= small;
        a.setPosition(posA); b.setPosition(posB);
        a.setLastPosition(posA); b.setLastPosition(posB);
        return;
    }//

    float dist = std::sqrt(totalDistance);
    float invDist = 1.0f / dist;
    float overlap = collisionDistance - dist;
    //sf::Vector2f normal = {dx * invDist, dy * invDist};
    //Replace vector math with float math
    const float normalX = dx * invDist;
    const float normalY = dy * invDist;

    const float half = 0.5f * overlap;
    posA.x += normalX * half;
    posA.y += normalY * half;
    posB.x -= normalX * half;
    posB.y -= normalY * half;

    a.setPosition(posA);
    b.setPosition(posB);

    
    //Apply restitution
    sf::Vector2f vA = posA - a.getLastPosition();
    sf::Vector2f vB = posB - b.getLastPosition();
    sf::Vector2f lastA = a.getLastPosition();
    sf::Vector2f lastB = b.getLastPosition();

    //Find velocity
    float vAX = posA.x - lastA.x;
    float vAY = posA.y - lastA.y;
    float vBX = posB.x - lastB.x;
    float vBY = posB.y - lastB.y;

    //Get relative veolocity
    float relVX = vAX - vBX;
    float relVY = vAY - vBY;
    const float vn = relVX * normalX + relVY * normalY;

    if(vn < 0.f){
        // Equal masses impulse split
        const float j = -(1.f + coefficientOfRestitution) * vn * 0.5f;
        //sf::Vector2f impulse = normal * j;
        //vA += impulse
        //vB -= impulse;

        vA.x += normalX * j;
        vA.y += normalY * j;
        vB.x -= normalX * j;
        vB.y -= normalY * j;
        //Maybe refactor this to be something like
        //a.setLastPosition({posA.x - vA.x}, {posA.y - vA.y});
        a.setLastPosition(posA - vA);
        b.setLastPosition(posB - vB);
    }
}

inline void processCell(std::vector<MovingObject>& objectList, uint32_t index){
    const Cell& c = grid[index];
    const std::vector<uint32_t>& indices = c.indices;
    const uint32_t len = indices.size();
    if(len == 0) return;

    // Intra-cell pairs, avoid duplicates with i<j
    if(len >= 2){
        for(size_t i = 0; i + 1 < len; ++i){
            for(size_t j = i + 1; j < len; ++j){
                solveCollision(objectList, (uint32_t)indices[i], (uint32_t)indices[j]);
            }
        }
    }

    // Neighbor cells: surrounding cells processed once per pair
    const int cx = (int)(index % GRID_WIDTH);
    const int cy = (int)(index / GRID_WIDTH);
    
    for(int dy = -1; dy <= 1; ++dy){
        for(int dx = -1; dx <= 1; ++dx){
            if(dx == 0 && dy == 0) continue;

            int nx = cx + dx;
            int ny = cy + dy;

            // Forward-only condition to avoid duplicates
            if(ny < cy || (ny == cy && nx <= cx)) continue;

            if(nx < 0 || nx >= GRID_WIDTH || ny < 0 || ny >= GRID_HEIGHT) continue;

            const std::vector<uint32_t>& other = grid[cellIndex(nx, ny)].indices;
            if(other.empty()) continue;
            for(int ia : indices){
                for(int ib : other){
                    solveCollision(objectList, (uint32_t)ia, (uint32_t)ib);
                }
            }
        }
    } 
    
}

void applyCollisions(std::vector<MovingObject>& objectList){
    buildGrid(objectList);
    
    for(int cellIndex : dirtyCells){
        const Cell& cell = grid[cellIndex];
        processCell(objectList, cellIndex);
    }
}
};