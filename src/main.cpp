#include <optional>
#include <iostream>
#include <chrono>
#include <sstream>
#include <string>
#include <windows.h> //temp to find src file
#include <SFML/Graphics.hpp>

#include "Engine.hpp"
#include "Rng.hpp"

int main()
{
    //temp to find src file/
    /*
    char cwd[1024];
    if (GetCurrentDirectoryA(sizeof(cwd), cwd))
        std::cout << "Current working directory: " << cwd << std::endl;
    */
    const int windowHeight = 1080;
    const int windowWidth = 1920;
    const int maxObjects = 12000;

    sf::ContextSettings settings;
    //for smoothing out jagged edges of shapes
    settings.antialiasingLevel = 0;//set to 0 for optimization

    //Create render window and set framerate
    auto window = sf::RenderWindow(sf::VideoMode({windowWidth, windowHeight}), "Verlet Physics Simulator",sf::Style::Default, settings);
    //window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    Engine engine(window);
    engine.init(windowHeight, windowWidth, maxObjects);

    //Add Rng object and position object
    Rng random;
    sf::Vector2f position = {windowWidth / 2, 50};

    //Add timing elements
    sf::Clock clock;
    float accumulator = 0.0f;
    const float fixedDeltaT = 1.0f / 180.0f; //180 Hz physics
    const float spawnInterval = 0.0025f;
    //float spawnTimer = 0.0f;
    double lastSpawnTime = 0.0;

    
    //FPS counter/
    sf::Clock fpsClock;
    int frameCount = 0;
    float fps = 0.f;
    sf::Font font;
    
    std::string fontPath = "../../../fonts/arial.ttf";
    //std::string fontPath = "fonts/arial.ttf";
    std::cout << "Loading font from: " << fontPath << std::endl;
    if (!font.loadFromFile(fontPath)) {
        std::cout << "Error loading font\n";
        throw std::runtime_error("Failed to load font");
    }
    sf::Text infoText;
    infoText.setFont(font);
    infoText.setCharacterSize(16);
    infoText.setFillColor(sf::Color::Black);
    infoText.setPosition(10.f, 10.f);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    const int maxSubsteps = 8;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        
        frameCount++;
        if(fpsClock.getElapsedTime().asSeconds() >= 1.0f){
            fps = frameCount / fpsClock.getElapsedTime().asSeconds();
            frameCount = 0;
            fpsClock.restart();
        }

        infoText.setString(
            "FPS: " + std::to_string(static_cast<int>(fps)) + 
            "\nObjects: " + std::to_string(engine.getObjectCount())
        );
        

        //float deltaT = clock.restart().asSeconds();
        float frameTime = clock.restart().asSeconds();
        accumulator += frameTime;
        //spawnTimer += frameTime;
        if(accumulator >0.25f) accumulator = 0.25f;

        //TODO fix updatecount not updating in a set amount of substeps
        //Target is to update 8 times per frame or 8 substeps
        //When the system lags, it causes deltaT to become larger causing there to be
        //more updates per frame
        int substeps = 0;
        while(accumulator >= fixedDeltaT && substeps < maxSubsteps){
            engine.update(fixedDeltaT);
            accumulator -= fixedDeltaT;
            substeps++;
        }
        //std::cout << "The substep count was: " << substeps << std::endl;

        if(substeps == maxSubsteps){
            accumulator = 0.0f;
        }

        auto now = std::chrono::high_resolution_clock::now();
        double elapsedSeconds = std::chrono::duration<double>(now - startTime).count();

        //spawnTimer >= spawnInterval
        const int batchSize = 10;
        if(engine.getObjectCount() <= maxObjects && (elapsedSeconds - lastSpawnTime) >= spawnInterval){
            for(int i = 0; i < batchSize && engine.getObjectCount() < maxObjects; i++){
                MovingObject& temp = engine.addObject(position, random.randint(3, 5));
                float x = random.randint(-500,500);
                float y = random.randint(50, 300);
                temp.setVelocity(sf::Vector2f{x, y}, fixedDeltaT);
                //sf::Color customColor(random.randint(0, 255), random.randint(0, 255), random.randint(0, 255));
                //temp.getShape().setFillColor(customColor);
                //spawnTimer = 0.0f;
                lastSpawnTime = elapsedSeconds;
            }
        }
        //start = std::chrono::high_resolution_clock::now();

        window.clear(sf::Color::White);
        engine.draw();
        //end = std::chrono::high_resolution_clock::now();
        //fps = (float)1e9/(float)std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
        /*
        if(fps < lowestFps){
            lowestFps = fps;
        }
        */
        
        /*
        std::ostringstream ss;
        ss << fps;

        sf::Text atext;
        atext.setFont(font);
        atext.setCharacterSize(20);
        atext.setStyle(sf::Text::Bold);
        atext.setColor(sf::Color::Black);
        atext.setPosition(0,0);
        atext.setString(ss.str());
        window.draw(atext);
        */
        //std::cout << engine.getObjectCount() << " Objects \n";
        window.draw(infoText);
        window.display();
    }
    //std::cout << "The lowest fps was " << lowestFps << std::endl;
    //std::cout << "The loop count was " << engine.getGlobalCount() << std::endl;
    return 0;
}
