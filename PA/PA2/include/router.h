#pragma once

#include "types.h"
#include <vector>
#include <random>

class Router
{
public:
    Router(int chipX, int chipY);

    void setReserved(const std::vector<Interconnection> &nets);
    void addBlockage(int x1, int y1, int x2, int y2);
    RoutingResult route(const Interconnection &net);
    std::vector<Point> reconstructRandomPath(const Point &start, const Point &end,
                                             const std::vector<std::vector<std::vector<Point>>> &parents, std::mt19937 &gen);
    double getScore(const Interconnection &net, double alpha, double beta, double gamma);
    int countFreeNeighbors(const Point &p);
    int edgeProximity(const Point &p);

private:
    int chipX, chipY;
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<bool>> reserved;
    const int FREE = 0;
    const int BLOCKED = -1;
    static inline int dx[4] = {0, 1, 0, -1};
    static inline int dy[4] = {1, 0, -1, 0};

    bool inBounds(int x, int y) const;
    static int countBends(const std::vector<Point> &path);
    void blockPath(const std::vector<Point> &path);
};
