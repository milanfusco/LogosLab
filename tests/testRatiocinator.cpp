#include "Ratiocinator.h"
#include <iostream>
#include <cassert>
#include <string>

// Use paths relative to the project root (where tests are run from)
#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR ".."
#endif

const std::string assumptionsFile = std::string(TEST_DATA_DIR) + "/assumptions_file.txt";
const std::string factsFile = std::string(TEST_DATA_DIR) + "/facts_file.txt";

// Test: parseAssumptionsFile correctly parses 'implies' relations
void testParseImpliesRelation() {
    std::cout << "Running testParseImpliesRelation..." << std::endl;
    
    Ratiocinator rationator;
    rationator.parseAssumptionsFile(assumptionsFile);

    // Verify 'implies' relation was parsed correctly (stored by consequent)
    // p, implies(light-from-galaxies, red-shifted, universe, expanding)
    auto& prop = rationator.propositions["universe"];
    assert(prop.getRelation() == LogicalOperator::IMPLIES);
    assert(prop.getAntecedent() == "light-from-galaxies");
    assert(prop.getSubject() == "red-shifted");
    assert(prop.getConsequent() == "universe");
    assert(prop.getPredicate() == "expanding");
    assert(prop.getPropositionScope() == Quantifier::UNIVERSAL_AFFIRMATIVE);

    std::cout << "Test passed: implies relation parsed correctly." << std::endl;
}

// Test: parseAssumptionsFile correctly parses 'some' relations
void testParseSomeRelation() {
    std::cout << "Running testParseSomeRelation..." << std::endl;

    Ratiocinator rationator;
    rationator.parseAssumptionsFile(assumptionsFile);

    // Verify 'some' relation was parsed
    // m, some(microwave-radiation, explosion-residue)
    // Note: 'some' stores by subject (parts[0]) and sets truth to TRUE
    auto& prop = rationator.propositions["microwave-radiation"];
    assert(prop.getTruthValue() == Tripartite::TRUE);
    assert(prop.getPropositionScope() == Quantifier::PARTICULAR_AFFIRMATIVE);
    assert(prop.getSubject() == "microwave-radiation");
    assert(prop.getPredicate() == "explosion-residue");
    
    std::cout << "Test passed: some relation parsed correctly." << std::endl;
}

// Test: parseAssumptionsFile correctly parses 'not' relations
void testParseNotRelation() {
    std::cout << "Running testParseNotRelation..." << std::endl;

    Ratiocinator rationator;
    rationator.parseAssumptionsFile(assumptionsFile);

    // Verify 'not' relation was parsed and sets truth to FALSE
    // q, not(galaxy-formation)
    auto& prop = rationator.propositions["galaxy-formation"];
    assert(prop.getRelation() == LogicalOperator::NOT);
    assert(prop.getTruthValue() == Tripartite::FALSE);
    assert(prop.getPropositionScope() == Quantifier::UNIVERSAL_NEGATIVE);

    std::cout << "Test passed: not relation parsed correctly." << std::endl;
}

// Test: parseAssumptionsFile correctly parses 'discovered' relations
void testParseDiscoveredRelation() {
    std::cout << "Running testParseDiscoveredRelation..." << std::endl;

    Ratiocinator rationator;
    rationator.parseAssumptionsFile(assumptionsFile);

    // Verify 'discovered' relation was parsed
    // t, discovered(WMAP, 999-millimeter-radiation)
    auto& prop = rationator.propositions["WMAP"];
    assert(prop.getSubject() == "WMAP");
    assert(prop.getPredicate() == "999-millimeter-radiation");

    std::cout << "Test passed: discovered relation parsed correctly." << std::endl;
}

// Test: parseFactsFile correctly sets truth values
void testParseFactsFile() {
    std::cout << "Running testParseFactsFile..." << std::endl;

    Ratiocinator rationator;
    rationator.parseAssumptionsFile(assumptionsFile);
    rationator.parseFactsFile(factsFile);

    // Facts file contains: !q, p && n, !r, t = p && n, s = t || r, u = t && s
    // parseFactsFile sets truth values based on tokens
    // !q sets q to FALSE, p sets p to TRUE, n sets n to TRUE, etc.
    
    assert(rationator.propositions["p"].getTruthValue() == Tripartite::TRUE);
    assert(rationator.propositions["n"].getTruthValue() == Tripartite::TRUE);

    std::cout << "Test passed: facts file parsed correctly." << std::endl;
}

// ============================================================
// INFERENCE TESTS - The brain of the Ratiocinator
// ============================================================

// Test: Modus Ponens - P → Q, P is TRUE ⊢ Q is TRUE
void testModusPonens() {
    std::cout << "Running testModusPonens..." << std::endl;

    Ratiocinator rationator;
    rationator.parseAssumptionsFile(assumptionsFile);
    
    // Assumptions file has: p, implies(light-from-galaxies, red-shifted, universe, expanding)
    // This means: light-from-galaxies → universe
    
    // Initially, universe should be UNKNOWN
    assert(rationator.propositions["universe"].getTruthValue() == Tripartite::UNKNOWN);
    
    // Set the antecedent to TRUE
    rationator.propositions["light-from-galaxies"].setTruthValue(Tripartite::TRUE);
    
    // Run deduction - Modus Ponens should fire
    rationator.deduceAll();
    
    // Now universe should be TRUE (because light-from-galaxies is TRUE)
    assert(rationator.propositions["universe"].getTruthValue() == Tripartite::TRUE);
    
    std::cout << "Test passed: Modus Ponens correctly infers consequent." << std::endl;
}

// Test: Modus Tollens - P → Q, Q is FALSE ⊢ P is FALSE
void testModusTollens() {
    std::cout << "Running testModusTollens..." << std::endl;

    Ratiocinator rationator;
    rationator.parseAssumptionsFile(assumptionsFile);
    
    // Assumptions file has: p, implies(light-from-galaxies, red-shifted, universe, expanding)
    // This means: light-from-galaxies → universe
    
    // Initially, light-from-galaxies should be UNKNOWN
    assert(rationator.propositions["light-from-galaxies"].getTruthValue() == Tripartite::UNKNOWN);
    
    // Set the consequent to FALSE
    rationator.propositions["universe"].setTruthValue(Tripartite::FALSE);
    
    // Run deduction - Modus Tollens should fire
    rationator.deduceAll();
    
    // Now light-from-galaxies should be FALSE (because universe is FALSE)
    assert(rationator.propositions["light-from-galaxies"].getTruthValue() == Tripartite::FALSE);
    
    std::cout << "Test passed: Modus Tollens correctly infers antecedent is false." << std::endl;
}

// Test: Chained inference - A → B, B → C, A is TRUE ⊢ C is TRUE
void testChainedInference() {
    std::cout << "Running testChainedInference..." << std::endl;

    Ratiocinator rationator;
    
    // Manually create a chain: A → B → C
    Proposition impAB;
    impAB.setRelation(LogicalOperator::IMPLIES);
    impAB.setAntecedent("A");
    impAB.setConsequent("B");
    rationator.propositions["B"] = impAB;
    
    Proposition impBC;
    impBC.setRelation(LogicalOperator::IMPLIES);
    impBC.setAntecedent("B");
    impBC.setConsequent("C");
    rationator.propositions["C"] = impBC;
    
    // Set A to TRUE
    rationator.propositions["A"].setTruthValue(Tripartite::TRUE);
    
    // Initially B and C should be UNKNOWN
    assert(rationator.propositions["B"].getTruthValue() == Tripartite::UNKNOWN);
    assert(rationator.propositions["C"].getTruthValue() == Tripartite::UNKNOWN);
    
    // Run deduction - should chain: A→B fires, then B→C fires
    rationator.deduceAll();
    
    // Both B and C should now be TRUE
    assert(rationator.propositions["B"].getTruthValue() == Tripartite::TRUE);
    assert(rationator.propositions["C"].getTruthValue() == Tripartite::TRUE);
    
    std::cout << "Test passed: Chained inference propagates correctly." << std::endl;
}

// Test: No inference when antecedent is UNKNOWN
void testNoInferenceOnUnknown() {
    std::cout << "Running testNoInferenceOnUnknown..." << std::endl;

    Ratiocinator rationator;
    rationator.parseAssumptionsFile(assumptionsFile);
    
    // Don't set anything - all should remain UNKNOWN
    rationator.deduceAll();
    
    // universe should still be UNKNOWN (no evidence either way)
    assert(rationator.propositions["universe"].getTruthValue() == Tripartite::UNKNOWN);
    
    std::cout << "Test passed: No spurious inference on UNKNOWN values." << std::endl;
}

// Main function to run all tests
int main() {
    // Parsing tests
    testParseImpliesRelation();
    testParseSomeRelation();
    testParseNotRelation();
    testParseDiscoveredRelation();
    testParseFactsFile();
    
    // Inference tests (THE IMPORTANT ONES!)
    testModusPonens();
    testModusTollens();
    testChainedInference();
    testNoInferenceOnUnknown();

    std::cout << "\n All Ratiocinator tests passed successfully!" << std::endl;
    return 0;
}
