#ifndef PARSER_H
#define PARSER_H

#include "Proposition.h"
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

/**
 * RelationHandler is a function type that processes a parsed relation.
 * 
 * @param prefix The proposition prefix (identifier)
 * @param args The parsed arguments from the relation
 * @param propositions The map to store the resulting proposition(s)
 * @return true if the relation was handled successfully, false otherwise
 * 
 * Example: For "n, implies(big-bang, occurred, microwave-radiation, present)"
 *   - prefix = "n"
 *   - args = {"big-bang", "occurred", "microwave-radiation", "present"}
 */
using RelationHandler = std::function<bool(
    const std::string& prefix,
    const std::vector<std::string>& args,
    std::unordered_map<std::string, Proposition>& propositions
)>;

/**
 * Parser class responsible for parsing input files and producing propositions.
 * 
 * Supports extensible relation types through a registry pattern.
 * Built-in relations: implies, some, not, discovered
 * 
 * Usage:
 *   Parser parser;
 *   // Register a custom relation handler
 *   parser.registerRelation("custom", [](const std::string& prefix, 
 *                                        const std::vector<std::string>& args,
 *                                        std::unordered_map<std::string, Proposition>& props) {
 *       // Handle the custom relation
 *       return true;
 *   });
 *   auto propositions = parser.parseAssumptionsFile("file.txt");
 */
class Parser {
private:
    std::unordered_map<std::string, RelationHandler> relationHandlers_;
    
    // Initialize built-in relation handlers
    void registerBuiltinHandlers();
    
    // Built-in handler implementations
    static bool handleImplies(const std::string& prefix,
                              const std::vector<std::string>& args,
                              std::unordered_map<std::string, Proposition>& propositions);
    
    static bool handleSome(const std::string& prefix,
                           const std::vector<std::string>& args,
                           std::unordered_map<std::string, Proposition>& propositions);
    
    static bool handleNot(const std::string& prefix,
                          const std::vector<std::string>& args,
                          std::unordered_map<std::string, Proposition>& propositions);
    
    static bool handleDiscovered(const std::string& prefix,
                                 const std::vector<std::string>& args,
                                 std::unordered_map<std::string, Proposition>& propositions);

public:
    Parser();
    ~Parser() = default;

    /**
     * Register a custom relation handler.
     * 
     * @param relationName The name of the relation (e.g., "implies", "custom")
     * @param handler The function to handle this relation type
     */
    void registerRelation(const std::string& relationName, RelationHandler handler);
    
    /**
     * Unregister a relation handler.
     * 
     * @param relationName The name of the relation to remove
     * @return true if the relation was found and removed
     */
    bool unregisterRelation(const std::string& relationName);
    
    /**
     * Check if a relation handler is registered.
     * 
     * @param relationName The name of the relation to check
     * @return true if a handler is registered for this relation
     */
    bool hasRelation(const std::string& relationName) const;
    
    /**
     * Get the list of registered relation names.
     * 
     * @return Vector of registered relation names
     */
    std::vector<std::string> getRegisteredRelations() const;

    /**
     * Parse an assumptions file containing proposition definitions.
     * Uses registered relation handlers to process each line.
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

