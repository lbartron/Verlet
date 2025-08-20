#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class MovingObject {
private:
    //sf::CircleShape shape;
    sf::Vector2f position;
    sf::Vector2f lastPosition;
    sf::Vector2f acceleration;
    float radius;

public:
    MovingObject(sf::Vector2f position, float radius){
        this->position = position;
        this->radius = radius;
        lastPosition = position;
        acceleration = {0.0f, 0.0f};
    }

    void update(float deltaT){
        //sf::Vector2f newPosition = position + acceleration * deltaT * deltaT;
        sf::Vector2f displacement = position - lastPosition;
        lastPosition = position;
        position += displacement + acceleration * (deltaT * deltaT);
        //shape.setPosition(position);
    }

    void reflectX(float COR){
        float displacement = position.x - lastPosition.x;
        lastPosition.x = position.x + displacement * COR;
    }

    void reflectY(float COR){
        float displacement = position.y - lastPosition.y;
        lastPosition.y = position.y + displacement * COR;
    }

    void setVelocity(sf::Vector2f newVelocity, float dt){
        lastPosition = position - (newVelocity * dt);
    }
    

    void addAcceleration(sf::Vector2f newAcceleration){
        acceleration += newAcceleration;
    }

    void resetAcceleration(){
        acceleration = {0.0f, 0.0f};
    }

    void setPosition(sf::Vector2f newPosition){
        position = newPosition;
        //shape.setPosition(newPosition);
    }
    /*
    sf::CircleShape& getShape(){
        return shape;
    }*/

    sf::Vector2f getPosition() const{
        return position;
    }

    float getRadius() const{
        return radius;
    }
};