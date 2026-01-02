#include "Lexer.h"

#include <sstream>
#include <algorithm>
#include <cctype>

// ========== TokenType Utilities ==========

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::IDENTIFIER:    return "IDENTIFIER";
        case TokenType::AND:           return "AND";
        case TokenType::OR:            return "OR";
        case TokenType::NOT:           return "NOT";
        case TokenType::IMPLIES:       return "IMPLIES";
        case TokenType::EQUIVALENT:    return "EQUIVALENT";
        case TokenType::LPAREN:        return "LPAREN";
        case TokenType::RPAREN:        return "RPAREN";
        case TokenType::COMMA:         return "COMMA";
        case TokenType::ASSIGN:        return "ASSIGN";
        case TokenType::NEWLINE:       return "NEWLINE";
        case TokenType::END_OF_INPUT:  return "END_OF_INPUT";
        case TokenType::UNKNOWN:       return "UNKNOWN";
        default:                       return "UNKNOWN";
    }
}

// ========== SourceLocation ==========

std::string SourceLocation::toString() const {
    std::ostringstream oss;
    oss << "line " << line << ", column " << column;
    return oss.str();
}

// ========== LexerToken ==========

bool LexerToken::isOperator() const {
    switch (type) {
        case TokenType::AND:
        case TokenType::OR:
        case TokenType::NOT:
        case TokenType::IMPLIES:
        case TokenType::EQUIVALENT:
            return true;
        default:
            return false;
    }
}

bool LexerToken::isPunctuation() const {
    switch (type) {
        case TokenType::LPAREN:
        case TokenType::RPAREN:
        case TokenType::COMMA:
        case TokenType::ASSIGN:
            return true;
        default:
            return false;
    }
}

std::string LexerToken::toString() const {
    std::ostringstream oss;
    oss << tokenTypeToString(type) << "(\"" << value << "\") at " << location.toString();
    return oss.str();
}

// ========== LexerError ==========

std::string LexerError::formatMessage(const std::string& msg,
                                       const SourceLocation& loc,
                                       const std::string& ctx) {
    std::ostringstream oss;
    oss << "Lexer error at " << loc.toString() << ": " << msg;
    if (!ctx.empty()) {
        oss << "\n  Context: \"" << ctx << "\"";
        // Add caret pointing to the error column
        if (loc.column > 0 && loc.column <= ctx.length() + 12) {
            oss << "\n            ";
            for (size_t i = 1; i < loc.column && i < 50; ++i) {
                oss << ' ';
            }
            oss << '^';
        }
    }
    return oss.str();
}

// ========== Lexer Implementation ==========

Lexer::Lexer() : pos_(0), line_(1), column_(1), options_() {}

Lexer::Lexer(const Options& opts) : pos_(0), line_(1), column_(1), options_(opts) {}

void Lexer::setOptions(const Options& opts) {
    options_ = opts;
}

const Lexer::Options& Lexer::getOptions() const {
    return options_;
}

void Lexer::reset() {
    pos_ = 0;
    line_ = 1;
    column_ = 1;
    input_.clear();
}

char Lexer::current() const {
    if (pos_ >= input_.length()) return '\0';
    return input_[pos_];
}

char Lexer::peek(size_t n) const {
    size_t idx = pos_ + n;
    if (idx >= input_.length()) return '\0';
    return input_[idx];
}

void Lexer::advance() {
    if (pos_ < input_.length()) {
        if (input_[pos_] == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        pos_++;
    }
}

bool Lexer::isAtEnd() const {
    return pos_ >= input_.length();
}

SourceLocation Lexer::currentLocation() const {
    return SourceLocation(line_, column_, pos_);
}

std::string Lexer::getContext() const {
    // Find start of current line
    size_t lineStart = pos_;
    while (lineStart > 0 && input_[lineStart - 1] != '\n') {
        lineStart--;
    }
    
    // Find end of current line
    size_t lineEnd = pos_;
    while (lineEnd < input_.length() && input_[lineEnd] != '\n') {
        lineEnd++;
    }
    
    // Extract line, limiting length
    std::string context = input_.substr(lineStart, std::min(lineEnd - lineStart, size_t(60)));
    return context;
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = current();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '\n' && !options_.emitNewlines) {
            advance();
        } else if (c == '#') {
            // Skip line comments starting with #
            skipLineComment();
        } else {
            break;
        }
    }
}

void Lexer::skipLineComment() {
    // Skip until end of line
    while (!isAtEnd() && current() != '\n') {
        advance();
    }
}

bool Lexer::isIdentifierStart(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_' || c == '~';
}

bool Lexer::isIdentifierContinue(char c) const {
    if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
        return true;
    }
    // Allow hyphens in identifiers if enabled (e.g., "big-bang")
    if (options_.allowHyphensInIds && c == '-') {
        return true;
    }
    return false;
}

LexerToken Lexer::scanIdentifier() {
    SourceLocation startLoc = currentLocation();
    std::string value;
    
    // First character
    value += current();
    advance();
    
    // Subsequent characters
    while (!isAtEnd() && isIdentifierContinue(current())) {
        value += current();
        advance();
    }
    
    // Check for keyword operators
    if (options_.treatKeywordsAsOps) {
        std::string lower = value;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        
        if (lower == "and") {
            return LexerToken(TokenType::AND, value, startLoc);
        } else if (lower == "or") {
            return LexerToken(TokenType::OR, value, startLoc);
        } else if (lower == "not") {
            return LexerToken(TokenType::NOT, value, startLoc);
        } else if (lower == "implies") {
            // Note: "implies" as a keyword is only treated as operator in expression context
            // In assumption files, it's a function name (identifier)
            // We return IDENTIFIER here; the parser decides based on context
        } else if (lower == "iff") {
            return LexerToken(TokenType::EQUIVALENT, value, startLoc);
        }
    }
    
    return LexerToken(TokenType::IDENTIFIER, value, startLoc);
}

LexerToken Lexer::scanToken() {
    skipWhitespace();
    
    if (isAtEnd()) {
        return LexerToken(TokenType::END_OF_INPUT, "", currentLocation());
    }
    
    SourceLocation startLoc = currentLocation();
    char c = current();
    
    // Newline (only if emitNewlines is true)
    if (c == '\n' && options_.emitNewlines) {
        advance();
        return LexerToken(TokenType::NEWLINE, "\\n", startLoc);
    }
    
    // Single-character tokens
    switch (c) {
        case '(':
            advance();
            return LexerToken(TokenType::LPAREN, "(", startLoc);
        case ')':
            advance();
            return LexerToken(TokenType::RPAREN, ")", startLoc);
        case ',':
            advance();
            return LexerToken(TokenType::COMMA, ",", startLoc);
        case '!':
            advance();
            return LexerToken(TokenType::NOT, "!", startLoc);
        case '~':
            // Could be negation operator or start of identifier like ~P
            if (peek(1) != '\0' && isIdentifierStart(peek(1))) {
                // It's part of an identifier like ~P
                return scanIdentifier();
            }
            advance();
            return LexerToken(TokenType::NOT, "~", startLoc);
        default:
            break;
    }
    
    // Multi-character operators
    if (c == '&' && peek(1) == '&') {
        advance(); advance();
        return LexerToken(TokenType::AND, "&&", startLoc);
    }
    
    if (c == '|' && peek(1) == '|') {
        advance(); advance();
        return LexerToken(TokenType::OR, "||", startLoc);
    }
    
    if (c == '-' && peek(1) == '>') {
        advance(); advance();
        return LexerToken(TokenType::IMPLIES, "->", startLoc);
    }
    
    if (c == '<' && peek(1) == '-' && peek(2) == '>') {
        advance(); advance(); advance();
        return LexerToken(TokenType::EQUIVALENT, "<->", startLoc);
    }
    
    if (c == '=') {
        // Check for == (equality) vs = (assignment)
        if (peek(1) == '=') {
            advance(); advance();
            return LexerToken(TokenType::EQUIVALENT, "==", startLoc);
        }
        advance();
        return LexerToken(TokenType::ASSIGN, "=", startLoc);
    }
    
    // Identifiers
    if (isIdentifierStart(c)) {
        return scanIdentifier();
    }
    
    // Digits can start identifiers in this language (e.g., "4-fundamental-forces")
    if (std::isdigit(static_cast<unsigned char>(c))) {
        return scanIdentifier();
    }
    
    // Unknown character
    std::string unknownChar(1, c);
    advance();
    throw LexerError("Unexpected character '" + unknownChar + "'", startLoc, getContext());
}

std::vector<LexerToken> Lexer::tokenize(const std::string& input) {
    reset();
    input_ = input;
    
    std::vector<LexerToken> tokens;
    
    while (true) {
        LexerToken token = scanToken();
        tokens.push_back(token);
        
        if (token.type == TokenType::END_OF_INPUT) {
            break;
        }
    }
    
    return tokens;
}

std::vector<LexerToken> Lexer::tokenizeContent(const std::string& input) {
    auto tokens = tokenize(input);
    
    // Remove the END_OF_INPUT token
    if (!tokens.empty() && tokens.back().type == TokenType::END_OF_INPUT) {
        tokens.pop_back();
    }
    
    return tokens;
}

