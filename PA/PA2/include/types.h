#pragma once

#include <vector>

struct Point
{
    int x, y;
};

struct Interconnection
{
    Point start, end;
};

struct Blockage
{
    int x1, y1, x2, y2;
};

struct RoutingResult
{
    bool success = false;
    std::vector<Point> path;
    int length = 0;
    int bends = 0;
};

struct RouteState
{
    Point pos;
    int dist;
    int bends;
    int lastDir;
    Point parent;

    RouteState(Point p, int d, int b, int dir, Point par)
        : pos(p), dist(d), bends(b), lastDir(dir), parent(par) {}

    bool operator<(const RouteState &other) const
    {
        if (dist != other.dist)
            return dist > other.dist;
        return bends > other.bends;
    }
};
