#pragma once
#include <random>

class Rng{
    private:
        std::mt19937 gen;
        std::random_device rd;
        //Rng() : gen(rd()){};
    
    public:
        Rng() : gen(std::random_device{}()) {}

        int randint(int low, int high) {
            std::uniform_int_distribution<> dist(low, high);
            return dist(gen);
        }
};