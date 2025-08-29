#include "solver.h"
#include "router.h"
#include <iostream>
#include <random>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <climits>
#include <tuple>

using namespace std;
using namespace chrono;
#define mode 0

ChipRouterSolver::ChipRouterSolver(ifstream &fin)
{
    parseInput(fin);
}

void ChipRouterSolver::solve()
{
    if (mode == 0)
    {
        int bestRouted = -1;
        vector<RoutingResult> bestResults;
        int bestTotalLength = 0, bestTotalBends = 0;
        int bestLongestIndex = -1, bestLongestLength = 0;

        auto startTime = steady_clock::now();

        int noImprovementCount = 0;
        vector<int> netOrder(numInterconnections);
        iota(netOrder.begin(), netOrder.end(), 0);

        random_device rd;
        mt19937 rng(rd());

        int NUM_ATTEMPTS = 10000 * numInterconnections;
        bool earlyflag = false;

        for (int attempt = 1; attempt <= NUM_ATTEMPTS; ++attempt)
        {
            Router router(chipX, chipY);

            // Add blockages
            for (const auto &b : blockages)
            {
                router.addBlockage(b.x1, b.y1, b.x2, b.y2);
            }
            router.setReserved(nets);

            // Random shuffle routing net order
            shuffle(netOrder.begin(), netOrder.end(), rng);

            // Routing
            vector<RoutingResult> results(numInterconnections);
            int routedCount = 0, totalLength = 0, totalBends = 0;
            int longestLength = 0;

            for (int i : netOrder)
            {
                results[i] = router.route(nets[i]);
                if (results[i].success)
                {
                    routedCount++;
                    totalLength += results[i].length;
                    totalBends += results[i].bends;
                    longestLength = max(longestLength, results[i].length);
                }
            }

            auto currentTuple = make_tuple(-routedCount, totalLength, longestLength, totalBends);
            auto bestTuple = make_tuple(-bestRouted, bestTotalLength, bestLongestLength, bestTotalBends);

            if (currentTuple < bestTuple)
            {
                bestResults = results;
                bestRouted = routedCount;
                bestTotalLength = totalLength;
                bestTotalBends = totalBends;
                bestLongestLength = longestLength;

                bestLongestIndex = -1;
                for (int i = 0; i < numInterconnections; ++i)
                {
                    if (results[i].success && results[i].length == longestLength)
                    {
                        bestLongestIndex = i;
                        break;
                    }
                }

                noImprovementCount = 0;
            }
            else
            {
                noImprovementCount++;
            }

            // Early stop
            if (noImprovementCount >= numInterconnections * 1000)
            {
                earlyflag = true;
                break;
            }

            // Time limit
            if (steady_clock::now() - startTime > minutes(19))
                break;
        }

        cout << "Mode = 0(Shuffle)" << endl;

        cout << (earlyflag ? "Early stop" : "Finish")
             << " at "
             << (earlyflag ? noImprovementCount : NUM_ATTEMPTS)
             << " attempts"
             << endl;

        cout << "====== Final Result ======" << endl;
        cout << "Final Routed = " << bestRouted << endl;
        cout << "Final TotalLength = " << bestTotalLength << endl;
        cout << "Final LongestLength = " << bestLongestLength << endl;
        cout << "Final TotalBends = " << bestTotalBends << endl;

        saveBestResult(bestResults, bestRouted, bestTotalLength, bestTotalBends, bestLongestLength, bestLongestIndex);
    }

    else
    {
        Router router(chipX, chipY);
        for (const auto &b : blockages)
        {
            router.addBlockage(b.x1, b.y1, b.x2, b.y2);
        }

        // Calculate order score
        // score = alpha * Manhattan_distance + beta * (not enough space) + gamma * (close to edge)
        double alpha = 1.5, beta = 1.0, gamma = 0.5;
        vector<pair<double, int>> scores;
        for (int i = 0; i < numInterconnections; i++)
        {
            double s = router.getScore(nets[i], alpha, beta, gamma);
            scores.push_back({s, i});
        }

        // higher the first
        sort(scores.begin(), scores.end(),
             [](const pair<double, int> &a, const pair<double, int> &b)
             { return a.first > b.first; });

        router.setReserved(nets);

        vector<RoutingResult> results(numInterconnections);
        int routedCount = 0, totalLength = 0, totalBends = 0;
        int longestIndex = -1, longestLength = 0;
        // Routing according to net order
        for (auto &entry : scores)
        {
            int i = entry.second;
            results[i] = router.route(nets[i]);
            if (results[i].success)
            {
                routedCount++;
                totalLength += results[i].length;
                totalBends += results[i].bends;
                if (results[i].length > longestLength)
                {
                    longestLength = results[i].length;
                    longestIndex = i;
                }
            }
        }
        cout << "Mode 1(Weighted) alpha = " << alpha << ", beta = " << beta
             << ", gamma = " << gamma << endl;
        cout << "====== Final Result ======" << endl;
        cout << "Final Routed = " << routedCount << endl;
        cout << "Final TotalLength = " << totalLength << endl;
        cout << "Final LongestLength = " << longestLength << endl;
        cout << "Final TotalBends = " << totalBends << endl;

        saveBestResult(results, routedCount, totalLength, totalBends,
                       longestLength, longestIndex);
    }
}

void ChipRouterSolver::parseInput(ifstream &fin)
{
    if (!(fin >> chipX >> chipY))
        throw runtime_error("Invalid chip dimension input.");

    string header;
    fin >> header;
    if (header != "#blockages")
        throw runtime_error("Expected \"#blockages\" header.");

    int numBlockages;
    if (!(fin >> numBlockages))
        throw runtime_error("Invalid blockages count input.");

    blockages.resize(numBlockages);
    for (int i = 0; i < numBlockages; i++)
    {
        if (!(fin >> blockages[i].x1 >> blockages[i].y1 >> blockages[i].x2 >> blockages[i].y2))
            throw runtime_error("Invalid blockage data.");
    }

    fin >> header;
    if (header != "#interconnections")
        throw runtime_error("Expected \"#interconnections\" header.");

    if (!(fin >> numInterconnections))
        throw runtime_error("Invalid interconnections count.");

    nets.resize(numInterconnections);
    for (int i = 0; i < numInterconnections; i++)
    {
        if (!(fin >> nets[i].start.x >> nets[i].start.y >> nets[i].end.x >> nets[i].end.y))
            throw runtime_error("Invalid interconnection data.");
    }
}

void ChipRouterSolver::writeReport(ofstream &fout)
{
    fout << "#interconnections routed = " << FinalRouted << endl;
    fout << "Total interconnection length = " << FinalTotalLength << "\n";

    if (FinalLongestIndex >= 0)
        fout << "The longest interconnection = " << (FinalLongestIndex + 1)
             << "; length = " << FinalLongestLength << "\n";
    else
        fout << "The longest interconnection = N/A; length = 0\n";

    fout << "Total number of bends = " << FinalTotalBends << "\n";

    for (size_t i = 0; i < FinalResults.size(); i++)
    {
        fout << "Interconnection " << (i + 1) << ": ";
        if (!FinalResults[i].success)
        {
            fout << "fails.\n";
        }
        else
        {
            fout << "length = " << FinalResults[i].length
                 << ", #bends = " << FinalResults[i].bends << "\n";

            for (size_t j = 0; j < FinalResults[i].path.size(); j++)
            {
                const Point &pt = FinalResults[i].path[j];
                fout << "(" << pt.x << ", " << pt.y << ")";
                if (j + 1 != FinalResults[i].path.size())
                    fout << ", ";
            }
            fout << "\n";
        }
    }
}

void ChipRouterSolver::saveBestResult(const vector<RoutingResult> &results,
                                      int routedCount, int totalLength,
                                      int totalBends, int longestLength, int longestIndex)
{
    FinalResults = results;
    FinalRouted = routedCount;
    FinalTotalLength = totalLength;
    FinalTotalBends = totalBends;
    FinalLongestLength = longestLength;
    FinalLongestIndex = longestIndex;
}
