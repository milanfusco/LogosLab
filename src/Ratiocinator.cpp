#include "Ratiocinator.h"
#include "Expression.h"
#include "Proposition.h"
#include <iostream>
#include <sstream>

// ========== Facade Methods (Primary API) ==========

void Ratiocinator::loadAssumptions(const std::string& filename) {
    auto parsed = parser_.parseAssumptionsFile(filename);
    for (auto& entry : parsed) {
        propositions_[entry.first] = std::move(entry.second);
    }
}

void Ratiocinator::loadFacts(const std::string& filename) {
    parser_.parseFactsFile(filename, propositions_);
}

void Ratiocinator::deduce() {
    inferenceEngine_.deduceAll(propositions_, expressions_);
}

std::string Ratiocinator::formatResults() const {
    std::ostringstream oss;
    for (const auto& entry : propositions_) {
        const auto& name = entry.first;
        const auto& prop = entry.second;
        
        oss << name << ": ";
        switch (prop.getTruthValue()) {
            case Tripartite::TRUE:
                oss << "True";
                break;
            case Tripartite::FALSE:
                oss << "False";
                break;
            case Tripartite::UNKNOWN:
                oss << "Unknown";
                break;
        }
        oss << "\n";
    }
    return oss.str();
}

// ========== Legacy Methods (Backward Compatibility) ==========

void Ratiocinator::parseAssumptionsFile(const std::string& filename) {
    loadAssumptions(filename);
}

void Ratiocinator::parseFactsFile(const std::string& filename) {
    loadFacts(filename);
}

void Ratiocinator::deduceAll() {
    deduce();
}

std::string Ratiocinator::outputTruthValues() const {
    // Legacy behavior: prints to cout AND returns string
    std::string output = formatResults();
    std::cout << output;
    return output;
}

// ========== Proposition Accessors ==========

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
