#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdint>
#include <cmath>
#include "MovingObject.hpp"
#include "GridCollision.hpp"

class Engine{
private:
    std::vector<MovingObject> objectList;
    sf::Vector2f gravity = {0, 750};
    sf::RenderWindow& window;
    sf::CircleShape boundaryCircle;
    GridCollision grid;
    float boundaryRadius = (window.getSize().y / 2.f);
    sf::VertexArray particleVertexes;

    void updateObjects(float deltaT){
        for(MovingObject& object : objectList){
            object.update(deltaT);
        }
    }

    void applyGravity(){
        for(MovingObject& object : objectList){
            //object.addAcceleration(gravity);
            object.setAcceleration(gravity);
        }
    }

    void initBoundaryCirlce(int radius){
        boundaryCircle.setRadius(radius);
        boundaryCircle.setPointCount(100);
        boundaryCircle.setOrigin(radius, radius);
        boundaryCircle.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
        boundaryCircle.setFillColor(sf::Color::Black);
    }
    /*
    void applyCollisions(){
        const uint64_t objectCount = objectList.size();
        const float coefficientOfRestitution = 0.70f;

        for(uint64_t i = 0; i < objectCount; i++){
            MovingObject& firstObject = objectList[i];

            //avoid checking each pair twice by setting j = i + 1
            for(uint64_t j = i + 1; j < objectCount; j++){
                //globalCount++;
                MovingObject& secondObject = objectList[j];
                sf::Vector2f distance = firstObject.getPosition() - secondObject.getPosition();
                float collisionDistance = firstObject.getRadius() + secondObject.getRadius();
                float totalDistance = (distance.x * distance.x) + (distance.y * distance.y);

                //squared euclidian distance that doesn't use square root function
                //square the collisionDistance instead of finding square root of totalDistance                
                if(totalDistance < collisionDistance * collisionDistance){
                    float dist = sqrt(totalDistance);
                    sf::Vector2f normal = distance / dist;
                    float overlap = collisionDistance - dist;

                    float radius1 = firstObject.getRadius();
                    float radius2 = secondObject.getRadius();

                    float mass1 = radius1 / (radius1 + radius2);
                    float mass2 = radius2 / (radius1 + radius2);

                    //TODO figure out how exactly this works
                    float overlapCorrection = 0.5f * coefficientOfRestitution * (collisionDistance - dist);

                    firstObject.setPosition(firstObject.getPosition() + (normal * (mass2 * overlapCorrection)));
                    secondObject.setPosition(secondObject.getPosition() - (normal * (mass1 * overlapCorrection)));
                }
            }
        }
    }
    */
    /*
    void applyEdgeCollisions(){
        sf::Vector2f center(boundaryCircle.getPosition());
        const float coefficientOfRestitution = 0.70f;
        for(MovingObject& object : objectList){
            //globalCount++;
            sf::Vector2f objectPosition = object.getPosition();
            const sf::Vector2f distance = objectPosition - center;

            const float totalDistance = ((distance.x * distance.x) + (distance.y * distance.y));
            const float collisionDistance = boundaryRadius - object.getRadius(); //keep circle inside boundary

            if(totalDistance > (collisionDistance * collisionDistance)){
                float dist = sqrt(totalDistance);
                sf::Vector2f normal = distance / dist;

                float overlap = dist - collisionDistance;
                float overlapCorrection = coefficientOfRestitution * overlap;

                object.setPosition(object.getPosition() - (normal * overlapCorrection));
            }
            /*
            float radius = object.getRadius();
            const float coefficientOfRestitution = 0.10f;

            //Start detecting the objects reaching the edge
            //Left edge
            if(pos.x < radius){//radius is always positive
                pos.x = radius;
                object.reflectX(coefficientOfRestitution);
            }

            //Right edge
            if(pos.x > window.getSize().x - radius){
                pos.x = window.getSize().x - radius;
                object.reflectX(coefficientOfRestitution);
            }

            //Top of window
            if(pos.y < radius){
                pos.y = radius;
                object.reflectY(coefficientOfRestitution);
            }

            //Bottom of window
            if(pos.y > window.getSize().y - radius){
                pos.y = window.getSize().y - radius;
                object.reflectY(coefficientOfRestitution);
            }
            object.setPosition(pos);
            globalCount++;
            
        }
    }
    */
    void applyEdgeCollisions(){
        const auto windowX = window.getSize().x;
        const auto windowY = window.getSize().y;
        for(MovingObject& obj : objectList){
            sf::Vector2f pos = obj.getPosition();
            float radius = obj.getRadius();

            //Start detecting the objects reaching the edge
            //Left edge
            if(pos.x < radius){//radius is always positive
                pos.x = radius;
            }
            //Right edge
            if(pos.x > windowX - radius){
                pos.x = windowX - radius;
            }
            //Top of window
            if(pos.y < radius){
                pos.y = radius;
            }
            //Bottom of window
            if(pos.y > windowY - radius){
                pos.y = windowY - radius;
            }
            obj.setPosition(pos);
        }
    }

public:
    Engine(sf::RenderWindow& window) : window(window) {
        particleVertexes.setPrimitiveType(sf::Quads);
    }

    void init(int height, int width, int maxObjects, int particleRadius){
        objectList.reserve(maxObjects); 
        //int radius = height / 2;
        //initBoundaryCircle(radius);
        int cellSize = (particleRadius * 2) * 1.5; //Double radius for diameter
        grid.init(width, height, cellSize);
    }

    //Update all objects in objectList with new positions and variables/
    void update(float deltaT){
        //This will loop through each object, and then all other objects positions will be updated after this one moves?
        applyGravity();
        updateObjects(deltaT);
        grid.applyCollisions(objectList);
        applyEdgeCollisions();

        // Reset acceleration for all objects after each substep
        /*
        for (MovingObject& object : objectList) {
            object.resetAcceleration();
        }   
        */
    }

    /*
    void draw(){
        window.draw(boundaryCircle);
        for(MovingObject& object : objectList){
            window.draw(object.getShape());
        }
    }
    */
    /*
    void draw(){
        window.draw(boundaryCircle);
        sf::CircleShape shape;
        shape.setPointCount(8);
        shape.setFillColor(sf::Color::White);
        
        for(const auto& object : objectList){
            float radius = object.getRadius();
            shape.setRadius(radius);
            shape.setOrigin(radius, radius);
            shape.setPosition(object.getPosition());
            window.draw(shape);
        }
    }
    */

    void updateParticleVisuals(){
        particleVertexes.resize(objectList.size() * 4);
        for(size_t i = 0; i < objectList.size(); i++){
            const auto& object = objectList[i];
            float r = object.getRadius();
            sf::Vector2f pos = object.getPosition();
            std::size_t index = i * 4;

            particleVertexes[index].position = {pos.x - r, pos.y - r};
            particleVertexes[index + 1].position = {pos.x + r, pos.y - r};
            particleVertexes[index + 2].position = {pos.x + r, pos.y + r};
            particleVertexes[index + 3].position = {pos.x - r, pos.y + r};

            sf::Color c = object.getColor();
            particleVertexes[index].color = c;
            particleVertexes[index + 1].color = c;
            particleVertexes[index + 2].color = c;
            particleVertexes[index + 3].color = c;
        }
    }

    void draw(){
        //window.draw(boundaryCircle);
        updateParticleVisuals();
        window.draw(particleVertexes);
    }

    MovingObject& addObject(sf::Vector2f position, float radius, sf::Color color){    
        objectList.emplace_back(position, radius, color);
        return objectList.back();
    }

    int getObjectCount(){
        return objectList.size();
    }
};