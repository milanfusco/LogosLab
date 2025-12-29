#include "Lexer.h"
#include <iostream>
#include <cassert>
#include <string>

// Test: Basic identifier tokenization
void testIdentifiers() {
    std::cout << "Running testIdentifiers..." << std::endl;
    
    Lexer lexer;
    auto tokens = lexer.tokenizeContent("foo bar baz");
    
    assert(tokens.size() == 3);
    assert(tokens[0].type == TokenType::IDENTIFIER);
    assert(tokens[0].value == "foo");
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].value == "bar");
    assert(tokens[2].type == TokenType::IDENTIFIER);
    assert(tokens[2].value == "baz");
    
    std::cout << "Test passed: Basic identifiers tokenized correctly." << std::endl;
}

// Test: Identifiers with hyphens (like "big-bang")
void testHyphenatedIdentifiers() {
    std::cout << "Running testHyphenatedIdentifiers..." << std::endl;
    
    Lexer lexer;
    auto tokens = lexer.tokenizeContent("big-bang light-from-galaxies 4-fundamental-forces");
    
    assert(tokens.size() == 3);
    assert(tokens[0].type == TokenType::IDENTIFIER);
    assert(tokens[0].value == "big-bang");
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].value == "light-from-galaxies");
    assert(tokens[2].type == TokenType::IDENTIFIER);
    assert(tokens[2].value == "4-fundamental-forces");
    
    std::cout << "Test passed: Hyphenated identifiers tokenized correctly." << std::endl;
}

// Test: Logical operators
void testOperators() {
    std::cout << "Running testOperators..." << std::endl;
    
    Lexer lexer;
    auto tokens = lexer.tokenizeContent("&& || ! -> <->");
    
    assert(tokens.size() == 5);
    assert(tokens[0].type == TokenType::AND);
    assert(tokens[0].value == "&&");
    assert(tokens[1].type == TokenType::OR);
    assert(tokens[1].value == "||");
    assert(tokens[2].type == TokenType::NOT);
    assert(tokens[2].value == "!");
    assert(tokens[3].type == TokenType::IMPLIES);
    assert(tokens[3].value == "->");
    assert(tokens[4].type == TokenType::EQUIVALENT);
    assert(tokens[4].value == "<->");
    
    std::cout << "Test passed: Operators tokenized correctly." << std::endl;
}

// Test: Punctuation
void testPunctuation() {
    std::cout << "Running testPunctuation..." << std::endl;
    
    Lexer lexer;
    auto tokens = lexer.tokenizeContent("( ) , =");
    
    assert(tokens.size() == 4);
    assert(tokens[0].type == TokenType::LPAREN);
    assert(tokens[1].type == TokenType::RPAREN);
    assert(tokens[2].type == TokenType::COMMA);
    assert(tokens[3].type == TokenType::ASSIGN);
    
    std::cout << "Test passed: Punctuation tokenized correctly." << std::endl;
}

// Test: Keyword operators (and, or, not)
void testKeywordOperators() {
    std::cout << "Running testKeywordOperators..." << std::endl;
    
    Lexer lexer;
    auto tokens = lexer.tokenizeContent("and or not AND OR NOT");
    
    assert(tokens.size() == 6);
    assert(tokens[0].type == TokenType::AND);
    assert(tokens[1].type == TokenType::OR);
    assert(tokens[2].type == TokenType::NOT);
    assert(tokens[3].type == TokenType::AND);
    assert(tokens[4].type == TokenType::OR);
    assert(tokens[5].type == TokenType::NOT);
    
    std::cout << "Test passed: Keyword operators tokenized correctly." << std::endl;
}

// Test: Assumptions file format
void testAssumptionsFormat() {
    std::cout << "Running testAssumptionsFormat..." << std::endl;
    
    Lexer lexer;
    auto tokens = lexer.tokenizeContent("n, implies(big-bang, occurred, microwave-radiation, present)");
    
    // Expected: n , implies ( big-bang , occurred , microwave-radiation , present )
    assert(tokens.size() == 12);
    assert(tokens[0].type == TokenType::IDENTIFIER);
    assert(tokens[0].value == "n");
    assert(tokens[1].type == TokenType::COMMA);
    assert(tokens[2].type == TokenType::IDENTIFIER);
    assert(tokens[2].value == "implies");
    assert(tokens[3].type == TokenType::LPAREN);
    assert(tokens[4].type == TokenType::IDENTIFIER);
    assert(tokens[4].value == "big-bang");
    assert(tokens[5].type == TokenType::COMMA);
    assert(tokens[6].type == TokenType::IDENTIFIER);
    assert(tokens[6].value == "occurred");
    assert(tokens[11].type == TokenType::RPAREN);
    
    std::cout << "Test passed: Assumptions format tokenized correctly." << std::endl;
}

// Test: Facts file format
void testFactsFormat() {
    std::cout << "Running testFactsFormat..." << std::endl;
    
    Lexer lexer;
    auto tokens = lexer.tokenizeContent("!q");
    
    assert(tokens.size() == 2);
    assert(tokens[0].type == TokenType::NOT);
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].value == "q");
    
    tokens = lexer.tokenizeContent("p && n");
    assert(tokens.size() == 3);
    assert(tokens[0].type == TokenType::IDENTIFIER);
    assert(tokens[1].type == TokenType::AND);
    assert(tokens[2].type == TokenType::IDENTIFIER);
    
    tokens = lexer.tokenizeContent("t = p && n");
    assert(tokens.size() == 5);
    assert(tokens[0].type == TokenType::IDENTIFIER);
    assert(tokens[0].value == "t");
    assert(tokens[1].type == TokenType::ASSIGN);
    assert(tokens[2].type == TokenType::IDENTIFIER);
    assert(tokens[3].type == TokenType::AND);
    assert(tokens[4].type == TokenType::IDENTIFIER);
    
    std::cout << "Test passed: Facts format tokenized correctly." << std::endl;
}

// Test: Negated identifiers (like ~P)
void testNegatedIdentifiers() {
    std::cout << "Running testNegatedIdentifiers..." << std::endl;
    
    Lexer lexer;
    auto tokens = lexer.tokenizeContent("~P ~Q");
    
    assert(tokens.size() == 2);
    assert(tokens[0].type == TokenType::IDENTIFIER);
    assert(tokens[0].value == "~P");
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].value == "~Q");
    
    std::cout << "Test passed: Negated identifiers tokenized correctly." << std::endl;
}

// Test: Source location tracking
void testSourceLocations() {
    std::cout << "Running testSourceLocations..." << std::endl;
    
    Lexer lexer;
    auto tokens = lexer.tokenizeContent("foo bar");
    
    assert(tokens.size() == 2);
    assert(tokens[0].location.line == 1);
    assert(tokens[0].location.column == 1);
    assert(tokens[1].location.line == 1);
    assert(tokens[1].location.column == 5);
    
    std::cout << "Test passed: Source locations tracked correctly." << std::endl;
}

// Test: Newline handling with emitNewlines option
void testNewlineHandling() {
    std::cout << "Running testNewlineHandling..." << std::endl;
    
    // Without emitNewlines (default)
    Lexer lexer;
    auto tokens = lexer.tokenizeContent("foo\nbar");
    assert(tokens.size() == 2);
    assert(tokens[0].value == "foo");
    assert(tokens[1].value == "bar");
    
    // With emitNewlines
    Lexer::Options opts;
    opts.emitNewlines = true;
    Lexer lexerWithNewlines(opts);
    tokens = lexerWithNewlines.tokenizeContent("foo\nbar");
    assert(tokens.size() == 3);
    assert(tokens[0].value == "foo");
    assert(tokens[1].type == TokenType::NEWLINE);
    assert(tokens[2].value == "bar");
    
    std::cout << "Test passed: Newline handling works correctly." << std::endl;
}

// Test: Comments are skipped
void testComments() {
    std::cout << "Running testComments..." << std::endl;
    
    Lexer lexer;
    auto tokens = lexer.tokenizeContent("foo # this is a comment\nbar");
    
    assert(tokens.size() == 2);
    assert(tokens[0].value == "foo");
    assert(tokens[1].value == "bar");
    
    std::cout << "Test passed: Comments skipped correctly." << std::endl;
}

// Test: Empty input
void testEmptyInput() {
    std::cout << "Running testEmptyInput..." << std::endl;
    
    Lexer lexer;
    auto tokens = lexer.tokenize("");
    
    assert(tokens.size() == 1);
    assert(tokens[0].type == TokenType::END_OF_INPUT);
    
    tokens = lexer.tokenizeContent("");
    assert(tokens.empty());
    
    std::cout << "Test passed: Empty input handled correctly." << std::endl;
}

// Test: Token utility methods
void testTokenMethods() {
    std::cout << "Running testTokenMethods..." << std::endl;
    
    Token andToken(TokenType::AND, "&&", SourceLocation(1, 1, 0));
    assert(andToken.isOperator());
    assert(!andToken.isPunctuation());
    
    Token parenToken(TokenType::LPAREN, "(", SourceLocation(1, 1, 0));
    assert(!parenToken.isOperator());
    assert(parenToken.isPunctuation());
    
    Token idToken(TokenType::IDENTIFIER, "foo", SourceLocation(1, 1, 0));
    assert(!idToken.isOperator());
    assert(!idToken.isPunctuation());
    
    std::cout << "Test passed: Token methods work correctly." << std::endl;
}

// Test: Error on unknown character
void testUnknownCharacterError() {
    std::cout << "Running testUnknownCharacterError..." << std::endl;
    
    Lexer lexer;
    bool errorThrown = false;
    
    try {
        lexer.tokenize("foo @ bar");
    } catch (const LexerError& e) {
        errorThrown = true;
        assert(e.location.column == 5);  // @ is at column 5
    }
    
    assert(errorThrown);
    
    std::cout << "Test passed: Unknown character error thrown correctly." << std::endl;
}

// Test: Complex expression
void testComplexExpression() {
    std::cout << "Running testComplexExpression..." << std::endl;
    
    Lexer lexer;
    auto tokens = lexer.tokenizeContent("(A && B) || (C -> D)");
    
    // Expected: ( A && B ) || ( C -> D )
    assert(tokens.size() == 11);
    assert(tokens[0].type == TokenType::LPAREN);
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].value == "A");
    assert(tokens[2].type == TokenType::AND);
    assert(tokens[3].type == TokenType::IDENTIFIER);
    assert(tokens[4].type == TokenType::RPAREN);
    assert(tokens[5].type == TokenType::OR);
    assert(tokens[6].type == TokenType::LPAREN);
    assert(tokens[7].type == TokenType::IDENTIFIER);
    assert(tokens[8].type == TokenType::IMPLIES);
    assert(tokens[9].type == TokenType::IDENTIFIER);
    assert(tokens[10].type == TokenType::RPAREN);
    
    std::cout << "Test passed: Complex expression tokenized correctly." << std::endl;
}

int main() {
    testIdentifiers();
    testHyphenatedIdentifiers();
    testOperators();
    testPunctuation();
    testKeywordOperators();
    testAssumptionsFormat();
    testFactsFormat();
    testNegatedIdentifiers();
    testSourceLocations();
    testNewlineHandling();
    testComments();
    testEmptyInput();
    testTokenMethods();
    testUnknownCharacterError();
    testComplexExpression();
    
    std::cout << "\nAll Lexer tests passed successfully!" << std::endl;
    return 0;
}

