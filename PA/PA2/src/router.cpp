#include "router.h"
#include <queue>
#include <algorithm>
#include <random>
#include <climits>
#include <iostream>

using namespace std;

// Constructor: Initializes the routing grid and reservation map.
// The grid size is (chipX + 1) × (chipY + 1), with all cells marked as FREE.
// The reserved map tracks fixed terminals that cannot be used for routing.
Router::Router(int chipX, int chipY) : chipX(chipX), chipY(chipY)
{
    grid.assign(chipX + 1, vector<int>(chipY + 1, FREE));
    reserved.assign(chipX + 1, vector<bool>(chipY + 1, false));
}

// Marks the start and end points of each net as reserved.
// Reserved cells are excluded from general routing, except as endpoints.
void Router::setReserved(const vector<Interconnection> &nets)
{
    for (const auto &net : nets)
    {
        if (inBounds(net.start.x, net.start.y))
            reserved[net.start.x][net.start.y] = true;
        if (inBounds(net.end.x, net.end.y))
            reserved[net.end.x][net.end.y] = true;
    }
}

// Adds a rectangular blockage to the grid between (x1, y1) and (x2, y2).
// All cells within the bounding box are marked as BLOCKED if in bounds.
void Router::addBlockage(int x1, int y1, int x2, int y2)
{
    int lowX = min(x1, x2), highX = max(x1, x2);
    int lowY = min(y1, y2), highY = max(y1, y2);
    for (int x = lowX; x <= highX; x++)
    {
        for (int y = lowY; y <= highY; y++)
        {
            if (inBounds(x, y))
                grid[x][y] = BLOCKED;
        }
    }
}

// Attempts to route a single net using BFS.
// Returns a RoutingResult containing the path, length, bend count, and success flag.
// If no valid path is found, result.success remains false.
// The path is reconstructed randomly from multiple shortest paths.
// Once routed, the path is blocked to prevent reuse.
RoutingResult Router::route(const Interconnection &net)
{
    RoutingResult result;
    result.success = false;

    if (!inBounds(net.start.x, net.start.y) || !inBounds(net.end.x, net.end.y) ||
        grid[net.start.x][net.start.y] != FREE || grid[net.end.x][net.end.y] != FREE)
        return result;

    vector<vector<int>> dist(
        chipX + 1, vector<int>(chipY + 1, INT_MAX));

    vector<vector<vector<Point>>> parents(
        chipX + 1, vector<vector<Point>>(chipY + 1));

    queue<Point> q;

    dist[net.start.x][net.start.y] = 0;
    q.push(net.start);

    while (!q.empty())
    {
        Point cur = q.front();
        q.pop();

        for (int d = 0; d < 4; d++)
        {
            int nx = cur.x + dx[d];
            int ny = cur.y + dy[d];

            if (!inBounds(nx, ny) || grid[nx][ny] != FREE)
                continue;
            if (reserved[nx][ny] && !(nx == net.end.x && ny == net.end.y))
                continue;

            int newDist = dist[cur.x][cur.y] + 1;

            if (newDist < dist[nx][ny])
            {
                dist[nx][ny] = newDist;
                parents[nx][ny].clear();
                parents[nx][ny].push_back(cur);
                q.push({nx, ny});
            }
            else if (newDist == dist[nx][ny])
            {
                bool found = false;
                for (const Point &p : parents[nx][ny])
                {
                    if (p.x == cur.x && p.y == cur.y)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    parents[nx][ny].push_back(cur);
                }
            }
        }
    }

    if (dist[net.end.x][net.end.y] == INT_MAX)
        return result;

    static random_device rd;
    static mt19937 gen(rd());

    vector<Point> path = reconstructRandomPath(net.start, net.end, parents, gen);

    result.success = true;
    result.path = path;
    result.length = static_cast<int>(path.size()) - 1;
    result.bends = countBends(path);
    blockPath(path);
    return result;
}

// Reconstructs a random shortest path from end to start using the parent map.
// At each step, randomly selects one of the parent candidates.
// Returns the full path from start to end in correct order.
vector<Point> Router::reconstructRandomPath(const Point &start, const Point &end,
                                            const vector<vector<vector<Point>>> &parents, mt19937 &gen)
{

    vector<Point> path;
    Point cur = end;

    while (!(cur.x == start.x && cur.y == start.y))
    {
        path.push_back(cur);

        if (parents[cur.x][cur.y].empty())
            break;

        uniform_int_distribution<> dis(0, parents[cur.x][cur.y].size() - 1);
        int randomIndex = dis(gen);
        cur = parents[cur.x][cur.y][randomIndex];
    }

    path.push_back(start);
    reverse(path.begin(), path.end());
    return path;
}

// Computes the routing difficulty score for a given connection:
// score = α × Manhattan distance + β × (spatial congestion) + γ × (edge proximity)
double Router::getScore(const Interconnection &net, double alpha, double beta, double gamma)
{
    int manhattan = abs(net.start.x - net.end.x) + abs(net.start.y - net.end.y);
    int freeAroundSource = countFreeNeighbors(net.start);
    int freeAroundTarget = countFreeNeighbors(net.end);
    // Assuming a maximum 3×3 neighborhood, excluding the center cell, yields 8 adjacent cells
    int spaceDifficulty = (8 - freeAroundSource) + (8 - freeAroundTarget);
    int edgeDifficulty = (3 - edgeProximity(net.start)) + (3 - edgeProximity(net.end));
    if (edgeDifficulty < 0)
        edgeDifficulty = 0;
    return alpha * manhattan + beta * spaceDifficulty + gamma * edgeDifficulty;
}

// Count the number of FREE cells in the 8-neighborhood around point p
int Router::countFreeNeighbors(const Point &p)
{
    int count = 0;
    for (int dx = -1; dx <= 1; dx++)
    {
        for (int dy = -1; dy <= 1; dy++)
        {
            if (dx == 0 && dy == 0)
                continue;
            int nx = p.x + dx;
            int ny = p.y + dy;
            if (inBounds(nx, ny))
            {
                if (grid[nx][ny] == FREE)
                    count++;
            }
        }
    }
    return count;
}

// Compute the distance from point p to the nearest boundary
int Router::edgeProximity(const Point &p)
{
    int d1 = p.x - 1;
    int d2 = chipX - p.x;
    int d3 = p.y - 1;
    int d4 = chipY - p.y;
    int min_d = min({d1, d2, d3, d4});
    return min_d;
}

bool Router::inBounds(int x, int y) const
{
    return x >= 1 && y >= 1 && x <= chipX && y <= chipY;
}

int Router::countBends(const vector<Point> &path)
{
    if (path.size() < 2)
        return 0;
    int bends = 0;
    int dx1 = path[1].x - path[0].x;
    int dy1 = path[1].y - path[0].y;

    for (size_t i = 2; i < path.size(); ++i)
    {
        int dx2 = path[i].x - path[i - 1].x;
        int dy2 = path[i].y - path[i - 1].y;
        if (dx1 != dx2 || dy1 != dy2)
            bends++;
        dx1 = dx2;
        dy1 = dy2;
    }
    return bends;
}

void Router::blockPath(const vector<Point> &path)
{
    for (const auto &pt : path)
        grid[pt.x][pt.y] = BLOCKED;
}
