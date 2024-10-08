// Task 3 - Implement Bresenham drawing alg.
//        - Draw a star using lines
//        - Make use of std::vector to define the shape
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>

#include <ppgso/ppgso.h>

// Size of the framebuffer
const unsigned int SIZE = 512;

struct Point {
    int x,y;
};

// Bresenham drawing algorithm
void drawLine(ppgso::Image& framebuffer, Point& from, Point& to) {

    int x0 = from.x;
    int y0 = from.y;
    int x1 = to.x;
    int y1 = to.y;

    int dx = abs(x1 - x0);      // Delta x
    int dy = abs(y1 - y0);      // Delta y
    int p = 2 * dy - dx;           // Starting point of p

    int x = x0, y = y0;            // Start coordinates

    // Starting direction
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    if (dx > dy)
    {
        // If line more horizontal
        while (x != x1)
        {
            framebuffer.setPixel(x, y, 255, 255, 255);
            x += sx;
            if (p >= 0)
            {
                y += sy;
                p += 2 * (dy - dx);
            }
            else
            {
                p += 2 * dy;
            }
        }
    }
    else
    {
        // If line more horizontal
        while (y != y1) {
            framebuffer.setPixel(x, y, 255, 255, 255);
            y += sy;
            if (p >= 0)
            {
                x += sx;
                p += 2 * (dx - dy);
            }
            else
            {
                p += 2 * dx;
            }
        }
    }
}

int main()
{
    // Use ppgso::Image as our framebuffer
    ppgso::Image framebuffer(SIZE, SIZE);

    // Generate star points
    std::vector<Point> points;
    Point point1 = {(int)512/2,0};
    Point point2 = {0,SIZE};
    Point point3 = {SIZE,(int)512/3};
    Point point4 = {0,(int)512/3};
    Point point5 = {SIZE,SIZE};
    Point point6 = {(int)512/2,0};

    points.push_back(point1);
    points.push_back(point2);
    points.push_back(point3);
    points.push_back(point4);
    points.push_back(point5);
    points.push_back(point6);
    // Draw lines
    for(unsigned int i = 0; i < points.size() - 1; i++)
        drawLine(framebuffer, points[i], points[i+1]);

    // Save the result
    std::cout << "Generating task2_bresenham.bmp file ..." << std::endl;
    ppgso::image::saveBMP(framebuffer, "task2_bresenham.bmp");

    std::cout << "Done." << std::endl;
    return EXIT_SUCCESS;
}
