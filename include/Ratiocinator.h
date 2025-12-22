#ifndef RATIOCINATOR_H
#define RATIOCINATOR_H

#include "Expression.h"
#include <vector>
#include <unordered_map>
#include <string>

class Ratiocinator {
private:
    std::unordered_map<std::string, Proposition> propositions_; // Stores all propositions by name
    std::vector<Expression> expressions_;                       // Collection of expressions to evaluate

    // Safe internal helper to find proposition (returns nullptr if not found)
    Proposition* findProposition(const std::string& name);
    const Proposition* findProposition(const std::string& name) const;

    // Helper methods for specific logical inference rules
    bool applyModusPonens(const Proposition& proposition);
    bool applyModusTollens(const Proposition& proposition);
    // TODO: Add more inference rules here

public:
    // Constructors and Destructor
    Ratiocinator();
    ~Ratiocinator();

    // Proposition accessors
    void setProposition(const std::string& name, const Proposition& prop);
    const Proposition* getProposition(const std::string& name) const;
    Proposition* getProposition(const std::string& name);
    bool hasProposition(const std::string& name) const;
    void setPropositionTruthValue(const std::string& name, Tripartite value);
    Tripartite getPropositionTruthValue(const std::string& name) const;
    const std::unordered_map<std::string, Proposition>& getPropositions() const;

    // Expression accessors
    void addExpression(const Expression& expr);
    const std::vector<Expression>& getExpressions() const;

    // Load expressions from facts and arguments files
    void parseAssumptionsFile(const std::string& filename);
    void parseFactsFile(const std::string& filename);

    // Deduce truth values of all propositions based on expressions
    void deduceAll();

    // Output the final truth values of each proposition
    std::string outputTruthValues() const;
};

#endif // RATIOCINATOR_H
