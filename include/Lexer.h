#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <stdexcept>
#include <optional>

/**
 * TokenType enumerates all token types the lexer can produce.
 */
enum class TokenType {
    // Identifiers and literals
    IDENTIFIER,     ///< Variable names, proposition names (e.g., "big-bang", "p", "implies")
    
    // Logical operators
    AND,            ///< && or "and"
    OR,             ///< || or "or"
    NOT,            ///< ! or ~ or "not"
    IMPLIES,        ///< -> or "implies"
    EQUIVALENT,     ///< <-> or "iff"
    
    // Punctuation
    LPAREN,         ///< (
    RPAREN,         ///< )
    COMMA,          ///< ,
    ASSIGN,         ///< =
    
    // Special
    NEWLINE,        ///< End of line (useful for line-based parsing)
    END_OF_INPUT,   ///< End of input stream
    
    // Error
    UNKNOWN         ///< Unrecognized token
};

/**
 * Convert TokenType to string for debugging and error messages.
 */
std::string tokenTypeToString(TokenType type);

/**
 * SourceLocation tracks position in source text for error reporting.
 */
struct SourceLocation {
    size_t line;        ///< 1-based line number
    size_t column;      ///< 1-based column number
    size_t offset;      ///< 0-based character offset from start
    
    SourceLocation() : line(1), column(1), offset(0) {}
    SourceLocation(size_t l, size_t c, size_t o) : line(l), column(c), offset(o) {}
    
    std::string toString() const;
};

/**
 * LexerToken represents a single lexical token with its type, value, and location.
 * Named LexerToken to avoid conflict with Expression::Token.
 */
struct LexerToken {
    TokenType type;           ///< The type of token
    std::string value;        ///< The original text of the token
    SourceLocation location;  ///< Where the token starts in the source
    
    LexerToken() : type(TokenType::UNKNOWN), value(""), location() {}
    LexerToken(TokenType t, const std::string& v, const SourceLocation& loc)
        : type(t), value(v), location(loc) {}
    
    /// Check if this is an operator token
    bool isOperator() const;
    
    /// Check if this is a punctuation token
    bool isPunctuation() const;
    
    /// Human-readable description for error messages
    std::string toString() const;
};

/**
 * LexerError provides detailed error information with source location.
 */
class LexerError : public std::runtime_error {
public:
    SourceLocation location;
    std::string context;  ///< Surrounding text for context
    
    LexerError(const std::string& message, const SourceLocation& loc, 
               const std::string& ctx = "")
        : std::runtime_error(formatMessage(message, loc, ctx)),
          location(loc), context(ctx) {}
    
private:
    static std::string formatMessage(const std::string& msg, 
                                     const SourceLocation& loc,
                                     const std::string& ctx);
};

/**
 * Lexer tokenizes input strings into a stream of tokens.
 * 
 * Features:
 * - Line/column tracking for precise error messages
 * - Support for identifiers with hyphens and underscores
 * - Multi-character operators (&&, ||, ->, <->)
 * - Optional newline tokens for line-based parsing
 * 
 * Usage:
 *   Lexer lexer;
 *   auto tokens = lexer.tokenize("p, implies(A, B)");
 *   for (const auto& token : tokens) {
 *       std::cout << token.toString() << std::endl;
 *   }
 */
class Lexer {
public:
    /**
     * Configuration options for the lexer.
     */
    struct Options {
        bool emitNewlines = false;    ///< Include NEWLINE tokens in output
        bool allowHyphensInIds = true; ///< Allow hyphens in identifiers (e.g., "big-bang")
        bool treatKeywordsAsOps = true; ///< Treat "and", "or", "not" as operators
        
        Options() = default;
    };

private:
    std::string input_;
    size_t pos_;
    size_t line_;
    size_t column_;
    Options options_;
    
    /// Current character or '\0' if at end
    char current() const;
    
    /// Peek ahead n characters (0 = current)
    char peek(size_t n = 0) const;
    
    /// Advance position by one character
    void advance();
    
    /// Skip whitespace (except newlines if emitNewlines is true)
    void skipWhitespace();
    
    /// Skip to end of line (for comments)
    void skipLineComment();
    
    /// Check if at end of input
    bool isAtEnd() const;
    
    /// Get current source location
    SourceLocation currentLocation() const;
    
    /// Get context string around current position for error messages
    std::string getContext() const;
    
    /// Scan a single token
    LexerToken scanToken();
    
    /// Scan an identifier (alphanumeric, hyphens, underscores)
    LexerToken scanIdentifier();
    
    /// Check if character can start an identifier
    static bool isIdentifierStart(char c);
    
    /// Check if character can continue an identifier
    bool isIdentifierContinue(char c) const;

public:
    Lexer();
    explicit Lexer(const Options& opts);
    ~Lexer() = default;
    
    /**
     * Tokenize an input string.
     * 
     * @param input The source text to tokenize
     * @return Vector of tokens (always ends with END_OF_INPUT)
     * @throws LexerError on invalid input
     */
    std::vector<LexerToken> tokenize(const std::string& input);
    
    /**
     * Tokenize and return tokens without END_OF_INPUT marker.
     * Useful for simpler iteration.
     */
    std::vector<LexerToken> tokenizeContent(const std::string& input);
    
    /**
     * Set lexer options.
     */
    void setOptions(const Options& opts);
    
    /**
     * Get current lexer options.
     */
    const Options& getOptions() const;
    
    /**
     * Reset lexer state (called automatically by tokenize).
     */
    void reset();
};

#endif // LEXER_H

