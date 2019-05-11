#include <stddef.h>
#include <stdio.h>
#include <math.h>
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

        Camera(float focalLength, float screenWidth, float screenHeight)
        {
            this->focalLength = focalLength;
            this->screenWidth = screenWidth;
            this->screenHeight = screenHeight;

            fovx = 2 * atan(screenWidth / (2 * (sqrtf(4 * focalLength * focalLength - screenWidth * screenWidth / 4) - focalLength)));
            fovy = 2 * atan(screenHeight / (2 * (sqrtf(4 * focalLength * focalLength - screenHeight * screenHeight / 4) - focalLength)));
        }

        float getFovX() { return fovx; }
        float getFovY() { return fovy; }
    };

    std::vector<pb::Vector> CalculateRelFloorPositions(bool* pixels, int width, int height, pb::Camera cam);

}