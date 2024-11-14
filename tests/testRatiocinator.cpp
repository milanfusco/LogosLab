#include "Ratiocinator.h"
#include <iostream>
#include <cassert>
#include <string>

const std::string assumptionsFile = "/Users/milanfusco/Documents/repositories/LogosLab/assumptions_file.txt";
const std::string factsFile = "/Users/milanfusco/Documents/repositories/LogosLab/facts_file.txt";

// Test function to parse assumptions and verify initial conditions
void testParseAssumptions() {
    std::cout << "Running testParseAssumptions..." << std::endl;
    
    // Create a fresh Ratiocinator instance for each test
    Ratiocinator rationator;
    rationator.parseFactsFile(assumptionsFile);

    // Set an initial condition: "big-bang" is assumed to be true
    rationator.propositions["big-bang"].setTruthValue(Tripartite::TRUE);

    // Deduce values based on assumptions
    rationator.deduceAll();

    // Assertion: Expect "microwave-radiation" to be true because "big-bang implies microwave-radiation"
    assert(rationator.propositions["microwave-radiation"].getTruthValue() == Tripartite::TRUE);
    std::cout << "Test passed: big-bang implies microwave-radiation." << std::endl;
}

// Test function for universe expansion implication
void testUniverseExpansionImplication() {
    std::cout << "Running testUniverseExpansionImplication..." << std::endl;

    Ratiocinator rationator;
    rationator.parseFactsFile(assumptionsFile);

    // Set initial conditions based on known facts
    rationator.propositions["light-from-galaxies"].setTruthValue(Tripartite::TRUE);
    rationator.propositions["red-shifted"].setTruthValue(Tripartite::TRUE);

    // Deduce values based on new facts
    rationator.deduceAll();

    // Assertion: Expect "universe" to be true because "light-from-galaxies and red-shifted imply universe expanding"
    assert(rationator.propositions["universe"].getTruthValue() == Tripartite::TRUE);
    std::cout << "Test passed: light-from-galaxies and red-shifted imply universe expansion." << std::endl;
}

// Test function for negation on galaxy formation explanation
void testGalaxyFormationNegation() {
    std::cout << "Running testGalaxyFormationNegation..." << std::endl;

    Ratiocinator rationator;
    rationator.parseFactsFile(assumptionsFile);

    // Assertion: Expect "galaxy-formation" to be false because "not explains(big-bang, galaxy-formation)"
    assert(rationator.propositions["galaxy-formation"].getTruthValue() == Tripartite::FALSE);
    std::cout << "Test passed: big-bang does not explain galaxy formation." << std::endl;
}

// Test function for thermal-dynamics-law uncertainty
void testThermalDynamicsLawUncertainty() {
    std::cout << "Running testThermalDynamicsLawUncertainty..." << std::endl;

    Ratiocinator rationator;
    rationator.parseFactsFile(assumptionsFile);

    // Assertion: Expect "thermal-dynamics-law" to be unknown based on current information
    assert(rationator.propositions["thermal-dynamics-law"].getTruthValue() == Tripartite::UNKNOWN);
    std::cout << "Test passed: thermal-dynamics-law remains unknown." << std::endl;
}

// Test function to parse and evaluate facts
void testParseAndEvaluateFacts() {
    std::cout << "Running testParseAndEvaluateFacts..." << std::endl;

    Ratiocinator rationator;
    rationator.parseAssumptionsFile(assumptionsFile);
    rationator.parseFactsFile(factsFile);

    // Run deduction based on parsed facts
    rationator.deduceAll();

    // Verify combined deductions with intermediate variables
    assert(rationator.propositions["t"].getTruthValue() == Tripartite::TRUE);
    assert(rationator.propositions["s"].getTruthValue() == Tripartite::TRUE);
    assert(rationator.propositions["u"].getTruthValue() == Tripartite::TRUE);

    std::cout << "Test passed: Facts file parsed and evaluated with expected truth values for t, s, and u." << std::endl;
}

// Main function to run all tests
int main() {
    testParseAssumptions();
    testUniverseExpansionImplication();
    testGalaxyFormationNegation();
    testThermalDynamicsLawUncertainty();
    testParseAndEvaluateFacts();

    std::cout << "All tests passed successfully." << std::endl;
    return 0;
}
