#include <winsock2.h>
#include <stdio.h>
#include <string.h>

#include "PathBuilder.h"

int main()
{
    pb::Path path;
    pb::Edge edge1;
    pb::Edge edge2;
    
    edge1.waypoints.push_back({ {0, 0}, 0 });
    edge1.waypoints.push_back({ {0, 1}, 0 });
    edge1.waypoints.push_back({ {-0.5f, 0}, 0 });
    edge1.isClosed = true;

    edge2.waypoints.push_back({ {1, 0}, 1 });
    edge2.waypoints.push_back({ 1, 1 });
    edge2.waypoints.push_back({ {0.5f, 0}, 1 });
    edge2.isClosed = false;

    path.edge1 = edge1;
    path.edge2 = edge2;

    std::string serialized = path.serialize();

    printf("Serialized (%d): %s\n", serialized.length(), serialized.c_str());

    // OPEN SOCKET

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    struct sockaddr_in local;
    struct sockaddr_in dest;

    local.sin_family = AF_INET;
    local.sin_port = htons(1235);
    local.sin_addr.s_addr = INADDR_ANY;

    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr("192.168.49.1");
    dest.sin_port = htons(1235);

    printf("WSA started\n");

    SOCKET socketS = socket(AF_INET, SOCK_DGRAM, 0);
    bind(socketS, (sockaddr*)&local, sizeof(local));

    printf("Socket bound\n");

    int ret = sendto(socketS, serialized.c_str(), serialized.length(), 0, (sockaddr *)&dest, sizeof(dest));

    if (ret == SOCKET_ERROR)
    {
        fprintf(stderr, "Error sending packet: %d", WSAGetLastError());
    } else
    {
        printf("Successfully sent packet: %s", serialized.c_str());
    }

    return 0;
}