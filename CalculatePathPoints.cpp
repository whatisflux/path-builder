#include "CalculatePathPoints.h"

std::vector<pb::Vector> pb::CalculateRelFloorPositions(bool* ropeMask, int width, int height, pb::Camera cam)
{
    std::vector<pb::Vector> points;
    float maxdx = 0.01;
    float maxdy = 0.01;

    for (int y = 0; y < height; y++)
    {
        float sy = (float)y / (float)height * 2.f - 1.f;
        float pixTheta = cam.getFovY() / 2.f * sy;
        float theta = cam.theta + pixTheta;

        float rayLength = cam.height / tanf(theta);

        if (theta < 0) continue;

        printf("%d %f %f, %f\n", y, cam.theta, theta, rayLength);

        for (int x = 0; x < width; x++)
        {
            int i = y * width + x;

            if (!ropeMask[i]) continue;

            float sx = (float)x / (float)width * 2.f - 1.f;
            float pixPhi = cam.getFovX() / 2.f * sx;
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