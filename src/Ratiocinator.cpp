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
    inferenceEngine_.deduceAll(propositions_, expressions_);
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

