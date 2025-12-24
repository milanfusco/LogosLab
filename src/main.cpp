#include <iostream>
#include <fstream>
#include <string>
#include "Ratiocinator.h"

namespace {
    constexpr int EXPECTED_ARGS = 3;
    constexpr const char* REPORT_FILENAME = "ratiocinator_report.txt";
}

int main(int argc, char* argv[]) {
    if (argc != EXPECTED_ARGS) {
        std::cerr << "Usage: ./LogosLab "
                     "/path/to/<assumptions_file>.txt /path/to/<facts_file>.txt"
                  << std::endl;
        return 1;
    }

    // Get the file paths from command-line arguments
    std::string assumptionsFile = argv[1];
    std::string factsFile = argv[2];

    // Create an instance of Ratiocinator
    Ratiocinator engine;

    // Load assumptions
    std::cout << "Loading assumptions: " << assumptionsFile << std::endl;
    engine.loadAssumptions(assumptionsFile);

    // Load facts
    std::cout << "Loading facts: " << factsFile << std::endl;
    engine.loadFacts(factsFile);

    // Deduce truth values
    std::cout << "Deducing truth values..." << std::endl;
    engine.deduce();

    // Write report
    std::ofstream reportFile(REPORT_FILENAME);
    if (!reportFile) {
        std::cerr << "Error: Could not open " << REPORT_FILENAME << " for writing." << std::endl;
        return 1;
    }

    reportFile << "Final truth values of propositions:\n";
    reportFile << engine.formatResults();

    std::cout << "Results written to " << REPORT_FILENAME << std::endl;

    return 0;
}