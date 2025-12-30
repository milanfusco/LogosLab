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
    // Use the enhanced parseFactsFile that builds Expression objects
    // from complex facts and populates the expressions_ vector
    parser_.parseFactsFile(filename, propositions_, expressions_);
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

void Ratiocinator::printResults() const {
    std::cout << formatResults();
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

// ========== Incremental Update API ==========

bool Ratiocinator::addProposition(const std::string& name, const Proposition& prop) {
    // Only add if it doesn't already exist
    if (hasProposition(name)) {
        return false;
    }
    propositions_[name] = prop;
    return true;
}

bool Ratiocinator::removeProposition(const std::string& name) {
    auto it = propositions_.find(name);
    if (it == propositions_.end()) {
        return false;
    }
    propositions_.erase(it);
    return true;
}

bool Ratiocinator::updatePropositionTruthValue(const std::string& name,
                                                Tripartite value,
                                                const InferenceProvenance& provenance) {
    auto it = propositions_.find(name);
    if (it == propositions_.end()) {
        return false;
    }
    it->second.setTruthValue(value, provenance);
    return true;
}

void Ratiocinator::clearPropositions() {
    propositions_.clear();
}

void Ratiocinator::clearKnowledgeBase() {
    propositions_.clear();
    expressions_.clear();
}

size_t Ratiocinator::getPropositionCount() const {
    return propositions_.size();
}

size_t Ratiocinator::getExpressionCount() const {
    return expressions_.size();
}

// ========== Expression Accessors ==========

void Ratiocinator::addExpression(const Expression& expr) {
    expressions_.push_back(expr);
}

Expression Ratiocinator::addExpressionFromString(const std::string& exprString,
                                                  const std::string& prefix) {
    Expression expr = parser_.parseExpressionString(exprString, propositions_, prefix);
    expressions_.push_back(expr);
    return expr;
}

const std::vector<Expression>& Ratiocinator::getExpressions() const {
    return expressions_;
}

void Ratiocinator::clearExpressions() {
    expressions_.clear();
}
