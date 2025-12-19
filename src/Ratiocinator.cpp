#include "Ratiocinator.h"
#include "Expression.h"
#include "Proposition.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>

// Constructor
Ratiocinator::Ratiocinator() {}

// Destructor
Ratiocinator::~Ratiocinator() = default;

// Parse the facts file and initialize propositions or expressions
void Ratiocinator::parseAssumptionsFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    std::string line;
    // Updated regex pattern to match lines like "prefix, relation(arg1, arg2, arg3, arg4)"
    std::regex linePattern(R"(^\s*(\w+)\s*,\s*(\w+)\s*\(\s*([-\w\s\d,]+?)\s*\)\s*$)");
    /* Explanation of the regex pattern:
    R"( ... )" is a raw string literal that allows us to write the regex pattern without escaping backslashes.
    ^            : Assert position at the start of the line.
    \s*          : Match any leading whitespace (optional).
    (\w+)        : Match and capture the "prefix" as one or more word characters (letters, digits, or underscore).
    \s*          : Match optional whitespace after the prefix.
    ,            : Match the literal comma that separates the prefix from the relation type.
    \s*          : Match optional whitespace after the comma.
    (\w+)        : Match and capture the "relation" as one or more word characters (e.g., "implies", "some", "not").
    \s*          : Match optional whitespace after the relation.
    \(           : Match the opening parenthesis that starts the argument list.
    \s*          : Match any whitespace at the start of the argument list (optional).
    ([^)]+?)     : Match and capture everything inside the parentheses, stopping before the closing parenthesis.
                - `[^)]` matches any character except the closing parenthesis.
                - `+?` is a "lazy" quantifier that captures one or more characters, stopping as soon as it finds `)` after the arguments.
    \s*          : Match any whitespace after the argument list (optional).
    \)           : Match the closing parenthesis of the argument list.
    \s*          : Match any trailing whitespace after the closing parenthesis (optional).
    $            : Assert position at the end of the line.
    */

    // Example: For a line like "n, implies(big-bang, occurred, microwave-radiation, present)", this pattern captures:
    //   match[1] = "n"         (prefix)
    //   match[2] = "implies"   (relation)
    //   match[3] = "big-bang, occurred, microwave-radiation, present" (arguments as a single string)
    std::smatch match;

     while (std::getline(file, line)) {
        if (std::regex_match(line, match, linePattern)) {
            std::string prefix = match[1];
            std::string relation = match[2];
            std::string arguments = match[3];

            // Split arguments
            std::istringstream argsStream(arguments);
            std::vector<std::string> parts;
            std::string part;
            while (std::getline(argsStream, part, ',')) {
                part.erase(0, part.find_first_not_of(" \t"));
                part.erase(part.find_last_not_of(" \t") + 1);
                parts.push_back(part);
            }

            // Process each relation type
            Proposition proposition;
            proposition.setPrefix(prefix);

            if (relation == "implies" && parts.size() == 4) {
                proposition.setRelation(LogicalOperator::IMPLIES);
                proposition.setAntecedent(parts[0]);
                proposition.setSubject(parts[1]);
                proposition.setConsequent(parts[2]);
                proposition.setPredicate(parts[3]);
                proposition.setPropositionScope(Quantifier::UNIVERSAL_AFFIRMATIVE);
                propositions[parts[2]] = proposition;

            } else if (relation == "some" && parts.size() == 2) {
                proposition.setRelation(LogicalOperator::NONE);
                proposition.setSubject(parts[0]);
                proposition.setPredicate(parts[1]);
                proposition.setTruthValue(Tripartite::TRUE);
                proposition.setPropositionScope(Quantifier::PARTICULAR_AFFIRMATIVE);
                propositions[parts[0]] = proposition;

            } else if (relation == "not" && parts.size() == 1) {
                proposition.setRelation(LogicalOperator::NOT);
                proposition.setSubject(parts[0]);
                proposition.setTruthValue(Tripartite::FALSE);
                proposition.setPropositionScope(Quantifier::UNIVERSAL_NEGATIVE);
                propositions[parts[0]] = proposition;

            } else if (relation == "discovered" && parts.size() == 2) {
                proposition.setRelation(LogicalOperator::NONE);
                proposition.setSubject(parts[0]);
                proposition.setPredicate(parts[1]);
                propositions[parts[0]] = proposition;
            }
        } else {
            std::cerr << "Error parsing line: " << line 
                      << " (check for unbalanced parentheses or incorrect format)" << std::endl;
        }
    }
    file.close();
}

// Parse the arguments file and store expressions
void Ratiocinator::parseFactsFile(const std::string& filename) {
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


void Ratiocinator::deduceAll() {
    bool changesMade;
    do {
        changesMade = false;
        
        // ============================================================
        // PHASE 1: Apply inference rules to IMPLIES propositions
        // ============================================================
        for (auto& entry : propositions) {
            Proposition& prop = entry.second;
            if (prop.getRelation() == LogicalOperator::IMPLIES) {
                // Get the names of antecedent and consequent
                const std::string& antecedentName = prop.getAntecedent();
                const std::string& consequentName = prop.getConsequent();
                
                // Look up their actual truth values from the propositions map
                Tripartite antecedentTruth = propositions[antecedentName].getTruthValue();
                Tripartite consequentTruth = propositions[consequentName].getTruthValue();
                
                // Modus Ponens: P → Q, P is TRUE ⊢ Q is TRUE
                if (antecedentTruth == Tripartite::TRUE && consequentTruth != Tripartite::TRUE) {
                    propositions[consequentName].setTruthValue(Tripartite::TRUE);
                    changesMade = true;
                }
                
                // Modus Tollens: P → Q, Q is FALSE ⊢ P is FALSE
                if (consequentTruth == Tripartite::FALSE && antecedentTruth != Tripartite::FALSE) {
                    propositions[antecedentName].setTruthValue(Tripartite::FALSE);
                    changesMade = true;
                }
            }
        }
        
        // ============================================================
        // PHASE 2: Evaluate explicit Expression objects (if any)
        // ============================================================
        for (auto& expr : expressions) {
            Tripartite resultValue = expr.evaluate();
            const std::string& subject = expr.getPrefix();

            Tripartite currentValue = propositions[subject].getTruthValue();
            Quantifier scope = propositions[subject].getPropositionScope();

            switch (scope) {
                case Quantifier::UNIVERSAL_AFFIRMATIVE:
                    if (currentValue != resultValue && resultValue == Tripartite::TRUE) {
                        propositions[subject].setTruthValue(Tripartite::TRUE);
                        changesMade = true;
                    }
                    break;

                case Quantifier::UNIVERSAL_NEGATIVE:
                    if (currentValue != resultValue && resultValue == Tripartite::FALSE) {
                        propositions[subject].setTruthValue(Tripartite::FALSE);
                        changesMade = true;
                    }
                    break;

                case Quantifier::PARTICULAR_AFFIRMATIVE:
                    if (resultValue == Tripartite::TRUE) {
                        propositions[subject].setTruthValue(Tripartite::TRUE);
                        changesMade = true;
                    }
                    break;

                case Quantifier::PARTICULAR_NEGATIVE:
                    if (resultValue == Tripartite::FALSE && currentValue != Tripartite::TRUE) {
                        propositions[subject].setTruthValue(Tripartite::FALSE);
                        changesMade = true;
                    }
                    break;

                default:
                    break;
            }
        }
    } while (changesMade);
}

// Output the truth values of all propositions
std::string Ratiocinator::outputTruthValues() const {
    std::string output;
    std::string truthValueStr;

    for (const auto& entry : propositions) {
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

// Apply Modus Ponens: P → Q, P is TRUE ⊢ Q is TRUE
// Looks up the antecedent's truth value from propositions map
bool Ratiocinator::applyModusPonens(Proposition& implication) {
    const std::string& antecedentName = implication.getAntecedent();
    const std::string& consequentName = implication.getConsequent();
    
    Tripartite antecedentTruth = propositions[antecedentName].getTruthValue();
    
    if (antecedentTruth == Tripartite::TRUE) {
        propositions[consequentName].setTruthValue(Tripartite::TRUE);
        return true;
    }
    return false;
}

// Apply Modus Tollens: P → Q, Q is FALSE ⊢ P is FALSE
// Looks up the consequent's truth value from propositions map
bool Ratiocinator::applyModusTollens(Proposition& implication) {
    const std::string& antecedentName = implication.getAntecedent();
    const std::string& consequentName = implication.getConsequent();
    
    Tripartite consequentTruth = propositions[consequentName].getTruthValue();
    
    if (consequentTruth == Tripartite::FALSE) {
        propositions[antecedentName].setTruthValue(Tripartite::FALSE);
        return true;
    }
    return false;
}
