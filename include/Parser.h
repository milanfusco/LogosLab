#ifndef PARSER_H
#define PARSER_H

#include "Proposition.h"
#include <string>
#include <unordered_map>

/**
 * Parser class responsible for parsing input files and producing propositions.
 * Separates file I/O and parsing logic from the reasoning engine.
 */
class Parser {
public:
    Parser() = default;
    ~Parser() = default;

    /**
     * Parse an assumptions file containing proposition definitions.
     * Supports relation types: implies, some, not, discovered.
     * 
     * @param filename Path to the assumptions file
     * @return Map of proposition names to Proposition objects
     */
    std::unordered_map<std::string, Proposition> parseAssumptionsFile(const std::string& filename);

    /**
     * Parse a facts file and update truth values of existing propositions.
     * 
     * @param filename Path to the facts file
     * @param propositions Existing propositions map to update
     */
    void parseFactsFile(const std::string& filename, 
                        std::unordered_map<std::string, Proposition>& propositions);
};

#endif // PARSER_H

