#include <vector>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <utility>
#include <SFML/System/Vector2.hpp>

#include "MovingObject.hpp"

class GridCollision{
private:
struct Cell {
    std::vector<int> indices;
    bool dirty = false;
};
float coefficientOfRestitution = 0.98f;
int CELL_SIZE = 10;
int GRID_WIDTH = 192;
int GRID_HEIGHT = 108;
int WORLD_WIDTH = 1920;
int WORLD_HEIGHT = 1080;

std::vector<Cell> grid;
std::vector<int> dirtyCells;

inline int cellIndex(int x, int y) const {
    return y * GRID_WIDTH + x;
}

//Create hash of pair 
inline std::size_t makeKey(int a, int b){
    if( b < a) std::swap(a,b);
    return (std::hash<int>{}(a) ^ std::hash<int>{}(b));
}

public:
void init(int width, int height, int cellSize){
    GRID_WIDTH = width / cellSize;
    GRID_HEIGHT = height / cellSize;
    CELL_SIZE = cellSize;
    WORLD_WIDTH = width;
    WORLD_HEIGHT = height;
    grid.resize(GRID_WIDTH * GRID_HEIGHT);
    for (auto& c : grid) c.indices.reserve(8);
    dirtyCells.reserve(GRID_WIDTH * GRID_HEIGHT / 4);
}
//
void buildGrid(std::vector<MovingObject>& objectList){
    //Clear the dirty cells in the grid
    for(int index : dirtyCells){
        grid[index].indices.clear();
        grid[index].dirty = false;
    }
    dirtyCells.clear();

    const float invCellSize = 1.0f / CELL_SIZE;
    //TODO refactor this eventually
    
    for (int objIndex = 0; objIndex < objectList.size(); objIndex++) {
        auto& obj = objectList[objIndex];
        sf::Vector2f pos = obj.getPosition();
        const float r = obj.getRadius();
        /*
        const int x = (std::max)(0, int(pos.x * invCellSize));
        const int y = (std::min)(GRID_HEIGHT - 1, int(pos.y * invCellSize));

        if (x > 0 && x < WORLD_WIDTH - 1 && 
            y > 0 && y < WORLD_HEIGHT - 1){
            int cellIdx = cellIndex(x, y);
            if(cellIdx >= 0 && cellIdx < grid.size()) {
                if(!grid[cellIdx].dirty){
                    grid[cellIdx].dirty = true;
                    dirtyCells.push_back(cellIdx);
                }
                grid[cellIdx].indices.push_back(objIndex);
            }
        }
        */
        /*
        int startX = (std::max)(0, int((pos.x - r) / CELL_SIZE));
        int endX   = (std::min)(GRID_WIDTH - 1, int((pos.x + r) / CELL_SIZE));
        int startY = (std::max)(0, int((pos.y - r) / CELL_SIZE));
        int endY   = (std::min)(GRID_HEIGHT - 1, int((pos.y + r) / CELL_SIZE));
        */
        
        const int startX = (std::max)(0, int((pos.x - r) * invCellSize));
        const int endX   = (std::min)(GRID_WIDTH - 1, int((pos.x + r) * invCellSize));
        const int startY = (std::max)(0, int((pos.y - r) * invCellSize));
        const int endY   = (std::min)(GRID_HEIGHT - 1, int((pos.y + r) * invCellSize));

        for (int gx = startX; gx <= endX; gx++) {
            for (int gy = startY; gy <= endY; gy++) {
                int cellIdx = cellIndex(gx, gy);
                if (!grid[cellIdx].dirty) {
                    grid[cellIdx].dirty = true;
                    dirtyCells.push_back(cellIdx);
                }
                grid[cellIdx].indices.push_back(objIndex);
            }
        }
        
    }
}

void applyCollisions(std::vector<MovingObject>& objectList){
    buildGrid(objectList);

    //std::unordered_set<std::size_t> seenPairs;
    //seenPairs.reserve(objectList.size() * 2);

    for(int cellIndex : dirtyCells){
        auto& indices = grid[cellIndex].indices;
        if(indices.size() < 2) continue;
        
      /*sort potentialaly costs more at higher particles
        than it saves due to number of times it has to sort
        small amounts of particles in a cell
        TODO find more efficient way to figure out 
        how to not do duplicate calculations*/
        //std::sort(indices.begin(), indices.end());

        for(size_t i = 0; i < indices.size(); i++){
            MovingObject& a = objectList[indices[i]];

            for(size_t j = i + 1; j < indices.size(); j++){
                MovingObject& b = objectList[indices[j]];

                //std::size_t key = makeKey(indices[i], indices[j]);
                //if(seenPairs.find(key) != seenPairs.end()) continue;
                //seenPairs.insert(key);

                sf::Vector2f distance = a.getPosition() - b.getPosition();
                float collisionDistance = a.getRadius() + b.getRadius();
                float totalDistance = (distance.x * distance.x) + (distance.y * distance.y);

                //squared euclidian distance that doesn't use square root function
                //square the collisionDistance instead of finding square root of totalDistance                
                if(totalDistance < collisionDistance * collisionDistance){
                    //float dist = std::sqrt(totalDistance);
                    float invDist = 1 / std::sqrtf(totalDistance);
                    //if(dist == 0) continue;

                    //sf::Vector2f normal = distance / dist;
                    sf::Vector2f normal = distance * invDist;
                    //float overlap = collisionDistance - dist;
                    float overlap = collisionDistance - (1.0f / invDist); //approximate distance

                    float invRadius = 1 / (a.getRadius() + b.getRadius());
                    //float mass1 = a.getRadius() / (a.getRadius() + b.getRadius());
                    //float mass2 = b.getRadius() / (a.getRadius() + b.getRadius());
                    float mass1 = a.getRadius() * invRadius;
                    float mass2 = b.getRadius() * invRadius;

                    //TODO figure out how exactly this works
                    float overlapCorrection = 0.5f * coefficientOfRestitution * overlap;

                    a.setPosition(a.getPosition() + (normal * (mass2 * overlapCorrection)));
                    b.setPosition(b.getPosition() - (normal * (mass1 * overlapCorrection)));
                }
            }
        }
    }
}
};
