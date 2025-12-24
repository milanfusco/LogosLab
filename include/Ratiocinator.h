#ifndef RATIOCINATOR_H
#define RATIOCINATOR_H

#include "Expression.h"
#include "Parser.h"
#include "InferenceEngine.h"
#include <vector>
#include <unordered_map>
#include <string>

/**
 * Ratiocinator - The main reasoning engine facade.
 * 
 * Provides a unified interface for logical reasoning by coordinating:
 * - Parser: Handles file I/O and parsing of assumptions/facts
 * - InferenceEngine: Applies logical inference rules (Modus Ponens, Modus Tollens)
 * 
 * Usage:
 *   Ratiocinator engine;
 *   engine.loadAssumptions("assumptions.txt");
 *   engine.loadFacts("facts.txt");
 *   engine.deduce();
 *   std::string results = engine.formatResults();
 */
class Ratiocinator {
private:
    Parser parser_;                                              // Handles file parsing
    InferenceEngine inferenceEngine_;                            // Handles logical deduction
    std::unordered_map<std::string, Proposition> propositions_; // Knowledge base
    std::vector<Expression> expressions_;                       // Expressions to evaluate

public:
    Ratiocinator() = default;
    ~Ratiocinator() = default;

    // ========== Facade Methods (Primary API) ==========
    
    /// Load proposition definitions from an assumptions file
    void loadAssumptions(const std::string& filename);
    
    /// Load truth values from a facts file
    void loadFacts(const std::string& filename);
    
    /// Run the inference engine to deduce all possible truth values
    void deduce();
    
    /// Format all proposition truth values as a string (no side effects)
    std::string formatResults() const;

    /// Print formatted results to stdout
    void printResults() const;

    // ========== Proposition Accessors ==========
    
    void setProposition(const std::string& name, const Proposition& prop);
    const Proposition* getProposition(const std::string& name) const;
    Proposition* getProposition(const std::string& name);
    bool hasProposition(const std::string& name) const;
    void setPropositionTruthValue(const std::string& name, Tripartite value);
    Tripartite getPropositionTruthValue(const std::string& name) const;
    const std::unordered_map<std::string, Proposition>& getPropositions() const;

    // ========== Expression Accessors ==========
    
    void addExpression(const Expression& expr);
    const std::vector<Expression>& getExpressions() const;
};

#endif // RATIOCINATOR_H
