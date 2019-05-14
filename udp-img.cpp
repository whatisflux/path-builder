#include <winsock2.h>
#include <stdio.h>

#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "UDP Image Receiver Test");
    window.setFramerateLimit(20);

    sf::Texture receivedTexture;
    receivedTexture.create(80, 60);
    sf::Sprite receivedSprite;
    receivedSprite.setTexture(receivedTexture);

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
        char buffer[1024];
        ZeroMemory(buffer, sizeof(buffer));

        printf("Waiting for incoming image...\n");

        int bytes = recvfrom(socketS, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromlen);
        if (bytes != SOCKET_ERROR)
        {
            printf("Received message from %s (%d): %s\n", inet_ntoa(from.sin_addr), bytes, buffer);
            if (bytes == 600)
            {
                printf("Creating image from bitmap");
                // Image was received
                bool* bitmap = reinterpret_cast<bool*>(buffer);
                sf::Uint8 pixels[80*60*4];
                for (int i = 0; i < 80 * 60; i++)
                {
                    // printf("%d", bitmap[i]);
                    pixels[i*4] = bitmap[i] * 255;
                    pixels[i*4+1] = bitmap[i] * 255;
                    pixels[i*4+2] = bitmap[i] * 255;
                    pixels[i*4+3] = 255;
                }
                receivedTexture.update(pixels);
                receivedSprite.setTexture(receivedTexture);
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
        window.display();
    }

    closesocket(socketS);

    return 0;
}