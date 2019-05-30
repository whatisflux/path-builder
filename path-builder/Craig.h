//
// Created by peter on 5/28/19.
//

#ifndef AUTOCAR_CRAIG_H
#define AUTOCAR_CRAIG_H

#ifdef _DEBUG
#include <opencv2/opencv.hpp>
#endif

#include <opencv2/core/types.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>

#define WALK_LENGTH 6
#define MIN_SCAN_Y 10
#define HORIZ_SCAN_OFFSET 5
#define INIT_OFFSET_Y 1

#define INIT_GROUP_SIZE 2
#define INIT_SCAN_HEIGHT 4
#define INIT_POINT_MAX_DIST 5

#define _D_DEBUG

using namespace cv;

class Craig {
private:
	bool pointInImage(Point p, Mat img);

	Point2f tranformPoint(Point2f o, Size imgSize);

	Point findFirstPoint(Mat img, Mat debugOut);
	Point findNextPoint(Mat img, Point previous, Point current, Mat debugOut);
public:
	std::vector<Point2f> processImage(Mat &mat, Mat &debugOut);
};


#endif //AUTOCAR_CRAIG_H