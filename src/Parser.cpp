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

// ========== Expression Parsing Helpers ==========

LogicalOperator Parser::tokenTypeToLogicalOperator(TokenType type) {
    switch (type) {
        case TokenType::AND:        return LogicalOperator::AND;
        case TokenType::OR:         return LogicalOperator::OR;
        case TokenType::NOT:        return LogicalOperator::NOT;
        case TokenType::IMPLIES:    return LogicalOperator::IMPLIES;
        case TokenType::EQUIVALENT: return LogicalOperator::EQUIVALENT;
        case TokenType::LPAREN:     return LogicalOperator::LPAREN;
        case TokenType::RPAREN:     return LogicalOperator::RPAREN;
        default:                    return LogicalOperator::NONE;
    }
}

Expression Parser::buildExpression(const std::vector<LexerToken>& tokens,
                                   const std::unordered_map<std::string, Proposition>& propositions,
                                   const std::string& prefix) {
    Expression expr;
    expr.setPrefix(prefix);
    
    for (const auto& token : tokens) {
        switch (token.type) {
            case TokenType::IDENTIFIER: {
                // Look up the proposition's truth value
                Proposition prop;
                auto it = propositions.find(token.value);
                if (it != propositions.end()) {
                    prop = it->second;
                } else {
                    // Create a new proposition with the identifier
                    prop.setPrefix(token.value);
                    prop.setTruthValue(Tripartite::UNKNOWN);
                }
                expr.addToken(prop);
                break;
            }
            case TokenType::AND:
            case TokenType::OR:
            case TokenType::NOT:
            case TokenType::IMPLIES:
            case TokenType::EQUIVALENT:
            case TokenType::LPAREN:
            case TokenType::RPAREN:
                expr.addToken(tokenTypeToLogicalOperator(token.type));
                break;
            default:
                // Skip other tokens (COMMA, ASSIGN, etc.)
                break;
        }
    }
    
    return expr;
}

void Parser::parseFactsLine(const std::string& line,
                            std::unordered_map<std::string, Proposition>& propositions,
                            std::vector<Expression>& expressions) {
    // Skip empty lines
    if (line.empty() || line.find_first_not_of(" \t\n\r") == std::string::npos) {
        return;
    }
    
    try {
        auto tokens = lexer_.tokenizeContent(line);
        if (tokens.empty()) return;
        
        // Check if this is an assignment: identifier = expression
        size_t assignIndex = SIZE_MAX;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (tokens[i].type == TokenType::ASSIGN) {
                assignIndex = i;
                break;
            }
        }
        
        if (assignIndex != SIZE_MAX && assignIndex > 0) {
            // Assignment: target = expression
            std::string targetName = tokens[0].value;
            
            // Extract RHS tokens (after the =)
            std::vector<LexerToken> rhsTokens(tokens.begin() + assignIndex + 1, tokens.end());
            
            if (!rhsTokens.empty()) {
                // Build and evaluate the expression
                Expression expr = buildExpression(rhsTokens, propositions, targetName);
                Tripartite result = expr.evaluate();
                
                // Set the target's truth value
                propositions[targetName].setTruthValue(result);
                
                // Store the expression for potential re-evaluation
                expressions.push_back(expr);
            }
        } else {
            // No assignment - process as assertion(s)
            // Handle patterns like: !q, p && n, (a || b)
            
            // Check if it's a simple negation: !identifier
            if (tokens.size() == 2 && tokens[0].type == TokenType::NOT && 
                tokens[1].type == TokenType::IDENTIFIER) {
                propositions[tokens[1].value].setTruthValue(Tripartite::FALSE);
                return;
            }
            
            // Check if it's a simple assertion: identifier
            if (tokens.size() == 1 && tokens[0].type == TokenType::IDENTIFIER) {
                propositions[tokens[0].value].setTruthValue(Tripartite::TRUE);
                return;
            }
            
            // For compound expressions without assignment, set identifiers based on whether
            // they are negated, and build an expression for potential evaluation
            bool hasOperators = false;
            for (size_t i = 0; i < tokens.size(); ++i) {
                const auto& token = tokens[i];
                
                if (token.type == TokenType::AND || token.type == TokenType::OR ||
                    token.type == TokenType::IMPLIES) {
                    hasOperators = true;
                }
                
                if (token.type == TokenType::IDENTIFIER) {
                    // Check for preceding NOT operator
                    bool isNegated = (i > 0 && tokens[i - 1].type == TokenType::NOT);
                    
                    if (isNegated) {
                        propositions[token.value].setTruthValue(Tripartite::FALSE);
                    } else {
                        propositions[token.value].setTruthValue(Tripartite::TRUE);
                    }
                }
            }
            
            // If there are operators, also create an expression
            if (hasOperators) {
                Expression expr = buildExpression(tokens, propositions, "");
                expressions.push_back(expr);
            }
        }
    } catch (const LexerError& e) {
        std::cerr << "Error parsing facts line: " << e.what() << std::endl;
    }
}

void Parser::parseFactsFile(const std::string& filename, 
                            std::unordered_map<std::string, Proposition>& propositions) {
    std::vector<Expression> expressions;  // Discarded in this overload
    parseFactsFile(filename, propositions, expressions);
}

void Parser::parseFactsFile(const std::string& filename,
                            std::unordered_map<std::string, Proposition>& propositions,
                            std::vector<Expression>& expressions) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        parseFactsLine(line, propositions, expressions);
    }
    file.close();
}

Expression Parser::parseExpressionString(const std::string& exprString,
                                         const std::unordered_map<std::string, Proposition>& propositions,
                                         const std::string& prefix) {
    try {
        auto tokens = lexer_.tokenizeContent(exprString);
        return buildExpression(tokens, propositions, prefix);
    } catch (const LexerError& e) {
        std::cerr << "Error parsing expression: " << e.what() << std::endl;
        return Expression();  // Return empty expression on error
    }
}