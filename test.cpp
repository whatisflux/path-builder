#include <SFML/Graphics.hpp>

#include "CalculatePathPoints.h"

#define WIDTH 800
#define HEIGHT 800

int main()
{
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "AutoCar Path Detector Tester");
    window.setFramerateLimit(60);
    
    sf::RectangleShape quad;
    quad.setFillColor(sf::Color::Magenta);
    quad.setSize(sf::Vector2f(WIDTH, HEIGHT));

    sf::Image ropeMaskImg;
    if (!ropeMaskImg.loadFromFile("resources/rope-mat-1-s6-cam.png")) {
        return -1;
    }

    sf::Texture ropeMaskTex;
    ropeMaskTex.loadFromImage(ropeMaskImg);
    sf::Sprite ropeMaskSprite;
    ropeMaskSprite.setTexture(ropeMaskTex);
    ropeMaskSprite.setPosition(sf::Vector2f(0, 0));

    int imgWidth = ropeMaskImg.getSize().x;
    int imgHeight = ropeMaskImg.getSize().y;

    const sf::Uint8* ropeMaskPixels = ropeMaskImg.getPixelsPtr();
    int ropePixelsLength = imgWidth * imgHeight * 4;

    pb::Camera s6cam(4.35f, 5.95f, 3.55f);
    s6cam.height = 2.f; // meters
    s6cam.theta = 1.134464f; // radians

    bool ropeMask[imgWidth * imgHeight];
    for (int i = 0; i < ropePixelsLength; i += 4)
    {
        ropeMask[i / 4] = ropeMaskPixels[i] / 255;
    }
    auto points = pb::CalculateRelFloorPositions(ropeMask, imgWidth, imgHeight, s6cam);
    for (int i = 0; i < points.size(); i++) {
        printf("(%f, %f)\n", points[i].x, points[i].y);
    }

    // sf::Uint8 ropeDepthPixels[imgWidth * imgHeight * 4];
    // for (int i = 0; i < ropePixelsLength; i++)
    // {
    //     ropeDepthPixels[i] = (sf::Uint8)ropeDepthcPixels[i];
    // }

    // sf::Texture ropeDepthTex;
    // ropeDepthTex.create(imgWidth, imgHeight);
    // ropeDepthTex.update(ropeDepthPixels);
    
    // sf::Sprite ropeDepthSprite;
    // ropeDepthSprite.setTexture(ropeDepthTex);
    // ropeDepthSprite.setPosition(sf::Vector2f(0, HEIGHT / 2));
    
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

        window.clear();
        window.draw(quad);
        window.draw(ropeMaskSprite);
        // window.draw(ropeDepthSprite);
        window.display();
    }

    return 0;
}