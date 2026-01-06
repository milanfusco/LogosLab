#ifndef RATIOCINATOR_H
#define RATIOCINATOR_H

#include "Expression.h"
#include "Parser.h"
#include "InferenceEngine.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>

/**
 * Sorting options for result output.
 */
enum class ResultSortOrder {
    NONE,           ///< No sorting (hash map order)
    ALPHABETICAL,   ///< Sort by proposition name A-Z
    ALPHABETICAL_DESC, ///< Sort by proposition name Z-A
    BY_TRUTH_VALUE, ///< Group by truth value (TRUE, FALSE, UNKNOWN)
    BY_DERIVATION   ///< Group by derivation (derived first, then axioms)
};

/**
 * Filter criteria for result output.
 * Use to limit and organize output from formatResults().
 * 
 * Example usage:
 *   ResultFilter filter;
 *   filter.showTrue = true;
 *   filter.showFalse = false;
 *   filter.showUnknown = false;
 *   filter.prefixPattern = "user_";
 *   std::string results = engine.formatResults(filter);
 */
struct ResultFilter {
    // Truth value filters (default: show all)
    bool showTrue = true;       ///< Include propositions with TRUE value
    bool showFalse = true;      ///< Include propositions with FALSE value
    bool showUnknown = true;    ///< Include propositions with UNKNOWN value
    
    // Derivation filters
    bool showDerived = true;    ///< Include propositions derived by inference
    bool showAxioms = true;     ///< Include axioms (direct assertions)
    
    // Name filters
    std::string prefixPattern;  ///< Only include names starting with this prefix (empty = all)
    std::string containsPattern; ///< Only include names containing this substring (empty = all)
    
    // Output options
    ResultSortOrder sortOrder = ResultSortOrder::ALPHABETICAL;
    size_t limit = 0;           ///< Max results to return (0 = unlimited)
    bool includeTraces = false; ///< Include inference traces in output
    bool showProvenance = true; ///< Show "[derived via X]" annotations
    
    // Custom filter function (optional)
    std::function<bool(const std::string&, const Proposition&)> customFilter;
    
    /// Check if a proposition passes all filters
    bool matches(const std::string& name, const Proposition& prop) const;
    
    // Builder-style setters for fluent API
    ResultFilter& withTruthValues(bool t, bool f, bool u) {
        showTrue = t; showFalse = f; showUnknown = u;
        return *this;
    }
    ResultFilter& withPrefix(const std::string& prefix) {
        prefixPattern = prefix;
        return *this;
    }
    ResultFilter& withContains(const std::string& pattern) {
        containsPattern = pattern;
        return *this;
    }
    ResultFilter& withLimit(size_t n) {
        limit = n;
        return *this;
    }
    ResultFilter& withSort(ResultSortOrder order) {
        sortOrder = order;
        return *this;
    }
    ResultFilter& withTraces(bool traces = true) {
        includeTraces = traces;
        return *this;
    }
    ResultFilter& derivedOnly() {
        showDerived = true; showAxioms = false;
        return *this;
    }
    ResultFilter& axiomsOnly() {
        showDerived = false; showAxioms = true;
        return *this;
    }
    
    /// Create a filter that only shows TRUE propositions
    static ResultFilter trueOnly() {
        ResultFilter f;
        f.showTrue = true; f.showFalse = false; f.showUnknown = false;
        return f;
    }
    
    /// Create a filter that only shows FALSE propositions
    static ResultFilter falseOnly() {
        ResultFilter f;
        f.showTrue = false; f.showFalse = true; f.showUnknown = false;
        return f;
    }
    
    /// Create a filter that only shows known (TRUE or FALSE) propositions
    static ResultFilter knownOnly() {
        ResultFilter f;
        f.showTrue = true; f.showFalse = true; f.showUnknown = false;
        return f;
    }
    
    /// Create a filter that only shows UNKNOWN propositions
    static ResultFilter unknownOnly() {
        ResultFilter f;
        f.showTrue = false; f.showFalse = false; f.showUnknown = true;
        return f;
    }
};

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
    
    /// Format proposition truth values with filtering and sorting
    /// @param filter Filter criteria for selecting and ordering results
    /// @return Formatted string with filtered results
    std::string formatResults(const ResultFilter& filter) const;

    /// Print formatted results to stdout
    /// @param includeTraces If true, includes inference traces for derived propositions
    void printResults(bool includeTraces = false) const;
    
    /// Print filtered results to stdout
    /// @param filter Filter criteria for selecting and ordering results
    void printResults(const ResultFilter& filter) const;
    
    /// Get filtered list of proposition names
    /// @param filter Filter criteria
    /// @return Vector of proposition names matching the filter
    std::vector<std::string> getFilteredPropositionNames(const ResultFilter& filter) const;

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
