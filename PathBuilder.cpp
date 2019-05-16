#include "PathBuilder.h"

std::string pb::serializeFloat(float v)
{
    // Convert pointer to the float to a pointer to an int, and then deref it to get a value
    unsigned bits = *(unsigned*)&v;
    std::string serialized;
    for (int byte = 0; byte < 4; byte++)
    {
        // Shift the wanted bits to the lowest 8 bits
        // Bitwise-and it with 255 to clear out unwanted higher bits
        unsigned val = (bits >> (8 * (3 - byte))) & 0b11111111;
        serialized += (unsigned char)val;
    }
    return serialized;
}

std::string pb::Waypoint::serialize()
{
    auto serializedX = serializeFloat(position.x);
    auto serializedY = serializeFloat(position.y);
    char serializedDir = insideIsLeft ? (char)0b11111111 : (char)0;
    printf("%s|%s|%c\n", serializedX.c_str(), serializedY.c_str(), serializedDir);
    return "p" + serializedX + serializedY + serializedDir;
}

std::string pb::Edge::serialize()
{
    std::string serialized;
    serialized += "e";
    for (int i = 0; i < waypoints.size(); i++)
    {
        serialized += waypoints[i].serialize();
    }
    if (isClosed) serialized += "z";
    return serialized;
}

std::string pb::Path::serialize()
{
    return edge1.serialize() + edge2.serialize();
}

std::vector<pb::Vector> pb::CalculateRelFloorPositions(bool* ropeMask, int width, int height, pb::Camera cam)
{
    std::vector<pb::Vector> points;
    float maxdx = 0.01;
    float maxdy = 0.01;

    for (int y = 0; y < height; y++)
    {
        float sy = (float)y / (float)height * 2.f - 1.f;
        float pixTheta = atan(cam.screenHeight * sy / (2.f * cam.focalLength));
        float theta = cam.theta + pixTheta;

        float rayLength = cam.height / tanf(theta);

        if (theta < 0) continue;

        for (int x = 0; x < width; x++)
        {
            int i = y * width + x;

            if (!ropeMask[i]) continue;

            float sx = (float)x / (float)width * 2.f - 1.f;
            float pixPhi = atan(cam.screenWidth * sx / (2.f * cam.focalLength));
            float phi = cam.phi + pixPhi;

            float dx = rayLength * sin(phi);
            float dy = rayLength * cos(phi);
            if (rayLength < 0)
            {
                continue;
            }
            points.push_back({ dx, dy });
            if (dx > maxdx) maxdx = dx;
            if (dy > maxdy) maxdy = dy;
        }
    }

    for (int i = 0; i < points.size(); i++)
    {
        points[i] = {
            points[i].x,
            points[i].y
        };
    }

    return points;
}

std::vector<pb::Vector> RelPositionsToAbsPositions(std::vector<pb::Vector> relative, pb::Camera cam)
{
    std::vector<pb::Vector> absolute(relative);

    for (int i = 0; i < absolute.size(); i++)
    {
        absolute[i].x += cam.posX;
        absolute[i].y += cam.posY;
    }
    
    return absolute;
}