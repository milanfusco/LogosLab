#include "Ratiocinator.h"
#include "Expression.h"
#include "Proposition.h"
#include <iostream>

// Constructor
Ratiocinator::Ratiocinator() {}

// Destructor
Ratiocinator::~Ratiocinator() = default;

// Private helper: safe internal lookup (returns nullptr if not found)
Proposition* Ratiocinator::findProposition(const std::string& name) {
    auto it = propositions_.find(name);
    return (it != propositions_.end()) ? &it->second : nullptr;
}

const Proposition* Ratiocinator::findProposition(const std::string& name) const {
    auto it = propositions_.find(name);
    return (it != propositions_.end()) ? &it->second : nullptr;
}

// Proposition accessors
void Ratiocinator::setProposition(const std::string& name, const Proposition& prop) {
    propositions_[name] = prop;
}

const Proposition* Ratiocinator::getProposition(const std::string& name) const {
    auto it = propositions_.find(name);
    return (it != propositions_.end()) ? &it->second : nullptr;
}

Proposition* Ratiocinator::getProposition(const std::string& name) {
    auto it = propositions_.find(name);
    return (it != propositions_.end()) ? &it->second : nullptr;
}

bool Ratiocinator::hasProposition(const std::string& name) const {
    return propositions_.find(name) != propositions_.end();
}

void Ratiocinator::setPropositionTruthValue(const std::string& name, Tripartite value) {
    propositions_[name].setTruthValue(value);
}

Tripartite Ratiocinator::getPropositionTruthValue(const std::string& name) const {
    auto it = propositions_.find(name);
    return (it != propositions_.end()) ? it->second.getTruthValue() : Tripartite::UNKNOWN;
}

const std::unordered_map<std::string, Proposition>& Ratiocinator::getPropositions() const {
    return propositions_;
}

// Expression accessors
void Ratiocinator::addExpression(const Expression& expr) {
    expressions_.push_back(expr);
}

const std::vector<Expression>& Ratiocinator::getExpressions() const {
    return expressions_;
}

// Parse assumptions file and merge results into propositions
void Ratiocinator::parseAssumptionsFile(const std::string& filename) {
    auto parsed = parser_.parseAssumptionsFile(filename);
    for (auto& entry : parsed) {
        propositions_[entry.first] = std::move(entry.second);
    }
}

// Parse facts file and update truth values
void Ratiocinator::parseFactsFile(const std::string& filename) {
    parser_.parseFactsFile(filename, propositions_);
}


void Ratiocinator::deduceAll() {
    bool changesMade;
    do {
        changesMade = false;
        
        // ============================================================
        // PHASE 1: Apply inference rules to IMPLIES propositions
        // ============================================================
        for (auto& entry : propositions_) {
            const Proposition& prop = entry.second;
            if (prop.getRelation() == LogicalOperator::IMPLIES) {
                // Apply Modus Ponens: P → Q, P is TRUE ⊢ Q is TRUE
                if (applyModusPonens(prop)) {
                    changesMade = true;
                }
                
                // Apply Modus Tollens: P → Q, Q is FALSE ⊢ P is FALSE
                if (applyModusTollens(prop)) {
                    changesMade = true;
                }
            }
        }
        
        // ============================================================
        // PHASE 2: Evaluate explicit Expression objects (if any)
        // ============================================================
        for (auto& expr : expressions_) {
            Tripartite resultValue = expr.evaluate();
            const std::string& subject = expr.getPrefix();

            // Safe lookup of subject proposition
            Proposition* subjectProp = findProposition(subject);
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

// Output the truth values of all propositions
std::string Ratiocinator::outputTruthValues() const {
    std::string output;
    std::string truthValueStr;

    for (const auto& entry : propositions_) {
        const auto& name = entry.first;
        const auto& prop = entry.second;
        std::cout << name << ": ";
        switch (prop.getTruthValue()) {
            case Tripartite::TRUE:
                std::cout << "True";
                truthValueStr = "True";
                break;
            case Tripartite::FALSE:
                std::cout << "False";
                truthValueStr = "False";

                break;
            case Tripartite::UNKNOWN:
                std::cout << "Unknown";
                truthValueStr = "Unknown";
                break;
        }
       
        output += name + ": " + truthValueStr + "\n";
        
        std::cout << std::endl;
    }
    return output;
}

// Apply Modus Ponens: P → Q, P is TRUE ⊢ Q is TRUE
// Returns true if a change was made (consequent wasn't already TRUE)
bool Ratiocinator::applyModusPonens(const Proposition& implication) {
    const std::string& antecedentName = implication.getAntecedent();
    const std::string& consequentName = implication.getConsequent();
    
    // Safe lookup of antecedent and consequent
    const Proposition* antecedentProp = findProposition(antecedentName);
    Proposition* consequentProp = findProposition(consequentName);
    
    Tripartite antecedentTruth = antecedentProp ? antecedentProp->getTruthValue() : Tripartite::UNKNOWN;
    Tripartite consequentTruth = consequentProp ? consequentProp->getTruthValue() : Tripartite::UNKNOWN;
    
    // Only apply if antecedent is TRUE and consequent is not already TRUE
    if (antecedentTruth == Tripartite::TRUE && consequentTruth != Tripartite::TRUE) {
        if (consequentProp) {
            consequentProp->setTruthValue(Tripartite::TRUE);
        } else {
            propositions_[consequentName].setTruthValue(Tripartite::TRUE);
        }
        return true;
    }
    return false;
}

// Apply Modus Tollens: P → Q, Q is FALSE ⊢ P is FALSE
// Returns true if a change was made (antecedent wasn't already FALSE)
bool Ratiocinator::applyModusTollens(const Proposition& implication) {
    const std::string& antecedentName = implication.getAntecedent();
    const std::string& consequentName = implication.getConsequent();
    
    // Safe lookup of antecedent and consequent
    Proposition* antecedentProp = findProposition(antecedentName);
    const Proposition* consequentProp = findProposition(consequentName);
    
    Tripartite antecedentTruth = antecedentProp ? antecedentProp->getTruthValue() : Tripartite::UNKNOWN;
    Tripartite consequentTruth = consequentProp ? consequentProp->getTruthValue() : Tripartite::UNKNOWN;
    
    // Only apply if consequent is FALSE and antecedent is not already FALSE
    if (consequentTruth == Tripartite::FALSE && antecedentTruth != Tripartite::FALSE) {
        if (antecedentProp) {
            antecedentProp->setTruthValue(Tripartite::FALSE);
        } else {
            propositions_[antecedentName].setTruthValue(Tripartite::FALSE);
        }
        return true;
    }
    return false;
}
