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

std::string Ratiocinator::formatResults(bool includeTraces) const {
    std::ostringstream oss;
    
    oss << "=== Proposition Truth Values ===\n";
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
        
        // Add provenance indicator
        if (prop.hasProvenance()) {
            oss << " [derived via " << prop.getProvenance()->ruleFired << "]";
        }
        oss << "\n";
    }
    
    if (includeTraces) {
        oss << "\n" << formatAllTraces();
    }
    
    return oss.str();
}

void Ratiocinator::printResults(bool includeTraces) const {
    std::cout << formatResults(includeTraces);
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

// ========== Inference Tracing API ==========

void Ratiocinator::traceInferenceRecursive(const std::string& name,
                                            std::vector<InferenceStep>& trace,
                                            std::unordered_set<std::string>& visited,
                                            int depth) const {
    // Avoid infinite recursion on circular references
    if (visited.count(name) > 0) {
        return;
    }
    visited.insert(name);
    
    // Find the proposition
    auto it = propositions_.find(name);
    if (it == propositions_.end()) {
        return;
    }
    
    const Proposition& prop = it->second;
    
    // Create an inference step for this proposition
    InferenceStep step;
    step.proposition = name;
    step.truthValue = prop.getTruthValue();
    step.depth = depth;
    
    if (prop.hasProvenance()) {
        const auto& prov = prop.getProvenance();
        step.rule = prov->ruleFired;
        step.premises = prov->premises;
    } else {
        // No provenance - this is an axiom or direct assertion
        step.rule = "Axiom";
        step.premises.clear();
    }
    
    trace.push_back(step);
    
    // Recursively trace premises
    if (prop.hasProvenance()) {
        for (const auto& premise : prop.getProvenance()->premises) {
            traceInferenceRecursive(premise, trace, visited, depth + 1);
        }
    }
}

std::vector<InferenceStep> Ratiocinator::traceInference(const std::string& name) const {
    std::vector<InferenceStep> trace;
    std::unordered_set<std::string> visited;
    
    traceInferenceRecursive(name, trace, visited, 0);
    
    return trace;
}

std::string Ratiocinator::formatTrace(const std::string& name) const {
    std::vector<InferenceStep> trace = traceInference(name);
    
    if (trace.empty()) {
        return "No inference trace available for '" + name + "'\n";
    }
    
    std::ostringstream oss;
    oss << "Inference trace for '" << name << "':\n";
    oss << "========================================\n";
    
    for (const auto& step : trace) {
        // Indent based on depth
        std::string indent(step.depth * 2, ' ');
        
        // Format truth value
        std::string truthStr;
        switch (step.truthValue) {
            case Tripartite::TRUE:    truthStr = "TRUE"; break;
            case Tripartite::FALSE:   truthStr = "FALSE"; break;
            case Tripartite::UNKNOWN: truthStr = "UNKNOWN"; break;
        }
        
        oss << indent << step.proposition << " = " << truthStr;
        
        if (step.rule == "Axiom") {
            oss << " [Axiom/Direct Assertion]\n";
        } else {
            oss << " [" << step.rule << "]\n";
            
            if (!step.premises.empty()) {
                oss << indent << "  Premises: ";
                for (size_t i = 0; i < step.premises.size(); ++i) {
                    if (i > 0) oss << ", ";
                    oss << step.premises[i];
                }
                oss << "\n";
            }
        }
    }
    
    oss << "========================================\n";
    return oss.str();
}

bool Ratiocinator::hasInferenceProvenance(const std::string& name) const {
    auto it = propositions_.find(name);
    if (it == propositions_.end()) {
        return false;
    }
    return it->second.hasProvenance();
}

void Ratiocinator::printTrace(const std::string& name) const {
    std::cout << formatTrace(name);
}

std::string Ratiocinator::formatAllTraces() const {
    std::ostringstream oss;
    
    // Collect propositions that have provenance (were derived)
    std::vector<std::string> derivedProps;
    for (const auto& entry : propositions_) {
        if (entry.second.hasProvenance()) {
            derivedProps.push_back(entry.first);
        }
    }
    
    if (derivedProps.empty()) {
        oss << "=== Inference Traces ===\n";
        oss << "No derived propositions (all are axioms or unknown).\n";
        return oss.str();
    }
    
    oss << "=== Inference Traces ===\n";
    oss << "(" << derivedProps.size() << " derived proposition(s))\n\n";
    
    for (const auto& name : derivedProps) {
        oss << formatTrace(name) << "\n";
    }
    
    return oss.str();
}

void Ratiocinator::printAllTraces() const {
    std::cout << formatAllTraces();
}
