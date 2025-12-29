#include "Ratiocinator.h"
#include "Parser.h"
#include <iostream>
#include <cassert>
#include <string>

// Use paths relative to the project root (where tests are run from)
#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR ".."
#endif

const std::string assumptionsFile = std::string(TEST_DATA_DIR) + "/assumptions_file.txt";
const std::string factsFile = std::string(TEST_DATA_DIR) + "/facts_file.txt";

// Test: loadAssumptions correctly parses 'implies' relations
void testParseImpliesRelation() {
    std::cout << "Running testParseImpliesRelation..." << std::endl;
    
    Ratiocinator rationator;
    rationator.loadAssumptions(assumptionsFile);

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

// Test: loadAssumptions correctly parses 'some' relations
void testParseSomeRelation() {
    std::cout << "Running testParseSomeRelation..." << std::endl;

    Ratiocinator rationator;
    rationator.loadAssumptions(assumptionsFile);

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

// Test: loadAssumptions correctly parses 'not' relations
void testParseNotRelation() {
    std::cout << "Running testParseNotRelation..." << std::endl;

    Ratiocinator rationator;
    rationator.loadAssumptions(assumptionsFile);

    // Verify 'not' relation was parsed and sets truth to FALSE
    // q, not(galaxy-formation)
    const Proposition* prop = rationator.getProposition("galaxy-formation");
    assert(prop != nullptr);
    assert(prop->getRelation() == LogicalOperator::NOT);
    assert(rationator.getPropositionTruthValue("galaxy-formation") == Tripartite::FALSE);
    assert(prop->getPropositionScope() == Quantifier::UNIVERSAL_NEGATIVE);

    std::cout << "Test passed: not relation parsed correctly." << std::endl;
}

// Test: loadAssumptions correctly parses 'discovered' relations
void testParseDiscoveredRelation() {
    std::cout << "Running testParseDiscoveredRelation..." << std::endl;

    Ratiocinator rationator;
    rationator.loadAssumptions(assumptionsFile);

    // Verify 'discovered' relation was parsed
    // t, discovered(WMAP, 999-millimeter-radiation)
    const Proposition* prop = rationator.getProposition("WMAP");
    assert(prop != nullptr);
    assert(prop->getSubject() == "WMAP");
    assert(prop->getPredicate() == "999-millimeter-radiation");

    std::cout << "Test passed: discovered relation parsed correctly." << std::endl;
}

// Test: loadFacts correctly sets truth values
void testParseFactsFile() {
    std::cout << "Running testParseFactsFile..." << std::endl;

    Ratiocinator rationator;
    rationator.loadAssumptions(assumptionsFile);
    rationator.loadFacts(factsFile);

    // Facts file contains: !q, p && n, !r, t = p && n, s = t || r, u = t && s
    // loadFacts sets truth values based on tokens
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
    rationator.loadAssumptions(assumptionsFile);
    
    // Assumptions file has: p, implies(light-from-galaxies, red-shifted, universe, expanding)
    // This means: light-from-galaxies → universe
    
    // Initially, universe should be UNKNOWN
    assert(rationator.getPropositionTruthValue("universe") == Tripartite::UNKNOWN);
    
    // Set the antecedent to TRUE
    rationator.setPropositionTruthValue("light-from-galaxies", Tripartite::TRUE);
    
    // Run deduction - Modus Ponens should fire
    rationator.deduce();
    
    // Now universe should be TRUE (because light-from-galaxies is TRUE)
    assert(rationator.getPropositionTruthValue("universe") == Tripartite::TRUE);
    
    std::cout << "Test passed: Modus Ponens correctly infers consequent." << std::endl;
}

// Test: Modus Tollens - P → Q, Q is FALSE ⊢ P is FALSE
void testModusTollens() {
    std::cout << "Running testModusTollens..." << std::endl;

    Ratiocinator rationator;
    rationator.loadAssumptions(assumptionsFile);
    
    // Assumptions file has: p, implies(light-from-galaxies, red-shifted, universe, expanding)
    // This means: light-from-galaxies → universe
    
    // Initially, light-from-galaxies should be UNKNOWN (or not exist)
    assert(rationator.getPropositionTruthValue("light-from-galaxies") == Tripartite::UNKNOWN);
    
    // Set the consequent to FALSE
    rationator.setPropositionTruthValue("universe", Tripartite::FALSE);
    
    // Run deduction - Modus Tollens should fire
    rationator.deduce();
    
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
    rationator.deduce();
    
    // Both B and C should now be TRUE
    assert(rationator.getPropositionTruthValue("B") == Tripartite::TRUE);
    assert(rationator.getPropositionTruthValue("C") == Tripartite::TRUE);
    
    std::cout << "Test passed: Chained inference propagates correctly." << std::endl;
}

// Test: No inference when antecedent is UNKNOWN
void testNoInferenceOnUnknown() {
    std::cout << "Running testNoInferenceOnUnknown..." << std::endl;

    Ratiocinator rationator;
    rationator.loadAssumptions(assumptionsFile);
    
    // Don't set anything - all should remain UNKNOWN
    rationator.deduce();
    
    // universe should still be UNKNOWN (no evidence either way)
    assert(rationator.getPropositionTruthValue("universe") == Tripartite::UNKNOWN);
    
    std::cout << "Test passed: No spurious inference on UNKNOWN values." << std::endl;
}

// ============================================================
// EXTENDED INFERENCE TESTS - New rules
// ============================================================

// Test: Hypothetical Syllogism - P → Q, Q → R, P is TRUE ⊢ R is TRUE
void testHypotheticalSyllogism() {
    std::cout << "Running testHypotheticalSyllogism..." << std::endl;

    Ratiocinator rationator;
    
    // Create a chain: P → Q → R
    Proposition impPQ;
    impPQ.setPrefix("impl_PQ");
    impPQ.setRelation(LogicalOperator::IMPLIES);
    impPQ.setAntecedent("P");
    impPQ.setConsequent("Q");
    rationator.setProposition("Q", impPQ);
    
    Proposition impQR;
    impQR.setPrefix("impl_QR");
    impQR.setRelation(LogicalOperator::IMPLIES);
    impQR.setAntecedent("Q");
    impQR.setConsequent("R");
    rationator.setProposition("R", impQR);
    
    // Set P to TRUE
    rationator.setPropositionTruthValue("P", Tripartite::TRUE);
    
    // Initially Q and R should be UNKNOWN
    assert(rationator.getPropositionTruthValue("Q") == Tripartite::UNKNOWN);
    assert(rationator.getPropositionTruthValue("R") == Tripartite::UNKNOWN);
    
    // Run deduction - Hypothetical Syllogism should derive R directly from P
    rationator.deduce();
    
    // Both Q and R should now be TRUE
    assert(rationator.getPropositionTruthValue("Q") == Tripartite::TRUE);
    assert(rationator.getPropositionTruthValue("R") == Tripartite::TRUE);
    
    // Verify provenance was tracked
    const Proposition* rProp = rationator.getProposition("R");
    assert(rProp != nullptr);
    assert(rProp->hasProvenance());
    
    std::cout << "Test passed: Hypothetical Syllogism correctly chains implications." << std::endl;
}

// Test: Hypothetical Syllogism backward chaining - P → Q, Q → R, R is FALSE ⊢ P is FALSE
void testHypotheticalSyllogismBackward() {
    std::cout << "Running testHypotheticalSyllogismBackward..." << std::endl;

    Ratiocinator rationator;
    
    // Create a chain: P → Q → R
    Proposition impPQ;
    impPQ.setPrefix("impl_PQ");
    impPQ.setRelation(LogicalOperator::IMPLIES);
    impPQ.setAntecedent("P");
    impPQ.setConsequent("Q");
    rationator.setProposition("Q", impPQ);
    
    Proposition impQR;
    impQR.setPrefix("impl_QR");
    impQR.setRelation(LogicalOperator::IMPLIES);
    impQR.setAntecedent("Q");
    impQR.setConsequent("R");
    rationator.setProposition("R", impQR);
    
    // Set R to FALSE
    rationator.setPropositionTruthValue("R", Tripartite::FALSE);
    
    // Run deduction - backward chaining should derive P is FALSE
    rationator.deduce();
    
    // Both Q and P should now be FALSE
    assert(rationator.getPropositionTruthValue("Q") == Tripartite::FALSE);
    assert(rationator.getPropositionTruthValue("P") == Tripartite::FALSE);
    
    std::cout << "Test passed: Hypothetical Syllogism backward chaining works." << std::endl;
}

// Test: Disjunctive Syllogism - P ∨ Q, ¬P ⊢ Q
void testDisjunctiveSyllogism() {
    std::cout << "Running testDisjunctiveSyllogism..." << std::endl;

    Ratiocinator rationator;
    
    // Create disjunction: P ∨ Q
    Proposition disjPQ;
    disjPQ.setPrefix("disj_PQ");
    disjPQ.setRelation(LogicalOperator::OR);
    disjPQ.setAntecedent("P");
    disjPQ.setConsequent("Q");
    rationator.setProposition("disj_PQ", disjPQ);
    
    // Set P to FALSE
    rationator.setPropositionTruthValue("P", Tripartite::FALSE);
    
    // Initially Q should be UNKNOWN
    assert(rationator.getPropositionTruthValue("Q") == Tripartite::UNKNOWN);
    
    // Run deduction - Disjunctive Syllogism should derive Q is TRUE
    rationator.deduce();
    
    // Q should now be TRUE
    assert(rationator.getPropositionTruthValue("Q") == Tripartite::TRUE);
    
    // Verify provenance
    const Proposition* qProp = rationator.getProposition("Q");
    assert(qProp != nullptr);
    assert(qProp->hasProvenance());
    assert(qProp->getProvenance()->ruleFired == "DisjunctiveSyllogism");
    
    std::cout << "Test passed: Disjunctive Syllogism correctly infers from disjunction." << std::endl;
}

// Test: Disjunctive Syllogism - P ∨ Q, ¬Q ⊢ P
void testDisjunctiveSyllogismReverse() {
    std::cout << "Running testDisjunctiveSyllogismReverse..." << std::endl;

    Ratiocinator rationator;
    
    // Create disjunction: P ∨ Q
    Proposition disjPQ;
    disjPQ.setPrefix("disj_PQ");
    disjPQ.setRelation(LogicalOperator::OR);
    disjPQ.setAntecedent("P");
    disjPQ.setConsequent("Q");
    rationator.setProposition("disj_PQ", disjPQ);
    
    // Set Q to FALSE
    rationator.setPropositionTruthValue("Q", Tripartite::FALSE);
    
    // Initially P should be UNKNOWN
    assert(rationator.getPropositionTruthValue("P") == Tripartite::UNKNOWN);
    
    // Run deduction
    rationator.deduce();
    
    // P should now be TRUE
    assert(rationator.getPropositionTruthValue("P") == Tripartite::TRUE);
    
    std::cout << "Test passed: Disjunctive Syllogism works in reverse direction." << std::endl;
}

// Test: Resolution - P ∨ Q, ~P ∨ R, Q is FALSE ⊢ R is TRUE
void testResolution() {
    std::cout << "Running testResolution..." << std::endl;

    Ratiocinator rationator;
    
    // Create two disjunctions with complementary literals
    // disj1: P ∨ Q
    Proposition disj1;
    disj1.setPrefix("disj1");
    disj1.setRelation(LogicalOperator::OR);
    disj1.setAntecedent("P");
    disj1.setConsequent("Q");
    rationator.setProposition("disj1", disj1);
    
    // disj2: ~P ∨ R (using negation naming convention)
    Proposition disj2;
    disj2.setPrefix("disj2");
    disj2.setRelation(LogicalOperator::OR);
    disj2.setAntecedent("~P");
    disj2.setConsequent("R");
    rationator.setProposition("disj2", disj2);
    
    // Set Q to FALSE - by resolution, R must be TRUE
    rationator.setPropositionTruthValue("Q", Tripartite::FALSE);
    
    // Run deduction
    rationator.deduce();
    
    // R should now be TRUE (from resolution: Q∨R, Q=FALSE ⊢ R=TRUE)
    assert(rationator.getPropositionTruthValue("R") == Tripartite::TRUE);
    
    // Verify provenance
    const Proposition* rProp = rationator.getProposition("R");
    assert(rProp != nullptr);
    assert(rProp->hasProvenance());
    assert(rProp->getProvenance()->ruleFired == "Resolution");
    
    std::cout << "Test passed: Resolution rule correctly derives from complementary literals." << std::endl;
}

// Test: Inference provenance tracking
void testInferenceProvenance() {
    std::cout << "Running testInferenceProvenance..." << std::endl;

    Ratiocinator rationator;
    
    // Create an implication: A → B
    Proposition impAB;
    impAB.setPrefix("impl_AB");
    impAB.setRelation(LogicalOperator::IMPLIES);
    impAB.setAntecedent("A");
    impAB.setConsequent("B");
    rationator.setProposition("B", impAB);
    
    // Set A to TRUE
    rationator.setPropositionTruthValue("A", Tripartite::TRUE);
    
    // Run deduction
    rationator.deduce();
    
    // Verify B has provenance
    const Proposition* bProp = rationator.getProposition("B");
    assert(bProp != nullptr);
    assert(bProp->getTruthValue() == Tripartite::TRUE);
    assert(bProp->hasProvenance());
    assert(bProp->getProvenance()->ruleFired == "ModusPonens");
    assert(bProp->getProvenance()->premises.size() == 2);
    // Premises should include the antecedent "A" and the implication prefix
    bool hasAntecedent = false;
    for (const auto& premise : bProp->getProvenance()->premises) {
        if (premise == "A") hasAntecedent = true;
    }
    assert(hasAntecedent);
    
    std::cout << "Test passed: Inference provenance is correctly tracked." << std::endl;
}

// ============================================================
// ENHANCED PARSER TESTS - Expression syntax in facts files
// ============================================================

// Test: parseExpressionString builds expressions correctly
void testParseExpressionString() {
    std::cout << "Running testParseExpressionString..." << std::endl;
    
    Parser parser;
    std::unordered_map<std::string, Proposition> propositions;
    
    // Set up some known propositions
    propositions["A"].setTruthValue(Tripartite::TRUE);
    propositions["B"].setTruthValue(Tripartite::TRUE);
    propositions["C"].setTruthValue(Tripartite::FALSE);
    
    // Test simple AND expression
    Expression expr1 = parser.parseExpressionString("A && B", propositions, "test1");
    Tripartite result1 = expr1.evaluate();
    assert(result1 == Tripartite::TRUE);  // TRUE && TRUE = TRUE
    
    // Test AND with FALSE
    Expression expr2 = parser.parseExpressionString("A && C", propositions, "test2");
    Tripartite result2 = expr2.evaluate();
    assert(result2 == Tripartite::FALSE);  // TRUE && FALSE = FALSE
    
    // Test OR expression
    Expression expr3 = parser.parseExpressionString("A || C", propositions, "test3");
    Tripartite result3 = expr3.evaluate();
    assert(result3 == Tripartite::TRUE);  // TRUE || FALSE = TRUE
    
    // Test parenthesized expression
    Expression expr4 = parser.parseExpressionString("(A && B) || C", propositions, "test4");
    Tripartite result4 = expr4.evaluate();
    assert(result4 == Tripartite::TRUE);  // (TRUE && TRUE) || FALSE = TRUE
    
    std::cout << "Test passed: parseExpressionString builds expressions correctly." << std::endl;
}

// Test: Enhanced facts file with assignments
void testEnhancedFactsFile() {
    std::cout << "Running testEnhancedFactsFile..." << std::endl;
    
    Ratiocinator rationator;
    rationator.loadAssumptions(assumptionsFile);
    rationator.loadFacts(factsFile);
    
    // The facts file contains: t = p && n
    // p and n should both be TRUE (from other lines)
    assert(rationator.getPropositionTruthValue("p") == Tripartite::TRUE);
    assert(rationator.getPropositionTruthValue("n") == Tripartite::TRUE);
    
    // t should be assigned the result of p && n = TRUE
    assert(rationator.getPropositionTruthValue("t") == Tripartite::TRUE);
    
    // s = t || r, where t=TRUE, so s should be TRUE
    assert(rationator.getPropositionTruthValue("s") == Tripartite::TRUE);
    
    // u = t && s, where both are TRUE
    assert(rationator.getPropositionTruthValue("u") == Tripartite::TRUE);
    
    std::cout << "Test passed: Enhanced facts file with assignments works correctly." << std::endl;
}

// Test: Negation in facts file
void testFactsNegation() {
    std::cout << "Running testFactsNegation..." << std::endl;
    
    Ratiocinator rationator;
    rationator.loadAssumptions(assumptionsFile);
    rationator.loadFacts(factsFile);
    
    // !q should set q to FALSE
    assert(rationator.getPropositionTruthValue("q") == Tripartite::FALSE);
    
    // !r should set r to FALSE
    assert(rationator.getPropositionTruthValue("r") == Tripartite::FALSE);
    
    std::cout << "Test passed: Negation in facts file works correctly." << std::endl;
}

// Main function to run all tests
int main() {
    // Parsing tests
    testParseImpliesRelation();
    testParseSomeRelation();
    testParseNotRelation();
    testParseDiscoveredRelation();
    testParseFactsFile();
    
    // Basic inference tests
    testModusPonens();
    testModusTollens();
    testChainedInference();
    testNoInferenceOnUnknown();
    
    // Extended inference tests (new rules)
    testHypotheticalSyllogism();
    testHypotheticalSyllogismBackward();
    testDisjunctiveSyllogism();
    testDisjunctiveSyllogismReverse();
    testResolution();
    testInferenceProvenance();
    
    // Enhanced parser tests (expression syntax)
    testParseExpressionString();
    testEnhancedFactsFile();
    testFactsNegation();

    std::cout << "\n All Ratiocinator tests passed successfully!" << std::endl;
    return 0;
}
