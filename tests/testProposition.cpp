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

void testInferenceProvenance() {
    std::cout << "Running testInferenceProvenance..." << std::endl;

    Proposition prop;
    
    // Initially no provenance
    assert(!prop.hasProvenance());
    assert(!prop.getProvenance().has_value());
    
    // Set truth value with provenance
    InferenceProvenance prov("ModusPonens", {"A", "A->B"}, 0.95f);
    prop.setTruthValue(Tripartite::TRUE, prov);
    
    // Verify provenance is stored
    assert(prop.hasProvenance());
    assert(prop.getProvenance().has_value());
    assert(prop.getProvenance()->ruleFired == "ModusPonens");
    assert(prop.getProvenance()->premises.size() == 2);
    assert(prop.getProvenance()->premises[0] == "A");
    assert(prop.getProvenance()->premises[1] == "A->B");
    assert(prop.getProvenance()->confidence == 0.95f);
    
    // Setting value without provenance clears it
    prop.setTruthValue(Tripartite::FALSE);
    assert(!prop.hasProvenance());
    
    std::cout << "testInferenceProvenance passed.\n";
}

void testConflictDetection() {
    std::cout << "Running testConflictDetection..." << std::endl;

    Proposition prop;
    
    // Initially no conflicts
    assert(!prop.hasConflicts());
    assert(prop.getConflicts().empty());
    
    // Set initial value with provenance
    InferenceProvenance prov1("ModusPonens", {"P", "P->Q"});
    prop.setTruthValue(Tripartite::TRUE, prov1);
    
    // No conflict yet (was UNKNOWN before)
    assert(!prop.hasConflicts());
    
    // Now overwrite with a different value - should create a conflict
    InferenceProvenance prov2("ModusTollens", {"~Q", "P->Q"});
    prop.setTruthValue(Tripartite::FALSE, prov2);
    
    // Should have recorded a conflict
    assert(prop.hasConflicts());
    assert(prop.getConflicts().size() == 1);
    
    const Conflict& conflict = prop.getConflicts()[0];
    assert(conflict.oldValue == Tripartite::TRUE);
    assert(conflict.newValue == Tripartite::FALSE);
    assert(conflict.oldProvenance.ruleFired == "ModusPonens");
    assert(conflict.newProvenance.ruleFired == "ModusTollens");
    
    // Setting same value should not create additional conflict
    InferenceProvenance prov3("DirectAssertion", {"fact"});
    prop.setTruthValue(Tripartite::FALSE, prov3);
    assert(prop.getConflicts().size() == 1);  // Still just one conflict
    
    // Clear conflicts
    prop.clearConflicts();
    assert(!prop.hasConflicts());
    assert(prop.getConflicts().empty());
    
    std::cout << "testConflictDetection passed.\n";
}

void testProvenanceInAssignment() {
    std::cout << "Running testProvenanceInAssignment..." << std::endl;

    Proposition prop1;
    InferenceProvenance prov("TestRule", {"X"});
    prop1.setTruthValue(Tripartite::TRUE, prov);
    
    // Create conflict
    InferenceProvenance prov2("AnotherRule", {"Y"});
    prop1.setTruthValue(Tripartite::FALSE, prov2);
    
    // Copy via assignment
    Proposition prop2;
    prop2 = prop1;
    
    // Verify provenance and conflicts are copied
    assert(prop2.hasProvenance());
    assert(prop2.getProvenance()->ruleFired == "AnotherRule");
    assert(prop2.hasConflicts());
    assert(prop2.getConflicts().size() == 1);
    
    std::cout << "testProvenanceInAssignment passed.\n";
}

int main() {
    testPropositionAttributes();
    testPropositionAssertions();
    testLogicalOperations();
    testDisplay();
    testInferenceProvenance();
    testConflictDetection();
    testProvenanceInAssignment();

    std::cout << "All tests passed successfully.\n";
    return 0;
}
