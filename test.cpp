#include <SFML/Graphics.hpp>

#include "PathBuilder.h"

#define WIDTH 800
#define HEIGHT 800

sf::Texture PointsToFloorDiagram(std::vector<pb::Vector> points, int width);

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

    pb::Camera s6cam(4.35f, 5.95f, 3.55f); // focal length, screen width, screen height (mm)
    s6cam.height = 2.f; // meters
    s6cam.theta = 0.43633231299f; // radians

    bool ropeMask[imgWidth * imgHeight];
    for (int i = 0; i < ropePixelsLength; i += 4)
    {
        ropeMask[i / 4] = ropeMaskPixels[i] / 255;
    }
    auto points = pb::CalculateRelFloorPositions(ropeMask, imgWidth, imgHeight, s6cam);

    printf("FOV: (%f, %f)", s6cam.getFovX(), s6cam.getFovY());

    sf::Sprite floorDiagram;
    auto floorTexture = PointsToFloorDiagram(points, 400);
    floorDiagram.setTexture(floorTexture);
    floorDiagram.setPosition(sf::Vector2f(200, 150));
    
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
        window.draw(floorDiagram);
        window.display();
    }

    return 0;
}

sf::Texture PointsToFloorDiagram(std::vector<pb::Vector> points, int width)
{
    sf::Image img;

    float minx = 1e9; float maxx = -1e9; float miny = 1e9; float maxy = -1e9;
    for (int i = 0; i < points.size(); i++)
    {
        float x = points[i].x;
        float y = points[i].y;
        minx = x < minx ? x : minx;
        maxx = x > maxx ? x : maxx;
        miny = y < miny ? y : miny;
        maxy = y > maxy ? y : maxy;
    }

    float fwidth = maxx - minx;
    float fheight = maxy - miny;
    float aspect = fheight / fwidth;
    int height = width * aspect;
    img.create(width, height, sf::Color(0, 0, 0, 255));

    float widthScale = (float)width / fwidth;
    float heightScale = (float)height / fheight;

    for (int i = 0; i < points.size(); i++)
    {
        float x = points[i].x;
        float y = points[i].y;
        int sx = (x - minx) * widthScale;
        int sy = height - (y - miny) * heightScale;

        img.setPixel(sx, sy, sf::Color(0, 255, 0, 255));
    }

    printf("x: (%f, %f), y: (%f, %f)\n", minx, maxx, miny, maxy);

    sf::Texture tex;
    tex.loadFromImage(img);

    

    return tex;
}