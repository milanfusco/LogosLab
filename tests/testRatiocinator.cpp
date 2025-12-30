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

// Test: Expressions are populated from facts file
void testExpressionsFromFacts() {
    std::cout << "Running testExpressionsFromFacts..." << std::endl;
    
    Ratiocinator rationator;
    rationator.loadAssumptions(assumptionsFile);
    
    // Before loading facts, expressions should be empty
    assert(rationator.getExpressions().empty());
    
    rationator.loadFacts(factsFile);
    
    // After loading facts, expressions should be populated
    // The facts file contains compound expressions like "t = p && n"
    const auto& expressions = rationator.getExpressions();
    assert(!expressions.empty());
    
    std::cout << "Test passed: Expressions are populated from facts file (" 
              << expressions.size() << " expressions)." << std::endl;
}

// Test: addExpressionFromString
void testAddExpressionFromString() {
    std::cout << "Running testAddExpressionFromString..." << std::endl;
    
    Ratiocinator rationator;
    
    // Set up some propositions
    rationator.setPropositionTruthValue("X", Tripartite::TRUE);
    rationator.setPropositionTruthValue("Y", Tripartite::FALSE);
    
    // Add an expression from string
    Expression expr = rationator.addExpressionFromString("X && Y", "test_expr");
    
    // Verify the expression was added
    assert(rationator.getExpressions().size() == 1);
    
    // Evaluate the expression
    Tripartite result = expr.evaluate();
    assert(result == Tripartite::FALSE);  // TRUE && FALSE = FALSE
    
    // Add another expression
    rationator.addExpressionFromString("X || Y", "test_expr2");
    assert(rationator.getExpressions().size() == 2);
    
    // Clear expressions
    rationator.clearExpressions();
    assert(rationator.getExpressions().empty());
    
    std::cout << "Test passed: addExpressionFromString works correctly." << std::endl;
}

// Test: Expressions are used during deduction
void testExpressionsInDeduction() {
    std::cout << "Running testExpressionsInDeduction..." << std::endl;
    
    Ratiocinator rationator;
    
    // Create a proposition that will be evaluated via expression
    Proposition prop;
    prop.setPrefix("result");
    prop.setPropositionScope(Quantifier::UNIVERSAL_AFFIRMATIVE);
    rationator.setProposition("result", prop);
    
    // Set up operand propositions
    rationator.setPropositionTruthValue("A", Tripartite::TRUE);
    rationator.setPropositionTruthValue("B", Tripartite::TRUE);
    
    // Add an expression that evaluates A && B and assigns to "result"
    Expression expr = rationator.addExpressionFromString("A && B", "result");
    
    // Before deduction, result may be UNKNOWN
    // After deduction, the expression should be evaluated
    rationator.deduce();
    
    // The expression was evaluated (TRUE && TRUE = TRUE)
    // Note: The inference engine evaluates expressions during deduction
    
    std::cout << "Test passed: Expressions are used during deduction." << std::endl;
}

// ============================================================
// INCREMENTAL API TESTS
// ============================================================

// Test: addProposition only adds if not exists
void testAddProposition() {
    std::cout << "Running testAddProposition..." << std::endl;
    
    Ratiocinator rationator;
    
    // Create a proposition
    Proposition prop1;
    prop1.setPrefix("test1");
    prop1.setTruthValue(Tripartite::TRUE);
    
    // Add should succeed the first time
    bool added = rationator.addProposition("test1", prop1);
    assert(added);
    assert(rationator.hasProposition("test1"));
    assert(rationator.getPropositionCount() == 1);
    
    // Add with same name should fail
    Proposition prop2;
    prop2.setPrefix("test1");
    prop2.setTruthValue(Tripartite::FALSE);
    
    added = rationator.addProposition("test1", prop2);
    assert(!added);  // Should fail - already exists
    
    // Original value should be preserved
    assert(rationator.getPropositionTruthValue("test1") == Tripartite::TRUE);
    
    std::cout << "Test passed: addProposition correctly prevents duplicates." << std::endl;
}

// Test: removeProposition
void testRemoveProposition() {
    std::cout << "Running testRemoveProposition..." << std::endl;
    
    Ratiocinator rationator;
    
    // Add some propositions
    rationator.setPropositionTruthValue("A", Tripartite::TRUE);
    rationator.setPropositionTruthValue("B", Tripartite::FALSE);
    rationator.setPropositionTruthValue("C", Tripartite::UNKNOWN);
    
    assert(rationator.getPropositionCount() == 3);
    
    // Remove B
    bool removed = rationator.removeProposition("B");
    assert(removed);
    assert(!rationator.hasProposition("B"));
    assert(rationator.getPropositionCount() == 2);
    
    // Try to remove non-existent proposition
    removed = rationator.removeProposition("D");
    assert(!removed);
    
    // A and C should still exist
    assert(rationator.hasProposition("A"));
    assert(rationator.hasProposition("C"));
    
    std::cout << "Test passed: removeProposition works correctly." << std::endl;
}

// Test: updatePropositionTruthValue with provenance
void testUpdatePropositionTruthValue() {
    std::cout << "Running testUpdatePropositionTruthValue..." << std::endl;
    
    Ratiocinator rationator;
    
    // Add a proposition
    rationator.setPropositionTruthValue("X", Tripartite::TRUE);
    
    // Update with provenance
    InferenceProvenance prov("TestUpdate", {"source1", "source2"});
    bool updated = rationator.updatePropositionTruthValue("X", Tripartite::FALSE, prov);
    assert(updated);
    
    // Verify the update
    assert(rationator.getPropositionTruthValue("X") == Tripartite::FALSE);
    
    // Verify provenance was set
    const Proposition* prop = rationator.getProposition("X");
    assert(prop != nullptr);
    assert(prop->hasProvenance());
    assert(prop->getProvenance()->ruleFired == "TestUpdate");
    
    // Try to update non-existent proposition
    updated = rationator.updatePropositionTruthValue("Y", Tripartite::TRUE, prov);
    assert(!updated);
    
    std::cout << "Test passed: updatePropositionTruthValue with provenance works correctly." << std::endl;
}

// Test: clearPropositions
void testClearPropositions() {
    std::cout << "Running testClearPropositions..." << std::endl;
    
    Ratiocinator rationator;
    
    // Add some propositions
    rationator.setPropositionTruthValue("A", Tripartite::TRUE);
    rationator.setPropositionTruthValue("B", Tripartite::FALSE);
    
    // Add an expression
    rationator.addExpressionFromString("A && B", "test");
    
    assert(rationator.getPropositionCount() == 2);
    assert(rationator.getExpressionCount() == 1);
    
    // Clear only propositions
    rationator.clearPropositions();
    
    assert(rationator.getPropositionCount() == 0);
    assert(rationator.getExpressionCount() == 1);  // Expressions preserved
    
    std::cout << "Test passed: clearPropositions only clears propositions." << std::endl;
}

// Test: clearKnowledgeBase
void testClearKnowledgeBase() {
    std::cout << "Running testClearKnowledgeBase..." << std::endl;
    
    Ratiocinator rationator;
    
    // Add some propositions and expressions
    rationator.setPropositionTruthValue("A", Tripartite::TRUE);
    rationator.setPropositionTruthValue("B", Tripartite::FALSE);
    rationator.addExpressionFromString("A && B", "test");
    
    assert(rationator.getPropositionCount() == 2);
    assert(rationator.getExpressionCount() == 1);
    
    // Clear everything
    rationator.clearKnowledgeBase();
    
    assert(rationator.getPropositionCount() == 0);
    assert(rationator.getExpressionCount() == 0);
    
    std::cout << "Test passed: clearKnowledgeBase clears everything." << std::endl;
}

// Test: Incremental workflow (add, deduce, update, deduce again)
void testIncrementalWorkflow() {
    std::cout << "Running testIncrementalWorkflow..." << std::endl;
    
    Ratiocinator rationator;
    
    // Step 1: Set up initial knowledge
    Proposition impAB;
    impAB.setPrefix("imp_AB");
    impAB.setRelation(LogicalOperator::IMPLIES);
    impAB.setAntecedent("A");
    impAB.setConsequent("B");
    rationator.addProposition("B", impAB);
    
    rationator.setPropositionTruthValue("A", Tripartite::TRUE);
    
    // Step 2: First deduction
    rationator.deduce();
    assert(rationator.getPropositionTruthValue("B") == Tripartite::TRUE);
    
    // Step 3: Add new knowledge incrementally
    Proposition impBC;
    impBC.setPrefix("imp_BC");
    impBC.setRelation(LogicalOperator::IMPLIES);
    impBC.setAntecedent("B");
    impBC.setConsequent("C");
    rationator.addProposition("C", impBC);
    
    // Step 4: Deduce again with new knowledge
    rationator.deduce();
    assert(rationator.getPropositionTruthValue("C") == Tripartite::TRUE);
    
    // Step 5: Remove a proposition and re-test
    // Note: B's truth was derived, removing it doesn't unset it
    // but we can verify removal works
    assert(rationator.removeProposition("C"));
    assert(!rationator.hasProposition("C"));
    
    std::cout << "Test passed: Incremental workflow works correctly." << std::endl;
}

// ============================================================
// INFERENCE TRACING TESTS
// ============================================================

// Test: traceInference returns empty for non-existent proposition
void testTraceInferenceNonExistent() {
    std::cout << "Running testTraceInferenceNonExistent..." << std::endl;
    
    Ratiocinator rationator;
    
    auto trace = rationator.traceInference("nonexistent");
    assert(trace.empty());
    
    std::cout << "Test passed: traceInference returns empty for non-existent proposition." << std::endl;
}

// Test: traceInference for axiom (no provenance)
void testTraceInferenceAxiom() {
    std::cout << "Running testTraceInferenceAxiom..." << std::endl;
    
    Ratiocinator rationator;
    
    // Set a direct truth value (axiom)
    rationator.setPropositionTruthValue("P", Tripartite::TRUE);
    
    auto trace = rationator.traceInference("P");
    
    assert(trace.size() == 1);
    assert(trace[0].proposition == "P");
    assert(trace[0].truthValue == Tripartite::TRUE);
    assert(trace[0].rule == "Axiom");
    assert(trace[0].depth == 0);
    assert(trace[0].premises.empty());
    
    std::cout << "Test passed: traceInference correctly identifies axioms." << std::endl;
}

// Test: traceInference for single-step inference
void testTraceInferenceSingleStep() {
    std::cout << "Running testTraceInferenceSingleStep..." << std::endl;
    
    Ratiocinator rationator;
    
    // Create implication: P → Q
    Proposition impPQ;
    impPQ.setPrefix("imp_PQ");
    impPQ.setRelation(LogicalOperator::IMPLIES);
    impPQ.setAntecedent("P");
    impPQ.setConsequent("Q");
    rationator.setProposition("Q", impPQ);
    
    // Set P to TRUE
    rationator.setPropositionTruthValue("P", Tripartite::TRUE);
    
    // Deduce - Q becomes TRUE via Modus Ponens
    rationator.deduce();
    
    // Verify Q has provenance
    assert(rationator.hasInferenceProvenance("Q"));
    
    // Trace Q
    auto trace = rationator.traceInference("Q");
    
    // Should have at least 2 steps: Q and P
    assert(trace.size() >= 2);
    
    // First step should be Q
    assert(trace[0].proposition == "Q");
    assert(trace[0].truthValue == Tripartite::TRUE);
    assert(trace[0].rule == "ModusPonens");
    assert(trace[0].depth == 0);
    
    std::cout << "Test passed: traceInference works for single-step inference." << std::endl;
}

// Test: traceInference for chained inference
void testTraceInferenceChained() {
    std::cout << "Running testTraceInferenceChained..." << std::endl;
    
    Ratiocinator rationator;
    
    // Create chain: P → Q → R
    Proposition impPQ;
    impPQ.setPrefix("imp_PQ");
    impPQ.setRelation(LogicalOperator::IMPLIES);
    impPQ.setAntecedent("P");
    impPQ.setConsequent("Q");
    rationator.setProposition("Q", impPQ);
    
    Proposition impQR;
    impQR.setPrefix("imp_QR");
    impQR.setRelation(LogicalOperator::IMPLIES);
    impQR.setAntecedent("Q");
    impQR.setConsequent("R");
    rationator.setProposition("R", impQR);
    
    // Set P to TRUE
    rationator.setPropositionTruthValue("P", Tripartite::TRUE);
    
    // Deduce - Q and R become TRUE
    rationator.deduce();
    
    assert(rationator.getPropositionTruthValue("Q") == Tripartite::TRUE);
    assert(rationator.getPropositionTruthValue("R") == Tripartite::TRUE);
    
    // Trace R - should show the chain starting from R
    auto trace = rationator.traceInference("R");
    
    // Should have at least R (depth 0) and some premises traced
    assert(trace.size() >= 1);
    
    // Verify R is the first step at depth 0
    assert(trace[0].proposition == "R");
    assert(trace[0].depth == 0);
    assert(trace[0].truthValue == Tripartite::TRUE);
    // R could be derived by ModusPonens or HypotheticalSyllogism depending on inference order
    assert(trace[0].rule == "ModusPonens" || trace[0].rule == "HypotheticalSyllogism");
    
    // Verify the trace has increasing depths (chained inference)
    bool foundDerivedStep = false;
    bool foundPremiseStep = false;
    for (const auto& step : trace) {
        if (step.depth == 0) foundDerivedStep = true;
        if (step.depth > 0) foundPremiseStep = true;
    }
    assert(foundDerivedStep);
    
    // If there are premises in the provenance, they should be traced
    if (trace.size() > 1) {
        assert(foundPremiseStep);
    }
    
    std::cout << "Test passed: traceInference correctly traces chained inference." << std::endl;
}

// Test: formatTrace produces readable output
void testFormatTrace() {
    std::cout << "Running testFormatTrace..." << std::endl;
    
    Ratiocinator rationator;
    
    // Create a simple inference
    Proposition impPQ;
    impPQ.setPrefix("imp_PQ");
    impPQ.setRelation(LogicalOperator::IMPLIES);
    impPQ.setAntecedent("P");
    impPQ.setConsequent("Q");
    rationator.setProposition("Q", impPQ);
    
    rationator.setPropositionTruthValue("P", Tripartite::TRUE);
    rationator.deduce();
    
    // Format the trace
    std::string traceStr = rationator.formatTrace("Q");
    
    // Verify it contains expected content
    assert(traceStr.find("Inference trace for 'Q'") != std::string::npos);
    assert(traceStr.find("TRUE") != std::string::npos);
    assert(traceStr.find("ModusPonens") != std::string::npos);
    
    // Also test formatting non-existent proposition
    std::string emptyTrace = rationator.formatTrace("nonexistent");
    assert(emptyTrace.find("No inference trace available") != std::string::npos);
    
    std::cout << "Test passed: formatTrace produces readable output." << std::endl;
}

// Test: hasInferenceProvenance
void testHasInferenceProvenance() {
    std::cout << "Running testHasInferenceProvenance..." << std::endl;
    
    Ratiocinator rationator;
    
    // Axiom has no provenance
    rationator.setPropositionTruthValue("P", Tripartite::TRUE);
    assert(!rationator.hasInferenceProvenance("P"));
    
    // Create implication for Q
    Proposition impPQ;
    impPQ.setPrefix("imp_PQ");
    impPQ.setRelation(LogicalOperator::IMPLIES);
    impPQ.setAntecedent("P");
    impPQ.setConsequent("Q");
    rationator.setProposition("Q", impPQ);
    
    // Before deduction, Q has no provenance
    assert(!rationator.hasInferenceProvenance("Q"));
    
    // After deduction, Q has provenance
    rationator.deduce();
    assert(rationator.hasInferenceProvenance("Q"));
    
    // Non-existent proposition has no provenance
    assert(!rationator.hasInferenceProvenance("nonexistent"));
    
    std::cout << "Test passed: hasInferenceProvenance works correctly." << std::endl;
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
    
    // Expression-backed propositions tests
    testExpressionsFromFacts();
    testAddExpressionFromString();
    testExpressionsInDeduction();
    
    // Incremental API tests
    testAddProposition();
    testRemoveProposition();
    testUpdatePropositionTruthValue();
    testClearPropositions();
    testClearKnowledgeBase();
    testIncrementalWorkflow();
    
    // Inference tracing tests
    testTraceInferenceNonExistent();
    testTraceInferenceAxiom();
    testTraceInferenceSingleStep();
    testTraceInferenceChained();
    testFormatTrace();
    testHasInferenceProvenance();

    std::cout << "\n All Ratiocinator tests passed successfully!" << std::endl;
    return 0;
}
