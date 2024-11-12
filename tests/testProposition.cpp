#include "Proposition.h"
#include <iostream>
#include <cassert>

void testPropositionAttributes() {
    Proposition prop;

    // Test setting and getting prefix
    prop.setPrefix("n");
    assert(prop.getPrefix() == "n");

    // Test setting and getting relation
    prop.setRelation(LogicalOperator::IMPLIES);
    assert(prop.getRelation() == LogicalOperator::IMPLIES);

    // Test setting and getting antecedent
    prop.setAntecedent("big-bang");
    assert(prop.getAntecedent() == "big-bang");

    // Test setting and getting subject
    prop.setSubject("occurred");
    assert(prop.getSubject() == "occurred");

    // Test setting and getting consequent
    prop.setConsequent("microwave-radiation");
    assert(prop.getConsequent() == "microwave-radiation");

    // Test setting and getting predicate
    prop.setPredicate("present");
    assert(prop.getPredicate() == "present");

    // Test setting and getting truth value (Tripartite)
    prop.setTruthValue(Tripartite::TRUE);
    assert(prop.getTruthValue() == Tripartite::TRUE);

    std::cout << "testPropositionAttributes passed.\n";
}

void testPropositionAssertions() {
    Proposition prop;

    // Set antecedent assertion and check
    prop.setAntecedentAssertion(Tripartite::TRUE);
    assert(prop.getAntecedentAssertion() == Tripartite::TRUE);

    // Set consequent assertion and check
    prop.setConsequentAssertion(Tripartite::FALSE);
    assert(prop.getConsequentAssertion() == Tripartite::FALSE);

    std::cout << "testPropositionAssertions passed.\n";
}

void testLogicalOperations() {
    Proposition propTrue, propFalse, propUnknown;

    propTrue.setTruthValue(Tripartite::TRUE);
    propFalse.setTruthValue(Tripartite::FALSE);
    propUnknown.setTruthValue(Tripartite::UNKNOWN);

    // AND operation
    assert((propTrue.getTruthValue() && propTrue.getTruthValue()) == Tripartite::TRUE);
    assert((propTrue.getTruthValue() && propFalse.getTruthValue()) == Tripartite::FALSE);
    assert((propTrue.getTruthValue() && propUnknown.getTruthValue()) == Tripartite::UNKNOWN);

    // OR operation
    assert((propTrue.getTruthValue() || propFalse.getTruthValue()) == Tripartite::TRUE);
    assert((propFalse.getTruthValue() || propFalse.getTruthValue()) == Tripartite::FALSE);
    assert((propUnknown.getTruthValue() || propFalse.getTruthValue()) == Tripartite::UNKNOWN);

    // NOT operation
    assert(!propTrue.getTruthValue() == Tripartite::FALSE);
    assert(!propFalse.getTruthValue() == Tripartite::TRUE);
    assert(!propUnknown.getTruthValue() == Tripartite::UNKNOWN);

    // IMPLIES operation
    assert(propTrue.implies(propTrue) == Tripartite::TRUE);
    assert(propFalse.implies(propTrue) == Tripartite::TRUE);
    assert(propTrue.implies(propFalse) == Tripartite::FALSE);
    assert(propFalse.implies(propFalse) == Tripartite::TRUE);
    assert(propUnknown.implies(propTrue) == Tripartite::TRUE);
    assert(propTrue.implies(propUnknown) == Tripartite::UNKNOWN);

    // Equality operation
    assert((propTrue == propTrue) == true);
    assert((propTrue == propFalse) == false);
    assert((propTrue == propUnknown) == false);

    std::cout << "testLogicalOperations passed.\n";
}

void testDisplay() {
    Proposition prop;
    prop.setPrefix("q");
    prop.setRelation(LogicalOperator::AND);
    prop.setAntecedent("earth");
    prop.setSubject("exists");
    prop.setConsequent("life");
    prop.setPredicate("true");
    prop.setTruthValue(Tripartite::TRUE);

    std::cout << "Display test:\n";
    prop.display();  // Manually verify the output
}

int main() {
    testPropositionAttributes();
    testPropositionAssertions();
    testLogicalOperations();
    testDisplay();

    std::cout << "All tests passed successfully.\n";
    return 0;
}
