#include "Ratiocinator.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    // Check if the correct number of arguments is provided
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    // Get the input file path from command-line arguments
    std::string inputFile = argv[1];

    // Create an instance of Ratiocinator
    Ratiocinator rationator;

    // Parse the input file for facts and assumptions
    std::cout << "Parsing input file: " << inputFile << std::endl;
    rationator.parseFactsFile(inputFile);

    // Deduce truth values of all propositions based on loaded expressions
    std::cout << "Deducting truth values based on loaded expressions..." << std::endl;
    rationator.deduceAll();

    // Output final truth values for each proposition
    std::cout << "Final truth values of propositions:" << std::endl;
    rationator.outputTruthValues();

    return 0;
}