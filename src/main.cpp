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
                  << "  --traces          Include inference traces in output\n"
                  << "  --true-only       Show only TRUE propositions\n"
                  << "  --false-only      Show only FALSE propositions\n"
                  << "  --known-only      Show only TRUE and FALSE propositions\n"
                  << "  --unknown-only    Show only UNKNOWN propositions\n"
                  << "  --derived-only    Show only derived propositions (not axioms)\n"
                  << "  --axioms-only     Show only axioms (not derived)\n"
                  << "  --prefix=PREFIX   Show only propositions starting with PREFIX\n"
                  << "  --contains=STR    Show only propositions containing STR\n"
                  << "  --limit=N         Show at most N results\n"
                  << "  --sort=ORDER      Sort results: alpha, alpha-desc, truth, derivation\n"
                  << "  --verbose         Print results to console as well as file\n"
                  << "  --help            Show this help message\n"
                  << "\nExamples:\n"
                  << "  " << programName << " assumptions.txt facts.txt\n"
                  << "  " << programName << " --traces --true-only assumptions.txt facts.txt\n"
                  << "  " << programName << " --prefix=user_ --sort=alpha assumptions.txt facts.txt\n";
    }
    
    bool startsWith(const char* str, const char* prefix) {
        return std::strncmp(str, prefix, std::strlen(prefix)) == 0;
    }
}

int main(int argc, char* argv[]) {
    ResultFilter filter;
    bool verbose = false;
    int fileArgStart = 1;
    
    // Parse options
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--traces") == 0) {
            filter.includeTraces = true;
        } else if (std::strcmp(argv[i], "--true-only") == 0) {
            filter.showTrue = true;
            filter.showFalse = false;
            filter.showUnknown = false;
        } else if (std::strcmp(argv[i], "--false-only") == 0) {
            filter.showTrue = false;
            filter.showFalse = true;
            filter.showUnknown = false;
        } else if (std::strcmp(argv[i], "--known-only") == 0) {
            filter.showTrue = true;
            filter.showFalse = true;
            filter.showUnknown = false;
        } else if (std::strcmp(argv[i], "--unknown-only") == 0) {
            filter.showTrue = false;
            filter.showFalse = false;
            filter.showUnknown = true;
        } else if (std::strcmp(argv[i], "--derived-only") == 0) {
            filter.showDerived = true;
            filter.showAxioms = false;
        } else if (std::strcmp(argv[i], "--axioms-only") == 0) {
            filter.showDerived = false;
            filter.showAxioms = true;
        } else if (startsWith(argv[i], "--prefix=")) {
            filter.prefixPattern = argv[i] + 9;
        } else if (startsWith(argv[i], "--contains=")) {
            filter.containsPattern = argv[i] + 11;
        } else if (startsWith(argv[i], "--limit=")) {
            filter.limit = std::stoul(argv[i] + 8);
        } else if (startsWith(argv[i], "--sort=")) {
            std::string sortOrder = argv[i] + 7;
            if (sortOrder == "alpha") {
                filter.sortOrder = ResultSortOrder::ALPHABETICAL;
            } else if (sortOrder == "alpha-desc") {
                filter.sortOrder = ResultSortOrder::ALPHABETICAL_DESC;
            } else if (sortOrder == "truth") {
                filter.sortOrder = ResultSortOrder::BY_TRUTH_VALUE;
            } else if (sortOrder == "derivation") {
                filter.sortOrder = ResultSortOrder::BY_DERIVATION;
            } else {
                std::cerr << "Unknown sort order: " << sortOrder << "\n";
                printUsage(argv[0]);
                return 1;
            }
        } else if (std::strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (std::strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (argv[i][0] == '-') {
            std::cerr << "Unknown option: " << argv[i] << "\n";
            printUsage(argv[0]);
            return 1;
        } else {
            fileArgStart = i;
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

    std::string results = engine.formatResults(filter);
    reportFile << results;

    std::cout << "Results written to " << REPORT_FILENAME << std::endl;
    
    // Print to console if verbose or traces requested
    if (verbose || filter.includeTraces) {
        std::cout << "\n" << results;
    }

    return 0;
}