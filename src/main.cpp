#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "Ratiocinator.h"

namespace {
    constexpr const char* REPORT_FILENAME = "ratiocinator_report.txt";
    
    void printUsage(const char* programName) {
        std::cerr << "Usage: " << programName << " [OPTIONS] <assumptions_file> <facts_file>\n"
                  << "\nOptions:\n"
                  << "  --traces    Include inference traces in output\n"
                  << "  --help      Show this help message\n"
                  << "\nExample:\n"
                  << "  " << programName << " assumptions.txt facts.txt\n"
                  << "  " << programName << " --traces assumptions.txt facts.txt\n";
    }
}

int main(int argc, char* argv[]) {
    bool showTraces = false;
    int fileArgStart = 1;
    
    // Parse options
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--traces") == 0) {
            showTraces = true;
            fileArgStart = i + 1;
        } else if (std::strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (argv[i][0] == '-') {
            std::cerr << "Unknown option: " << argv[i] << "\n";
            printUsage(argv[0]);
            return 1;
        } else {
            break;
        }
    }
    
    // Check for required file arguments
    if (argc - fileArgStart < 2) {
        std::cerr << "Error: Missing required file arguments.\n\n";
        printUsage(argv[0]);
        return 1;
    }

    // Get the file paths from command-line arguments
    std::string assumptionsFile = argv[fileArgStart];
    std::string factsFile = argv[fileArgStart + 1];

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

    reportFile << engine.formatResults(showTraces);

    std::cout << "Results written to " << REPORT_FILENAME << std::endl;
    
    // Also print to console if traces are requested
    if (showTraces) {
        std::cout << "\n";
        engine.printResults(true);
    }

    return 0;
}