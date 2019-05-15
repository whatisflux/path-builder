#include <winsock2.h>
#include <stdio.h>

#include <SFML/Graphics.hpp>

#include "PathBuilder.h"

#define IN_WIDTH 80
#define IN_HEIGHT 60

sf::Texture PointsToFloorDiagram(std::vector<pb::Vector> points, int width);

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 800), "UDP Image Receiver Test");

    sf::Texture receivedTexture;
    receivedTexture.create(IN_WIDTH, IN_HEIGHT);
    sf::Sprite receivedSprite;
    receivedSprite.setTexture(receivedTexture);
    receivedSprite.setScale(2, 2);

    sf::Texture floorTex;
    sf::Sprite floorSprite;
    floorSprite.setPosition(200, 200);

    pb::Camera cam(4.89f, 5.95f, 3.35f);
    cam.height = 2.f;
    cam.theta = 0.43633231299f;
    cam.phi = 0;
    cam.posX = 0;
    cam.posY = 0;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    struct sockaddr_in local;
    struct sockaddr_in from;
    int fromlen = sizeof(from);
    local.sin_family = AF_INET;
    local.sin_port = htons(1234);
    local.sin_addr.s_addr = INADDR_ANY;

    SOCKET socketS = socket(AF_INET, SOCK_DGRAM, 0);
    bind(socketS, (sockaddr*)&local, sizeof(local));


    while (window.isOpen())
    {
        char buffer[4096];
        ZeroMemory(buffer, sizeof(buffer));

        int bytes = recvfrom(socketS, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromlen);
        if (bytes != SOCKET_ERROR)
        {
            printf("Received message from %s (%d bytes)\n", inet_ntoa(from.sin_addr), bytes);
            if (bytes == IN_WIDTH * IN_HEIGHT / 8)
            {
                // Image was received
                bool bitmap[IN_WIDTH * IN_HEIGHT];
                sf::Uint8 pixels[IN_WIDTH * IN_HEIGHT * 4];

                for (int i = 0; i < bytes; i++)
                {
                    for (int bit = 0; bit < 8; bit++)
                    {
                        // Get the bit-th bit of the byte at buffer[i]
                        sf::Uint8 val = (buffer[i] >> bit) & 1;
                        int p = (i * 8 + bit) * 4;
                        bitmap[i*8+bit] = val;
                        pixels[p] = val * 255;
                        pixels[p+1] = val * 255;
                        pixels[p+2] = val * 255;
                        pixels[p+3] = 255;
                    }
                }

                receivedTexture.update(pixels);
                receivedSprite.setTexture(receivedTexture);

                auto floorPoints = pb::CalculateRelFloorPositions(bitmap, IN_WIDTH, IN_HEIGHT, cam);
                floorTex = PointsToFloorDiagram(floorPoints, 400);
                floorSprite.setTexture(floorTex);
            }
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        window.clear();
        window.draw(receivedSprite);
        window.draw(floorSprite);
        window.display();
    }

    closesocket(socketS);

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

        if (sy >= height || sx >= width) continue;

        img.setPixel(sx, sy, sf::Color(0, 255, 0, 255));
    }

    sf::Texture tex;
    tex.loadFromImage(img);

    return tex;
}