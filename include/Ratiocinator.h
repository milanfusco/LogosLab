#ifndef RATIOCINATOR_H
#define RATIOCINATOR_H

#include "Expression.h"
#include <vector>
#include <unordered_map>
#include <string>

class Ratiocinator {
private:
    // Helper methods for specific logical inference rules
    bool applyModusPonens(Proposition& proposition);
    bool applyModusTollens(Proposition& proposition);
    // TODO: Add more inference rules here
public:
    // Constructors and Destructor
    Ratiocinator();
    ~Ratiocinator();

    std::unordered_map<std::string, Proposition> propositions; // Stores all propositions by name
    std::vector<Expression> expressions;                       // Collection of expressions to evaluate


    // Load expressions from facts and arguments files
    void parseFactsFile(const std::string& filename);
    void parseArgumentsFile(const std::string& filename);

    // Add an expression to the list
    void addExpression(const Expression& expr);

    // Deduce truth values of all propositions based on expressions
    void deduceAll();

    // Output the final truth values of each proposition
    void outputTruthValues() const;


};

#endif // RATIOCINATOR_H
