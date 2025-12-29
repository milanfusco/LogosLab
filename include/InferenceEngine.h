#ifndef INFERENCE_ENGINE_H
#define INFERENCE_ENGINE_H

#include "Expression.h"
#include "Proposition.h"
#include <unordered_map>
#include <vector>
#include <string>

/**
 * InferenceEngine class responsible for logical deduction.
 * Applies inference rules (Modus Ponens, Modus Tollens, etc.) to derive
 * new truth values from existing propositions and expressions.
 * 
 * Supported inference rules:
 * - Modus Ponens:          P → Q, P ⊢ Q
 * - Modus Tollens:         P → Q, ¬Q ⊢ ¬P
 * - Hypothetical Syllogism: P → Q, Q → R ⊢ P → R (with truth propagation)
 * - Disjunctive Syllogism: P ∨ Q, ¬P ⊢ Q
 * - Resolution:            P ∨ Q, ¬P ∨ R ⊢ Q ∨ R
 */
class InferenceEngine {
private:
    // Safe internal helper to find proposition (returns nullptr if not found)
    static Proposition* findProposition(const std::string& name,
                                        std::unordered_map<std::string, Proposition>& propositions);
    static const Proposition* findProposition(const std::string& name,
                                              const std::unordered_map<std::string, Proposition>& propositions);

    // ========== Basic Inference Rules ==========
    
    /// Modus Ponens: P → Q, P is TRUE ⊢ Q is TRUE
    static bool applyModusPonens(const Proposition& implication,
                                 std::unordered_map<std::string, Proposition>& propositions);
    
    /// Modus Tollens: P → Q, Q is FALSE ⊢ P is FALSE
    static bool applyModusTollens(const Proposition& implication,
                                  std::unordered_map<std::string, Proposition>& propositions);

    // ========== Extended Inference Rules ==========
    
    /// Hypothetical Syllogism: P → Q, Q → R ⊢ P → R
    /// When P is TRUE, transitively derives R is TRUE
    static bool applyHypotheticalSyllogism(const Proposition& impl1,
                                           const Proposition& impl2,
                                           std::unordered_map<std::string, Proposition>& propositions);

    /// Disjunctive Syllogism: P ∨ Q, ¬P ⊢ Q (and P ∨ Q, ¬Q ⊢ P)
    /// When one disjunct is FALSE, the other is TRUE
    static bool applyDisjunctiveSyllogism(const Proposition& disjunction,
                                          std::unordered_map<std::string, Proposition>& propositions);

    /// Resolution: P ∨ Q, ¬P ∨ R ⊢ Q ∨ R
    /// Creates derived disjunctions from complementary literals
    static bool applyResolution(const Proposition& disj1,
                                const Proposition& disj2,
                                std::unordered_map<std::string, Proposition>& propositions);

    // ========== Helper Methods ==========
    
    /// Check if a proposition name represents a negation (starts with "~" or "!")
    static bool isNegatedName(const std::string& name);
    
    /// Get the base name without negation prefix
    static std::string getBaseName(const std::string& name);
    
    /// Get the negated form of a name
    static std::string getNegatedName(const std::string& name);

public:
    InferenceEngine() = default;
    ~InferenceEngine() = default;

    /**
     * Deduce truth values of all propositions based on inference rules and expressions.
     * Iterates until no more changes can be made (fixed-point iteration).
     * 
     * @param propositions Map of proposition names to Proposition objects (modified in-place)
     * @param expressions Vector of Expression objects to evaluate
     */
    void deduceAll(std::unordered_map<std::string, Proposition>& propositions,
                   std::vector<Expression>& expressions);
};

#endif // INFERENCE_ENGINE_H

