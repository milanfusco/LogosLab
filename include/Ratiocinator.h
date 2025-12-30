#ifndef RATIOCINATOR_H
#define RATIOCINATOR_H

#include "Expression.h"
#include "Parser.h"
#include "InferenceEngine.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>

/**
 * InferenceStep represents a single step in an inference trace.
 * Used to explain how a proposition's truth value was derived.
 */
struct InferenceStep {
    std::string proposition;              ///< The proposition that was derived
    Tripartite truthValue;                ///< The truth value that was derived
    std::string rule;                     ///< The inference rule used (e.g., "ModusPonens")
    std::vector<std::string> premises;    ///< The premises used in this step
    int depth;                            ///< Depth in the inference chain (0 = target)
    
    InferenceStep() 
        : proposition(""), truthValue(Tripartite::UNKNOWN), rule(""), premises(), depth(0) {}
    
    InferenceStep(const std::string& prop, Tripartite value, 
                  const std::string& r, const std::vector<std::string>& prem, int d)
        : proposition(prop), truthValue(value), rule(r), premises(prem), depth(d) {}
};

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

    /// Helper for recursive inference tracing
    void traceInferenceRecursive(const std::string& name,
                                  std::vector<InferenceStep>& trace,
                                  std::unordered_set<std::string>& visited,
                                  int depth) const;

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
    /// @param includeTraces If true, includes inference traces for derived propositions
    std::string formatResults(bool includeTraces = false) const;

    /// Print formatted results to stdout
    /// @param includeTraces If true, includes inference traces for derived propositions
    void printResults(bool includeTraces = false) const;

    // ========== Proposition Accessors ==========
    
    /// Set or update a proposition (creates if doesn't exist)
    void setProposition(const std::string& name, const Proposition& prop);
    
    /// Get a proposition by name (returns nullptr if not found)
    const Proposition* getProposition(const std::string& name) const;
    Proposition* getProposition(const std::string& name);
    
    /// Check if a proposition exists
    bool hasProposition(const std::string& name) const;
    
    /// Set the truth value of a proposition (creates if doesn't exist)
    void setPropositionTruthValue(const std::string& name, Tripartite value);
    
    /// Get the truth value of a proposition (returns UNKNOWN if not found)
    Tripartite getPropositionTruthValue(const std::string& name) const;
    
    /// Get all propositions
    const std::unordered_map<std::string, Proposition>& getPropositions() const;

    // ========== Incremental Update API ==========
    
    /// Add a new proposition (returns false if already exists)
    /// @param name The name/key for the proposition
    /// @param prop The proposition to add
    /// @return true if added, false if a proposition with that name already exists
    bool addProposition(const std::string& name, const Proposition& prop);
    
    /// Remove a proposition by name
    /// @param name The name of the proposition to remove
    /// @return true if removed, false if not found
    bool removeProposition(const std::string& name);
    
    /// Update the truth value of an existing proposition with provenance
    /// @param name The name of the proposition to update
    /// @param value The new truth value
    /// @param provenance Optional provenance information for tracking
    /// @return true if updated, false if proposition not found
    bool updatePropositionTruthValue(const std::string& name, 
                                     Tripartite value,
                                     const InferenceProvenance& provenance);
    
    /// Clear all propositions from the knowledge base
    void clearPropositions();
    
    /// Clear the entire knowledge base (propositions and expressions)
    void clearKnowledgeBase();
    
    /// Get the number of propositions in the knowledge base
    size_t getPropositionCount() const;
    
    /// Get the number of expressions in the knowledge base
    size_t getExpressionCount() const;

    // ========== Expression Accessors ==========
    
    /// Add a pre-built expression to the knowledge base
    void addExpression(const Expression& expr);
    
    /// Parse and add an expression from a string (e.g., "A && B -> C")
    /// @param exprString The expression to parse
    /// @param prefix Optional name/prefix for the expression
    /// @return The parsed and added expression
    Expression addExpressionFromString(const std::string& exprString, 
                                       const std::string& prefix = "");
    
    /// Get all expressions in the knowledge base
    const std::vector<Expression>& getExpressions() const;
    
    /// Clear all expressions
    void clearExpressions();

    // ========== Inference Tracing API ==========
    
    /**
     * Trace the inference chain that led to a proposition's truth value.
     * Walks backwards through provenance data to build an explanation.
     * 
     * @param name The name of the proposition to trace
     * @return Vector of InferenceSteps from target (depth 0) to axioms (max depth)
     */
    std::vector<InferenceStep> traceInference(const std::string& name) const;
    
    /**
     * Format an inference trace as a human-readable string.
     * 
     * @param name The name of the proposition to trace
     * @return Formatted multi-line string explaining the inference chain
     */
    std::string formatTrace(const std::string& name) const;
    
    /**
     * Check if a proposition has provenance (was derived by inference).
     * 
     * @param name The name of the proposition
     * @return true if the proposition has provenance data
     */
    bool hasInferenceProvenance(const std::string& name) const;
    
    /**
     * Print an inference trace for a proposition to stdout.
     * 
     * @param name The name of the proposition to trace
     */
    void printTrace(const std::string& name) const;
    
    /**
     * Format all inference traces for derived propositions.
     * 
     * @return Multi-line string with all traces
     */
    std::string formatAllTraces() const;
    
    /**
     * Print all inference traces for derived propositions to stdout.
     */
    void printAllTraces() const;
};

#endif // RATIOCINATOR_H
