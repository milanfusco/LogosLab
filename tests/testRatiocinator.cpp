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
    const Proposition* prop = rationator.getProposition("universe");
    assert(prop != nullptr);
    assert(prop->getRelation() == LogicalOperator::IMPLIES);
    assert(prop->getAntecedent() == "light-from-galaxies");
    assert(prop->getSubject() == "red-shifted");
    assert(prop->getConsequent() == "universe");
    assert(prop->getPredicate() == "expanding");
    assert(prop->getPropositionScope() == Quantifier::UNIVERSAL_AFFIRMATIVE);

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
    const Proposition* prop = rationator.getProposition("microwave-radiation");
    assert(prop != nullptr);
    assert(rationator.getPropositionTruthValue("microwave-radiation") == Tripartite::TRUE);
    assert(prop->getPropositionScope() == Quantifier::PARTICULAR_AFFIRMATIVE);
    assert(prop->getSubject() == "microwave-radiation");
    assert(prop->getPredicate() == "explosion-residue");
    
    std::cout << "Test passed: some relation parsed correctly." << std::endl;
}

// Test: parseAssumptionsFile correctly parses 'not' relations
void testParseNotRelation() {
    std::cout << "Running testParseNotRelation..." << std::endl;

    Ratiocinator rationator;
    rationator.parseAssumptionsFile(assumptionsFile);

    // Verify 'not' relation was parsed and sets truth to FALSE
    // q, not(galaxy-formation)
    const Proposition* prop = rationator.getProposition("galaxy-formation");
    assert(prop != nullptr);
    assert(prop->getRelation() == LogicalOperator::NOT);
    assert(rationator.getPropositionTruthValue("galaxy-formation") == Tripartite::FALSE);
    assert(prop->getPropositionScope() == Quantifier::UNIVERSAL_NEGATIVE);

    std::cout << "Test passed: not relation parsed correctly." << std::endl;
}

// Test: parseAssumptionsFile correctly parses 'discovered' relations
void testParseDiscoveredRelation() {
    std::cout << "Running testParseDiscoveredRelation..." << std::endl;

    Ratiocinator rationator;
    rationator.parseAssumptionsFile(assumptionsFile);

    // Verify 'discovered' relation was parsed
    // t, discovered(WMAP, 999-millimeter-radiation)
    const Proposition* prop = rationator.getProposition("WMAP");
    assert(prop != nullptr);
    assert(prop->getSubject() == "WMAP");
    assert(prop->getPredicate() == "999-millimeter-radiation");

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
    
    assert(rationator.getPropositionTruthValue("p") == Tripartite::TRUE);
    assert(rationator.getPropositionTruthValue("n") == Tripartite::TRUE);

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
    assert(rationator.getPropositionTruthValue("universe") == Tripartite::UNKNOWN);
    
    // Set the antecedent to TRUE
    rationator.setPropositionTruthValue("light-from-galaxies", Tripartite::TRUE);
    
    // Run deduction - Modus Ponens should fire
    rationator.deduceAll();
    
    // Now universe should be TRUE (because light-from-galaxies is TRUE)
    assert(rationator.getPropositionTruthValue("universe") == Tripartite::TRUE);
    
    std::cout << "Test passed: Modus Ponens correctly infers consequent." << std::endl;
}

// Test: Modus Tollens - P → Q, Q is FALSE ⊢ P is FALSE
void testModusTollens() {
    std::cout << "Running testModusTollens..." << std::endl;

    Ratiocinator rationator;
    rationator.parseAssumptionsFile(assumptionsFile);
    
    // Assumptions file has: p, implies(light-from-galaxies, red-shifted, universe, expanding)
    // This means: light-from-galaxies → universe
    
    // Initially, light-from-galaxies should be UNKNOWN (or not exist)
    assert(rationator.getPropositionTruthValue("light-from-galaxies") == Tripartite::UNKNOWN);
    
    // Set the consequent to FALSE
    rationator.setPropositionTruthValue("universe", Tripartite::FALSE);
    
    // Run deduction - Modus Tollens should fire
    rationator.deduceAll();
    
    // Now light-from-galaxies should be FALSE (because universe is FALSE)
    assert(rationator.getPropositionTruthValue("light-from-galaxies") == Tripartite::FALSE);
    
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
    rationator.setProposition("B", impAB);
    
    Proposition impBC;
    impBC.setRelation(LogicalOperator::IMPLIES);
    impBC.setAntecedent("B");
    impBC.setConsequent("C");
    rationator.setProposition("C", impBC);
    
    // Set A to TRUE
    rationator.setPropositionTruthValue("A", Tripartite::TRUE);
    
    // Initially B and C should be UNKNOWN
    assert(rationator.getPropositionTruthValue("B") == Tripartite::UNKNOWN);
    assert(rationator.getPropositionTruthValue("C") == Tripartite::UNKNOWN);
    
    // Run deduction - should chain: A→B fires, then B→C fires
    rationator.deduceAll();
    
    // Both B and C should now be TRUE
    assert(rationator.getPropositionTruthValue("B") == Tripartite::TRUE);
    assert(rationator.getPropositionTruthValue("C") == Tripartite::TRUE);
    
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
    assert(rationator.getPropositionTruthValue("universe") == Tripartite::UNKNOWN);
    
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
