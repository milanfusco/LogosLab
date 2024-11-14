#include <iostream>
#include <string>
#include "Ratiocinator.h"

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: ./LogosLab "
                "/path/to/<assumptions_file>.txt /path/to/<facts_file>.txt"
              << std::endl;
    return 1;
  }

  // Get the file paths from command-line arguments
  std::string assumptionsFile = argv[1];
  std::string factsFile = argv[2];

  // Create an instance of Ratiocinator
  Ratiocinator rationator;

  // Parse the assumptions file
  std::cout << "Parsing assumptions file: " << assumptionsFile << std::endl;
  rationator.parseAssumptionsFile(assumptionsFile);

  // Parse the facts file
  std::cout << "Parsing facts file: " << factsFile << std::endl;
  rationator.parseFactsFile(factsFile);

  // Deduce truth values of all propositions based on loaded expressions
  std::cout << "Deducting truth values based on loaded expressions..."
            << std::endl;
  rationator.deduceAll();

  // Output final truth values for each proposition
  std::cout << "Final truth values of propositions:" << std::endl;
  rationator.outputTruthValues();

  return 0;
}