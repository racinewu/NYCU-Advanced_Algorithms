#pragma once

#include "types.h"
#include <fstream>
#include <vector>

class ChipRouterSolver
{
public:
    ChipRouterSolver(std::ifstream &fin);

    void solve();
    void writeReport(std::ofstream &fout);

private:
    void parseInput(std::ifstream &fin);
    void saveBestResult(const std::vector<RoutingResult> &results, int routedCount, int totalLength, int totalBends, int longestLength, int longestIndex);

    int chipX, chipY;
    int numInterconnections;

    std::vector<Blockage> blockages;
    std::vector<Interconnection> nets;

    std::vector<RoutingResult> FinalResults;
    int FinalRouted = -1;
    int FinalTotalLength = 0;
    int FinalTotalBends = 0;
    int FinalLongestLength = 0;
    int FinalLongestIndex = -1;
};
