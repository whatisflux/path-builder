#include <winsock2.h>
#include <stdio.h>

#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "UDP Image Receiver Test");

    sf::Texture receivedTexture;
    receivedTexture.create(80, 60);
    sf::Sprite receivedSprite;
    receivedSprite.setTexture(receivedTexture);
    receivedSprite.setScale(10, 10);

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
            printf("Received message from %s (%d bytes)\n", inet_ntoa(from.sin_addr), bytes);
            if (bytes == 600)
            {
                printf("Creating image from bitmap...\n");
                // Image was received
                sf::Uint8 pixels[60 * 80 * 4];

                for (int i = 0; i < bytes; i++)
                {
                    for (int bit = 0; bit < 8; bit++)
                    {
                        // Get the bit-th bit of the byte at buffer[i]
                        sf::Uint8 val = (buffer[i] >> bit) & 1;
                        int p = (i * 8 + bit) * 4;
                        pixels[p] = val * 255;
                        pixels[p+1] = val * 255;
                        pixels[p+2] = val * 255;
                        pixels[p+3] = 255;
                    }
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