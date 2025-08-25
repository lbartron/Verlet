#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class MovingObject {
private:
    sf::Vector2f position;
    sf::Vector2f lastPosition;
    sf::Vector2f acceleration;
    float radius;
    sf::Color color;

public:
    MovingObject(sf::Vector2f position, float radius, sf::Color color){
        this->position = position;
        this->radius = radius;
        lastPosition = position;
        acceleration = {0.0f, 0.0f};
        this->color = color;
    }

    void update(float deltaT){
        //sf::Vector2f newPosition = position + acceleration * deltaT * deltaT;
        sf::Vector2f displacement = position - lastPosition;
        lastPosition = position;
        position += displacement + acceleration * (deltaT * deltaT);
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

    void setAcceleration(sf::Vector2f newAcceleration){
        acceleration = newAcceleration;
    }

    void resetAcceleration(){
        acceleration = {0.0f, 0.0f};
    }

    void setPosition(sf::Vector2f newPosition){
        position = newPosition;
        //shape.setPosition(newPosition);
    }

    void setLastPosition(sf::Vector2f newPosition){
        lastPosition = newPosition;
    }

    sf::Color getColor() const{
        return color;
    }

    sf::Vector2f getPosition() const{
        return position;
    }

    sf::Vector2f getLastPosition() const{
        return lastPosition;
    }

    float getRadius() const{
        return radius;
    }
};