#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <vector>

namespace pb
{
    struct Vector
    {
        float x;
        float y;
    };

    class Camera
    {
    private:
        float fovx;
        float fovy;
    public:
        float focalLength;
        float screenWidth;
        float screenHeight;

        float theta;
        float phi;
        float height;

        float posX;
        float posY;

        Camera(float focalLength, float screenWidth, float screenHeight)
        {
            this->focalLength = focalLength;
            this->screenWidth = screenWidth;
            this->screenHeight = screenHeight;

            fovx = 2 * atan(screenWidth / (2 * focalLength));
            fovy = 2 * atan(screenHeight / (2 * focalLength));
        }

        float getFovX() { return fovx; }
        float getFovY() { return fovy; }
    };

    std::string serializeFloat(float v);

    struct Waypoint
    {
        pb::Vector position;
        bool insideIsLeft;

        std::string serialize();
    };
    struct Edge
    {
        std::vector<pb::Waypoint> waypoints;
        bool isClosed;

        std::string serialize();
    };
    struct Path
    {
        pb::Edge edge1;
        pb::Edge edge2;

        std::string serialize();
    };

    std::vector<pb::Vector> CalculateRelFloorPositions(bool* pixels, int width, int height, pb::Camera cam);
    std::vector<pb::Vector> RelPositionsToAbsPositions(std::vector<pb::Vector> relative, pb::Camera cam);
}