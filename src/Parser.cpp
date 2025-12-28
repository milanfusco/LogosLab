#include "Parser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>

// Constructor: register built-in handlers
Parser::Parser() {
    registerBuiltinHandlers();
}

// Register all built-in relation handlers
void Parser::registerBuiltinHandlers() {
    registerRelation("implies", handleImplies);
    registerRelation("some", handleSome);
    registerRelation("not", handleNot);
    registerRelation("discovered", handleDiscovered);
}

// Built-in handler: implies(antecedent, subject, consequent, predicate)
bool Parser::handleImplies(const std::string& prefix,
                           const std::vector<std::string>& args,
                           std::unordered_map<std::string, Proposition>& propositions) {
    if (args.size() != 4) return false;
    
    Proposition proposition;
    proposition.setPrefix(prefix);
    proposition.setRelation(LogicalOperator::IMPLIES);
    proposition.setAntecedent(args[0]);
    proposition.setSubject(args[1]);
    proposition.setConsequent(args[2]);
    proposition.setPredicate(args[3]);
    proposition.setPropositionScope(Quantifier::UNIVERSAL_AFFIRMATIVE);
    propositions[args[2]] = proposition;  // Key is consequent
    return true;
}

// Built-in handler: some(subject, predicate)
bool Parser::handleSome(const std::string& prefix,
                        const std::vector<std::string>& args,
                        std::unordered_map<std::string, Proposition>& propositions) {
    if (args.size() != 2) return false;
    
    Proposition proposition;
    proposition.setPrefix(prefix);
    proposition.setRelation(LogicalOperator::NONE);
    proposition.setSubject(args[0]);
    proposition.setPredicate(args[1]);
    proposition.setTruthValue(Tripartite::TRUE);
    proposition.setPropositionScope(Quantifier::PARTICULAR_AFFIRMATIVE);
    propositions[args[0]] = proposition;  // Key is subject
    return true;
}

// Built-in handler: not(subject)
bool Parser::handleNot(const std::string& prefix,
                       const std::vector<std::string>& args,
                       std::unordered_map<std::string, Proposition>& propositions) {
    if (args.size() != 1) return false;
    
    Proposition proposition;
    proposition.setPrefix(prefix);
    proposition.setRelation(LogicalOperator::NOT);
    proposition.setSubject(args[0]);
    proposition.setTruthValue(Tripartite::FALSE);
    proposition.setPropositionScope(Quantifier::UNIVERSAL_NEGATIVE);
    propositions[args[0]] = proposition;  // Key is subject
    return true;
}

// Built-in handler: discovered(subject, predicate)
bool Parser::handleDiscovered(const std::string& prefix,
                              const std::vector<std::string>& args,
                              std::unordered_map<std::string, Proposition>& propositions) {
    if (args.size() != 2) return false;
    
    Proposition proposition;
    proposition.setPrefix(prefix);
    proposition.setRelation(LogicalOperator::NONE);
    proposition.setSubject(args[0]);
    proposition.setPredicate(args[1]);
    propositions[args[0]] = proposition;  // Key is subject
    return true;
}

// Register a custom relation handler
void Parser::registerRelation(const std::string& relationName, RelationHandler handler) {
    relationHandlers_[relationName] = std::move(handler);
}

// Unregister a relation handler
bool Parser::unregisterRelation(const std::string& relationName) {
    return relationHandlers_.erase(relationName) > 0;
}

// Check if a relation handler is registered
bool Parser::hasRelation(const std::string& relationName) const {
    return relationHandlers_.find(relationName) != relationHandlers_.end();
}

// Get list of registered relation names
std::vector<std::string> Parser::getRegisteredRelations() const {
    std::vector<std::string> relations;
    relations.reserve(relationHandlers_.size());
    for (const auto& entry : relationHandlers_) {
        relations.push_back(entry.first);
    }
    return relations;
}

// Parse assumptions file using registered handlers
std::unordered_map<std::string, Proposition> Parser::parseAssumptionsFile(const std::string& filename) {
    std::unordered_map<std::string, Proposition> propositions;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return propositions;
    }

    std::string line;
    // Regex pattern to match lines like "prefix, relation(arg1, arg2, arg3, arg4)"
    std::regex linePattern(R"(^\s*(\w+)\s*,\s*(\w+)\s*\(\s*([-\w\s\d,]+?)\s*\)\s*$)");
    std::smatch match;

    while (std::getline(file, line)) {
        if (std::regex_match(line, match, linePattern)) {
            std::string prefix = match[1];
            std::string relation = match[2];
            std::string arguments = match[3];

            // Split arguments
            std::istringstream argsStream(arguments);
            std::vector<std::string> args;
            std::string arg;
            while (std::getline(argsStream, arg, ',')) {
                arg.erase(0, arg.find_first_not_of(" \t"));
                arg.erase(arg.find_last_not_of(" \t") + 1);
                args.push_back(arg);
            }

            // Look up and invoke the registered handler
            auto it = relationHandlers_.find(relation);
            if (it != relationHandlers_.end()) {
                bool success = it->second(prefix, args, propositions);
                if (!success) {
                    std::cerr << "Warning: Handler for '" << relation 
                              << "' failed to process: " << line << std::endl;
                }
            } else {
                std::cerr << "Warning: Unknown relation type '" << relation 
                          << "' in line: " << line << std::endl;
            }
        } else {
            std::cerr << "Error parsing line: " << line 
                      << " (check for unbalanced parentheses or incorrect format)" << std::endl;
        }
    }
    file.close();
    
    return propositions;
}

void Parser::parseFactsFile(const std::string& filename, 
                            std::unordered_map<std::string, Proposition>& propositions) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string token;
        bool negate = false;

        // Handle each token and determine the operation or assignment
        while (lineStream >> token) {
            if (token == "!") {
                negate = true;
            } else if (token == "&&" || token == "||" || token == "=") {
                // Logical operators or assignment operators; handled below
            } else {
                // Assuming token is a proposition identifier
                if (negate) {
                    propositions[token].setTruthValue(Tripartite::FALSE);
                    negate = false;
                } else {
                    propositions[token].setTruthValue(Tripartite::TRUE);
                }
            }
        }
    }
    file.close();
}

