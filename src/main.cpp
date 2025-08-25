#include <optional>
#include <iostream>
#include <chrono>
#include <sstream>
#include <string>
//#include <windows.h> //temp to find src file
#include <SFML/Graphics.hpp>

#include "Engine.hpp"
#include "Rng.hpp"

inline sf::Color makeColor(float t){
    constexpr float TAU = 6.28318530718f;
    // Three phased sin waves mapped to 0..1
    float r = 0.5f + 0.5f * std::sin((t + 0.00f) * TAU);
    float g = 0.5f + 0.5f * std::sin((t + 0.33f) * TAU);
    float b = 0.5f + 0.5f * std::sin((t + 0.66f) * TAU);
    // Optional slight gamma to boost brightness (gamma < 1 brightens)
    auto toByte = [](float c){
        c = std::pow(c, 0.8f); // brighten (remove or adjust if too strong)
        if(c < 0.f) c = 0.f; else if(c > 1.f) c = 1.f;
        return static_cast<sf::Uint8>(c * 255.f + 0.5f);
    };
    return {toByte(r), toByte(g), toByte(b)};
}

int main()
{
    const int windowHeight = 1080;
    const int windowWidth = 1920;
    const int worldWidth = windowWidth * 2;
    const int worldHeight = windowHeight * 2;
    const int maxObjects = 20000;
    const int particleRadius = 5;

    sf::ContextSettings settings;
    //for smoothing out jagged edges of shapes
    settings.antialiasingLevel = 0;//set to 0 for optimization

    //Create render window and set framerate
    auto window = sf::RenderWindow(sf::VideoMode({windowWidth, windowHeight}), "Verlet Physics Simulator",sf::Style::Default, settings);
    window.setFramerateLimit(65);
    //window.setVerticalSyncEnabled(true);

    //sf::View view = window.getDefaultView();
    sf::View view(sf::FloatRect(0,0,worldWidth, worldHeight));
    //view.zoom(2.0f);
    window.setView(view);

    Engine engine(window);
    engine.init(worldHeight, worldWidth, maxObjects, particleRadius);

    //Add Rng object and position object
    Rng random;
    sf::Vector2f position = {10, 50};

    //Add timing elements
    sf::Clock clock;
    float accumulator = 0.0f;
    const float fixedDeltaT = 1.0f / 480.0f; //180 Hz physics
    const float spawnInterval = 0.0050f;
    //float spawnTimer = 0.0f;
    double lastSpawnTime = 0.0;

    
    //FPS counter
    sf::Clock fpsClock;
    int frameCount = 0;
    float fps = 0.f;
    sf::Font font;
    
    std::string fontPath = "../../../fonts/arial.ttf";
    std::cout << "Loading font from: " << fontPath << std::endl;
    if (!font.loadFromFile(fontPath)) {
        std::cout << "Error loading font\n";
        throw std::runtime_error("Failed to load font");
    }
    sf::Text infoText;
    infoText.setFont(font);
    infoText.setCharacterSize(48);
    infoText.setFillColor(sf::Color::White);
    infoText.setPosition(10.f, 10.f);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    const int maxSubsteps = 8;
    uint16_t spawnIndex = 0;

    //const float targetFrame = 1.f / 120.f; // cap render to 120 FPS (adjust)
    //sf::Clock frameCapClock;
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
        //std::cout << "The substep count was: " << substeps << "\n";

        if(substeps == maxSubsteps){
            // Just clamp leftover so we keep “owed” time without runaway
            accumulator = std::min(accumulator, fixedDeltaT * (float)maxSubsteps);
        }

        auto now = std::chrono::high_resolution_clock::now();
        double elapsedSeconds = std::chrono::duration<double>(now - startTime).count();

        //spawnTimer >= spawnInterval
        const int batchSize = 20;
        if(fps >= 60.0f && engine.getObjectCount() <= maxObjects && (elapsedSeconds - lastSpawnTime) >= spawnInterval){
            for(int i = 0; i < batchSize && engine.getObjectCount() < maxObjects; i++){
                //position.y = random.randint(50, 55);
                sf::Vector2f newPosition = {position.x, position.y + (i * (particleRadius * 2))};
                sf::Color customColor = makeColor(spawnIndex * 0.0002);
                //sf::Color customColor = sf::Color::White;
                spawnIndex++;
                MovingObject& temp = engine.addObject(newPosition, particleRadius, customColor);//random.randint(1, 2));
                float x = 2500; //random.randint(0,100);
                float y = 200; //random.randint(50, 300);
                temp.setVelocity(sf::Vector2f{x, y}, fixedDeltaT);
                //temp.getShape().setFillColor(customColor);
                //spawnTimer = 0.0f;
                lastSpawnTime = elapsedSeconds;
            }
        }
        //start = std::chrono::high_resolution_clock::now();

        window.clear(sf::Color::Black);
        engine.draw();
        //end = std::chrono::high_resolution_clock::now();
        //fps = (float)1e9/(float)std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
        window.draw(infoText);
        window.display();

        // Manual frame cap (only if vsync disabled)
        /*
        float frameElapsed = frameCapClock.getElapsedTime().asSeconds();
        if(frameElapsed < targetFrame){
            sf::sleep(sf::seconds(targetFrame - frameElapsed));
        }
        frameCapClock.restart();
        */
    }
    return 0;
}
