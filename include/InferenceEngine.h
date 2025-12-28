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
 */
class InferenceEngine {
private:
    // Safe internal helper to find proposition (returns nullptr if not found)
    static Proposition* findProposition(const std::string& name,
                                        std::unordered_map<std::string, Proposition>& propositions);
    static const Proposition* findProposition(const std::string& name,
                                              const std::unordered_map<std::string, Proposition>& propositions);

    // Helper methods for specific logical inference rules
    // Returns true if a change was made
    static bool applyModusPonens(const Proposition& implication,
                                 std::unordered_map<std::string, Proposition>& propositions);
    static bool applyModusTollens(const Proposition& implication,
                                  std::unordered_map<std::string, Proposition>& propositions);
    // TODO: Add more inference rules here (e.g., Hypothetical Syllogism, Disjunctive Syllogism)

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

