#include <iostream>
#include "router.h"
#include "solver.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: ./bin/Chip_Router <input_file> <output_file>" << endl;
        return 1;
    }
    cout << "Reminder:\n - Set mode (0 or 1)\n - Adjust weights (alpha, beta, gamma) for mode 0\n\n";

    try
    {
        string inputFile = argv[1];
        string outputFile = argv[2];

        ifstream infile(inputFile);
        if (!infile)
        {
            cerr << "Failed to open input file: " << inputFile << endl;
            return 1;
        }

        ofstream outfile(outputFile);
        if (!outfile)
        {
            cerr << "Failed to open output file: " << outputFile << endl;
            return 1;
        }

        ChipRouterSolver router(infile);
        router.solve();
        router.writeReport(outfile);

        infile.close();
        outfile.close();

        return 0;
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}