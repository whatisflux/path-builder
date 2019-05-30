//
// Created by peter on 5/28/19.
//

#include "Craig.h"

bool Craig::pointInImage(Point p, Mat img)
{
	return p.x >= 0 && p.x < img.cols && p.y >= 0 && p.y < img.rows;
}

Point2f Craig::tranformPoint(Point2f s, Size imgSize)
{
	Point2f t;
	auto o = Point2f(s.x / imgSize.width * 80, s.y / imgSize.height * 60);
	// Apply experimentally determined camera curve fit
	t.x = 4 * 1400 / (o.y + 7.5) * (o.x - imgSize.width / 2) / imgSize.width;
	t.y = 190000 / ((o.y + 16.9)*(o.y + 16.9));
	return t;
}

Point Craig::findNextPoint(Mat img, Point previous, Point current, Mat debugOut)
{
	Point2f ds = current - previous;
	Point2f u = ds / hypot(ds.x, ds.y);
	Point2f v(-u.y, u.x); // Perpendicular to u
	Point next = current + (Point)(u * WALK_LENGTH);
	if (next.y < MIN_SCAN_Y || next.y >= img.rows || next.x < 0 || next.x >= img.cols) return Point(-1, -1);

	if (img.at<uchar>(next) == 0)
	{
#ifdef _D_DEBUG
		debugOut.at<Vec3b>(next) = Vec3b(255, 200, 0);
#endif
		bool pointFound = false;

		// Scan perpendicular to u

		for (int d = 1; d < HORIZ_SCAN_OFFSET; d++)
		{
			Point right = next + (Point)(v * d);
			Point left = next - (Point)(v * d);

#ifdef _D_DEBUG
			if (pointInImage(right, img)) debugOut.at<Vec3b>(right) = Vec3b(0, 150, 255);
#endif
			if (pointInImage(right, img) && right.y > MIN_SCAN_Y && img.at<uchar>(right) > 0)
			{
				pointFound = true;
				next = right;
				break;
			}

#ifdef _D_DEBUG
			if (pointInImage(left, img)) debugOut.at<Vec3b>(left) = Vec3b(0, 150, 255);
#endif
			if (pointInImage(left, img) && left.y > MIN_SCAN_Y && img.at<uchar>(left) > 0)
			{
				pointFound = true;
				next = left;
				break;
			}
		}

		if (!pointFound) return Point(-1, -1);
	}

	Point2f pointSum(0, 0);
	int pointCount = 0;
	int x = next.x;
	Point2f tmp = next;
	while (pointInImage(tmp, img) && img.at<uchar>(tmp) > 0)
	{
		pointSum += tmp;
		pointCount++;
#ifdef _D_DEBUG
		debugOut.at<Vec3b>(tmp) = Vec3b(0, 255, 0);
#endif
		tmp += v;
	}
	tmp = (Point2f)next - v;
	while (pointInImage(tmp, img) && img.at<uchar>(tmp) > 0)
	{
		pointSum += tmp;
		pointCount++;
#ifdef _D_DEBUG
		debugOut.at<Vec3b>(tmp) = Vec3b(0, 255, 0);
#endif
		tmp -= v;
	}

	next = (Point)(pointSum / pointCount);

	return next;
}

Point Craig::findFirstPoint(Mat img, Mat debugOut)
{
	int height = img.rows;
	int width = img.cols;

	int xsum = 0;
	int xcount = 0;
	int lastx = -1;
	bool found = false;
	int windowBot = height - 1 + INIT_SCAN_HEIGHT;
	while (!found && windowBot >= MIN_SCAN_Y)
	{
		windowBot -= INIT_SCAN_HEIGHT;
		for (int y = windowBot; y >= windowBot - INIT_SCAN_HEIGHT; y--)
		{
			bool foundInRow = false;
			for (int x = 0; x < width / 2 && !foundInRow; x++)
			{
				if (lastx == -1 || x - lastx <= INIT_POINT_MAX_DIST)
				{
					if (img.at<uchar>(y, x) > 0)
					{
#ifdef _D_DEBUG
						debugOut.at<Vec3b>(y, x) = Vec3b(255, 0, 255);
#endif
						bool groupIsGood = true;
						for (int j = x + 1; j < x + INIT_GROUP_SIZE; j++)
						{
							groupIsGood = groupIsGood && img.at<uchar>(y, j) > 0;
#ifdef _D_DEBUG
							if (groupIsGood) debugOut.at<Vec3b>(y, j) = Vec3b(0, 255, 255);
#endif
						}
						if (groupIsGood)
						{
							lastx = x + INIT_GROUP_SIZE / 2;
							xsum += lastx;
							xcount++;
							foundInRow = true;
						}
					}
				}
			}
			found = foundInRow || found;
		}
	}

	if (xcount == 0) return Point(-1, -1);

	int avgx = xsum / xcount;
	int avgy = windowBot - INIT_SCAN_HEIGHT / 2;
	return Point(avgx, avgy);
}

std::vector<Point2f> Craig::processImage(Mat &mat, Mat &debugOut)
{
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3), Point(-1, -1));
	Mat img = mat.clone();

	//morphologyEx(img, img, MORPH_OPEN, kernel);
	//morphologyEx(img, img, MORPH_CLOSE, kernel);
	//dilate(img, img, kernel);

	//resize(img, img, Size(80, 60));
	threshold(img, img, 1, 255, THRESH_BINARY);

	cvtColor(img, debugOut, COLOR_GRAY2BGR);

	std::vector<Point2f> points(0);

	int height = img.rows;
	int width = img.cols;

	Point currentPoint = findFirstPoint(img, debugOut);
	Point previousPoint = Point(currentPoint.x, currentPoint.y + 1);
	while (currentPoint.x != -1 && currentPoint.y != -1)
	{
		points.push_back(currentPoint);
#ifdef _D_DEBUG
		line(debugOut, previousPoint, currentPoint, Scalar(255, 0, 0));
		debugOut.at<Vec3b>(currentPoint) = Vec3b(255, 0, 100);
		debugOut.at<Vec3b>(previousPoint) = Vec3b(255, 0, 100);
#endif
		Point tmp = currentPoint;
		currentPoint = findNextPoint(img, previousPoint, currentPoint, debugOut);
		previousPoint = tmp;
	}

	return points;
}