#include "InferenceEngine.h"

// Safe internal helper to find proposition (returns nullptr if not found)
Proposition* InferenceEngine::findProposition(const std::string& name,
                                              std::unordered_map<std::string, Proposition>& propositions) {
    auto it = propositions.find(name);
    return (it != propositions.end()) ? &it->second : nullptr;
}

const Proposition* InferenceEngine::findProposition(const std::string& name,
                                                    const std::unordered_map<std::string, Proposition>& propositions) {
    auto it = propositions.find(name);
    return (it != propositions.end()) ? &it->second : nullptr;
}

// Apply Modus Ponens: P → Q, P is TRUE ⊢ Q is TRUE
// Returns true if a change was made (consequent wasn't already TRUE)
bool InferenceEngine::applyModusPonens(const Proposition& implication,
                                       std::unordered_map<std::string, Proposition>& propositions) {
    const std::string& antecedentName = implication.getAntecedent();
    const std::string& consequentName = implication.getConsequent();
    
    // Safe lookup of antecedent and consequent
    const Proposition* antecedentProp = findProposition(antecedentName, propositions);
    Proposition* consequentProp = findProposition(consequentName, propositions);
    
    Tripartite antecedentTruth = antecedentProp ? antecedentProp->getTruthValue() : Tripartite::UNKNOWN;
    Tripartite consequentTruth = consequentProp ? consequentProp->getTruthValue() : Tripartite::UNKNOWN;
    
    // Only apply if antecedent is TRUE and consequent is not already TRUE
    if (antecedentTruth == Tripartite::TRUE && consequentTruth != Tripartite::TRUE) {
        if (consequentProp) {
            consequentProp->setTruthValue(Tripartite::TRUE);
        } else {
            propositions[consequentName].setTruthValue(Tripartite::TRUE);
        }
        return true;
    }
    return false;
}

// Apply Modus Tollens: P → Q, Q is FALSE ⊢ P is FALSE
// Returns true if a change was made (antecedent wasn't already FALSE)
bool InferenceEngine::applyModusTollens(const Proposition& implication,
                                        std::unordered_map<std::string, Proposition>& propositions) {
    const std::string& antecedentName = implication.getAntecedent();
    const std::string& consequentName = implication.getConsequent();
    
    // Safe lookup of antecedent and consequent
    Proposition* antecedentProp = findProposition(antecedentName, propositions);
    const Proposition* consequentProp = findProposition(consequentName, propositions);
    
    Tripartite antecedentTruth = antecedentProp ? antecedentProp->getTruthValue() : Tripartite::UNKNOWN;
    Tripartite consequentTruth = consequentProp ? consequentProp->getTruthValue() : Tripartite::UNKNOWN;
    
    // Only apply if consequent is FALSE and antecedent is not already FALSE
    if (consequentTruth == Tripartite::FALSE && antecedentTruth != Tripartite::FALSE) {
        if (antecedentProp) {
            antecedentProp->setTruthValue(Tripartite::FALSE);
        } else {
            propositions[antecedentName].setTruthValue(Tripartite::FALSE);
        }
        return true;
    }
    return false;
}

void InferenceEngine::deduceAll(std::unordered_map<std::string, Proposition>& propositions,
                                std::vector<Expression>& expressions) {
    bool changesMade;
    do {
        changesMade = false;
        
        // ============================================================
        // PHASE 1: Apply inference rules to IMPLIES propositions
        // ============================================================
        for (auto& entry : propositions) {
            const Proposition& prop = entry.second;
            if (prop.getRelation() == LogicalOperator::IMPLIES) {
                // Apply Modus Ponens: P → Q, P is TRUE ⊢ Q is TRUE
                if (applyModusPonens(prop, propositions)) {
                    changesMade = true;
                }
                
                // Apply Modus Tollens: P → Q, Q is FALSE ⊢ P is FALSE
                if (applyModusTollens(prop, propositions)) {
                    changesMade = true;
                }
            }
        }
        
        // ============================================================
        // PHASE 2: Evaluate explicit Expression objects (if any)
        // ============================================================
        for (auto& expr : expressions) {
            Tripartite resultValue = expr.evaluate();
            const std::string& subject = expr.getPrefix();

            // Safe lookup of subject proposition
            Proposition* subjectProp = findProposition(subject, propositions);
            if (!subjectProp) {
                // Skip if subject proposition doesn't exist
                continue;
            }

            Tripartite currentValue = subjectProp->getTruthValue();
            Quantifier scope = subjectProp->getPropositionScope();

            switch (scope) {
                case Quantifier::UNIVERSAL_AFFIRMATIVE:
                    if (currentValue != resultValue && resultValue == Tripartite::TRUE) {
                        subjectProp->setTruthValue(Tripartite::TRUE);
                        changesMade = true;
                    }
                    break;

                case Quantifier::UNIVERSAL_NEGATIVE:
                    if (currentValue != resultValue && resultValue == Tripartite::FALSE) {
                        subjectProp->setTruthValue(Tripartite::FALSE);
                        changesMade = true;
                    }
                    break;

                case Quantifier::PARTICULAR_AFFIRMATIVE:
                    if (resultValue == Tripartite::TRUE) {
                        subjectProp->setTruthValue(Tripartite::TRUE);
                        changesMade = true;
                    }
                    break;

                case Quantifier::PARTICULAR_NEGATIVE:
                    if (resultValue == Tripartite::FALSE && currentValue != Tripartite::TRUE) {
                        subjectProp->setTruthValue(Tripartite::FALSE);
                        changesMade = true;
                    }
                    break;

                default:
                    break;
            }
        }
    } while (changesMade);
}

