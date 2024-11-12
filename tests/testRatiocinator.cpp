#include "Ratiocinator.h"
#include "Proposition.h"
#include "Expression.h"
#include <iostream>
#include <cassert>

int main() {
    // Path to the test file with predefined facts and assertions
    const std::string factsFile = "/Users/milanfusco/Documents/repositories/LogosLab/test_file.txt";

    // Create an instance of Ratiocinator
    Ratiocinator rationator;

    // Test: Parse the facts file in the expected format
    rationator.parseFactsFile(factsFile);

    // Assertions based on initial conditions from the parsed file
    // Setting some initial truth values to simulate real conditions if needed for deductions

    // Initial condition: big-bang is assumed true to drive implications
    rationator.propositions["big-bang"].setTruthValue(Tripartite::TRUE);

    // Deduce values based on parsed expressions
    rationator.deduceAll();

    // Assertions after deductions
    // 1. Expect microwave-radiation to be true because "big-bang implies microwave-radiation"
    assert(rationator.propositions["microwave-radiation"].getTruthValue() == Tripartite::TRUE);

    // 2. Check for `universe` truth value based on `light-from-galaxies` and `red-shifted`
    rationator.propositions["light-from-galaxies"].setTruthValue(Tripartite::TRUE);
    rationator.propositions["red-shifted"].setTruthValue(Tripartite::TRUE);
    rationator.deduceAll();
    assert(rationator.propositions["universe"].getTruthValue() == Tripartite::TRUE);

    // 3. Test for `galaxy-formation` as false based on "not(explains(big-bang, galaxy-formation))"
    assert(rationator.propositions["galaxy-formation"].getTruthValue() == Tripartite::FALSE);

    // 4. Validate deduction for thermal-dynamics-law, which might be UNKNOWN based on complex factors
    assert(rationator.propositions["thermal-dynamics-law"].getTruthValue() == Tripartite::UNKNOWN);

    // Output truth values after deductions for verification
    std::cout << "Final truth values of propositions:" << std::endl;
    rationator.outputTruthValues();

    std::cout << "All tests passed successfully." << std::endl;

    return 0;
}
