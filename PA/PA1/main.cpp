#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>

using namespace std;

int compute(int i, int j, const vector<int> &pairIndex, vector<vector<int>> &dp)
{
    if (i >= j)
        return 0;
    if (dp[i][j] != -1)
        return dp[i][j];

    int k = pairIndex[j];
    if (k < i || k > j)
        return dp[i][j] = compute(i, j - 1, pairIndex, dp);
    else if (k == i)
        return dp[i][j] = compute(i + 1, j - 1, pairIndex, dp) + 1;
    else
        return dp[i][j] = max(
                   compute(i, j - 1, pairIndex, dp),
                   compute(i, k - 1, pairIndex, dp) + compute(k + 1, j - 1, pairIndex, dp) + 1);
}

void recover(int i, int j, const vector<int> &pairIndex,
             const vector<vector<int>> &dp, vector<pair<int, int>> &result)
{
    if (i >= j)
        return;

    int k = pairIndex[j];
    if (k < i || k > j)
    {
        recover(i, j - 1, pairIndex, dp, result);
    }
    else if (k == i)
    {
        result.push_back({i, j});
        recover(i + 1, j - 1, pairIndex, dp, result);
    }
    else
    {
        int exclude = compute(i, j - 1, pairIndex, const_cast<vector<vector<int>> &>(dp));
        int include = compute(i, k - 1, pairIndex, const_cast<vector<vector<int>> &>(dp)) +
                      compute(k + 1, j - 1, pairIndex, const_cast<vector<vector<int>> &>(dp)) + 1;
        if (include > exclude)
        {
            result.push_back({k, j});
            recover(i, k - 1, pairIndex, dp, result);
            recover(k + 1, j - 1, pairIndex, dp, result);
        }
        else
        {
            recover(i, j - 1, pairIndex, dp, result);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cerr << "Usage: ./program <input_file> <output_file>" << endl;
        return 1;
    }

    string inputFile = argv[1];
    string outputFile = argv[2];

    ifstream infile(inputFile);
    if (!infile)
    {
        cerr << "Failed to open " << inputFile << '\n';
        return 1;
    }

    int maxIndex;
    infile >> maxIndex;

    vector<int> pairIndex(maxIndex, -1);
    vector<vector<int>> dp(maxIndex, vector<int>(maxIndex, -1));

    int a, b;
    while (infile >> a >> b)
    {
        if (a == 0 && b == 0)
            break;
        pairIndex[a] = b;
        pairIndex[b] = a;
    }
    infile.close();

    int result = compute(0, maxIndex - 1, pairIndex, dp);

    ofstream outfile(outputFile);
    if (!outfile)
    {
        cerr << "Failed to open " << outputFile << '\n';
        return 1;
    }
    outfile << result << endl;

    vector<pair<int, int>> selectedPairs;
    recover(0, maxIndex - 1, pairIndex, dp, selectedPairs);

    sort(selectedPairs.begin(), selectedPairs.end());
    for (auto &p : selectedPairs)
        outfile << p.first << " " << p.second << endl;
    outfile.close();

    return 0;
}
