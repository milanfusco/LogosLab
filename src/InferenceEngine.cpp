#include "InferenceEngine.h"

// ========== Helper Methods ==========

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

// Check if a proposition name represents a negation (starts with "~" or "!")
bool InferenceEngine::isNegatedName(const std::string& name) {
    return !name.empty() && (name[0] == '~' || name[0] == '!');
}

// Get the base name without negation prefix
std::string InferenceEngine::getBaseName(const std::string& name) {
    if (isNegatedName(name)) {
        return name.substr(1);
    }
    return name;
}

// Get the negated form of a name
std::string InferenceEngine::getNegatedName(const std::string& name) {
    if (isNegatedName(name)) {
        return name.substr(1);  // Double negation returns the base name
    }
    return "~" + name;
}

// ========== Basic Inference Rules ==========

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
        InferenceProvenance prov("ModusPonens", {antecedentName, implication.getPrefix()});
        if (consequentProp) {
            consequentProp->setTruthValue(Tripartite::TRUE, prov);
        } else {
            propositions[consequentName].setTruthValue(Tripartite::TRUE, prov);
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
        InferenceProvenance prov("ModusTollens", {consequentName, implication.getPrefix()});
        if (antecedentProp) {
            antecedentProp->setTruthValue(Tripartite::FALSE, prov);
        } else {
            propositions[antecedentName].setTruthValue(Tripartite::FALSE, prov);
        }
        return true;
    }
    return false;
}

// ========== Extended Inference Rules ==========

// Apply Hypothetical Syllogism: P → Q, Q → R ⊢ P → R
// When P is TRUE, transitively infers R is TRUE through the chain
bool InferenceEngine::applyHypotheticalSyllogism(const Proposition& impl1,
                                                  const Proposition& impl2,
                                                  std::unordered_map<std::string, Proposition>& propositions) {
    // impl1: P → Q (antecedent = P, consequent = Q)
    // impl2: Q → R (antecedent = Q, consequent = R)
    // Check if impl1's consequent matches impl2's antecedent
    if (impl1.getConsequent() != impl2.getAntecedent()) {
        return false;
    }
    
    const std::string& P = impl1.getAntecedent();
    const std::string& R = impl2.getConsequent();
    
    // Get truth values
    const Proposition* pProp = findProposition(P, propositions);
    Proposition* rProp = findProposition(R, propositions);
    
    Tripartite pTruth = pProp ? pProp->getTruthValue() : Tripartite::UNKNOWN;
    Tripartite rTruth = rProp ? rProp->getTruthValue() : Tripartite::UNKNOWN;
    
    bool changesMade = false;
    
    // Forward chaining: If P is TRUE, then R is TRUE
    if (pTruth == Tripartite::TRUE && rTruth != Tripartite::TRUE) {
        InferenceProvenance prov("HypotheticalSyllogism", 
                                  {P, impl1.getPrefix(), impl2.getPrefix()});
        if (rProp) {
            rProp->setTruthValue(Tripartite::TRUE, prov);
        } else {
            propositions[R].setTruthValue(Tripartite::TRUE, prov);
        }
        changesMade = true;
    }
    
    // Backward chaining: If R is FALSE, then P is FALSE
    Proposition* pPropMut = findProposition(P, propositions);
    if (rTruth == Tripartite::FALSE && pTruth != Tripartite::FALSE) {
        InferenceProvenance prov("HypotheticalSyllogism", 
                                  {R, impl2.getPrefix(), impl1.getPrefix()});
        if (pPropMut) {
            pPropMut->setTruthValue(Tripartite::FALSE, prov);
        } else {
            propositions[P].setTruthValue(Tripartite::FALSE, prov);
        }
        changesMade = true;
    }
    
    return changesMade;
}

// Apply Disjunctive Syllogism: P ∨ Q, ¬P ⊢ Q (and P ∨ Q, ¬Q ⊢ P)
// For disjunctions stored with relation = OR, antecedent = P, consequent = Q
bool InferenceEngine::applyDisjunctiveSyllogism(const Proposition& disjunction,
                                                 std::unordered_map<std::string, Proposition>& propositions) {
    const std::string& leftDisjunct = disjunction.getAntecedent();   // P in P ∨ Q
    const std::string& rightDisjunct = disjunction.getConsequent();  // Q in P ∨ Q
    
    // Safe lookup of disjuncts
    const Proposition* leftProp = findProposition(leftDisjunct, propositions);
    const Proposition* rightProp = findProposition(rightDisjunct, propositions);
    Proposition* leftPropMut = findProposition(leftDisjunct, propositions);
    Proposition* rightPropMut = findProposition(rightDisjunct, propositions);
    
    Tripartite leftTruth = leftProp ? leftProp->getTruthValue() : Tripartite::UNKNOWN;
    Tripartite rightTruth = rightProp ? rightProp->getTruthValue() : Tripartite::UNKNOWN;
    
    bool changesMade = false;
    
    // Case 1: P ∨ Q, ¬P ⊢ Q
    // If left disjunct is FALSE, right disjunct must be TRUE
    if (leftTruth == Tripartite::FALSE && rightTruth != Tripartite::TRUE) {
        InferenceProvenance prov("DisjunctiveSyllogism", 
                                  {leftDisjunct, disjunction.getPrefix()});
        if (rightPropMut) {
            rightPropMut->setTruthValue(Tripartite::TRUE, prov);
        } else {
            propositions[rightDisjunct].setTruthValue(Tripartite::TRUE, prov);
        }
        changesMade = true;
    }
    
    // Case 2: P ∨ Q, ¬Q ⊢ P
    // If right disjunct is FALSE, left disjunct must be TRUE
    if (rightTruth == Tripartite::FALSE && leftTruth != Tripartite::TRUE) {
        InferenceProvenance prov("DisjunctiveSyllogism", 
                                  {rightDisjunct, disjunction.getPrefix()});
        if (leftPropMut) {
            leftPropMut->setTruthValue(Tripartite::TRUE, prov);
        } else {
            propositions[leftDisjunct].setTruthValue(Tripartite::TRUE, prov);
        }
        changesMade = true;
    }
    
    return changesMade;
}

// Apply Resolution: P ∨ Q, ¬P ∨ R ⊢ Q ∨ R
// Creates a new disjunction when two disjunctions share a complementary literal
bool InferenceEngine::applyResolution(const Proposition& disj1,
                                       const Proposition& disj2,
                                       std::unordered_map<std::string, Proposition>& propositions) {
    // disj1: P ∨ Q (antecedent = P, consequent = Q)
    // disj2: ¬P ∨ R (antecedent = ¬P, consequent = R)
    // Result: Q ∨ R
    
    const std::string& p1Left = disj1.getAntecedent();
    const std::string& p1Right = disj1.getConsequent();
    const std::string& p2Left = disj2.getAntecedent();
    const std::string& p2Right = disj2.getConsequent();
    
    bool changesMade = false;
    
    // Check all four possible complementary pairs
    auto tryResolve = [&](const std::string& lit1, const std::string& other1,
                          const std::string& lit2, const std::string& other2) -> bool {
        // Check if lit1 and lit2 are complementary (one is negation of the other)
        std::string baseLit1 = getBaseName(lit1);
        std::string baseLit2 = getBaseName(lit2);
        bool lit1Negated = isNegatedName(lit1);
        bool lit2Negated = isNegatedName(lit2);
        
        // They are complementary if same base name but different polarity
        if (baseLit1 == baseLit2 && lit1Negated != lit2Negated) {
            // Get truth values of the remaining disjuncts
            const Proposition* other1Prop = findProposition(other1, propositions);
            const Proposition* other2Prop = findProposition(other2, propositions);
            
            Tripartite other1Truth = other1Prop ? other1Prop->getTruthValue() : Tripartite::UNKNOWN;
            Tripartite other2Truth = other2Prop ? other2Prop->getTruthValue() : Tripartite::UNKNOWN;
            
            // If one of the resolved disjuncts is FALSE, the other must be TRUE
            if (other1Truth == Tripartite::FALSE && other2Truth != Tripartite::TRUE) {
                Proposition* other2PropMut = findProposition(other2, propositions);
                InferenceProvenance prov("Resolution", 
                                          {disj1.getPrefix(), disj2.getPrefix(), other1});
                if (other2PropMut) {
                    other2PropMut->setTruthValue(Tripartite::TRUE, prov);
                } else {
                    propositions[other2].setTruthValue(Tripartite::TRUE, prov);
                }
                return true;
            }
            
            if (other2Truth == Tripartite::FALSE && other1Truth != Tripartite::TRUE) {
                Proposition* other1PropMut = findProposition(other1, propositions);
                InferenceProvenance prov("Resolution", 
                                          {disj1.getPrefix(), disj2.getPrefix(), other2});
                if (other1PropMut) {
                    other1PropMut->setTruthValue(Tripartite::TRUE, prov);
                } else {
                    propositions[other1].setTruthValue(Tripartite::TRUE, prov);
                }
                return true;
            }
        }
        return false;
    };
    
    // Try all four combinations
    if (tryResolve(p1Left, p1Right, p2Left, p2Right)) changesMade = true;
    if (tryResolve(p1Left, p1Right, p2Right, p2Left)) changesMade = true;
    if (tryResolve(p1Right, p1Left, p2Left, p2Right)) changesMade = true;
    if (tryResolve(p1Right, p1Left, p2Right, p2Left)) changesMade = true;
    
    return changesMade;
}

void InferenceEngine::deduceAll(std::unordered_map<std::string, Proposition>& propositions,
                                std::vector<Expression>& expressions) {
    bool changesMade;
    do {
        changesMade = false;
        
        // ============================================================
        // PHASE 1: Apply basic inference rules to IMPLIES propositions
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
        // PHASE 2: Apply Hypothetical Syllogism to pairs of implications
        // P → Q, Q → R ⊢ derives truth through the chain
        // ============================================================
        std::vector<const Proposition*> implications;
        for (const auto& entry : propositions) {
            if (entry.second.getRelation() == LogicalOperator::IMPLIES) {
                implications.push_back(&entry.second);
            }
        }
        
        for (size_t i = 0; i < implications.size(); ++i) {
            for (size_t j = 0; j < implications.size(); ++j) {
                if (i != j) {
                    if (applyHypotheticalSyllogism(*implications[i], *implications[j], propositions)) {
                        changesMade = true;
                    }
                }
            }
        }
        
        // ============================================================
        // PHASE 3: Apply Disjunctive Syllogism to OR propositions
        // P ∨ Q, ¬P ⊢ Q
        // ============================================================
        for (auto& entry : propositions) {
            const Proposition& prop = entry.second;
            if (prop.getRelation() == LogicalOperator::OR) {
                if (applyDisjunctiveSyllogism(prop, propositions)) {
                    changesMade = true;
                }
            }
        }
        
        // ============================================================
        // PHASE 4: Apply Resolution to pairs of OR propositions
        // P ∨ Q, ¬P ∨ R ⊢ Q ∨ R
        // ============================================================
        std::vector<const Proposition*> disjunctions;
        for (const auto& entry : propositions) {
            if (entry.second.getRelation() == LogicalOperator::OR) {
                disjunctions.push_back(&entry.second);
            }
        }
        
        for (size_t i = 0; i < disjunctions.size(); ++i) {
            for (size_t j = i + 1; j < disjunctions.size(); ++j) {
                if (applyResolution(*disjunctions[i], *disjunctions[j], propositions)) {
                    changesMade = true;
                }
            }
        }
        
        // ============================================================
        // PHASE 5: Evaluate explicit Expression objects (if any)
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

