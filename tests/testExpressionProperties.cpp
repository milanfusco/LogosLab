/**
 * Property-based tests for Expression class using RapidCheck.
 * 
 * These tests verify logical properties that should hold for any inputs:
 * - Boolean algebra laws (identity, annihilation, complement, etc.)
 * - De Morgan's laws
 * - Implication equivalences
 * - Evaluation consistency
 */

#include <rapidcheck.h>
#include <iostream>
#include <string>
#include "Expression.h"
#include "Proposition.h"

// ============================================================
// GENERATORS
// ============================================================

namespace rc {

/**
 * Generator for Tripartite values.
 */
template<>
struct Arbitrary<Tripartite> {
    static Gen<Tripartite> arbitrary() {
        return gen::element(Tripartite::TRUE, Tripartite::FALSE, Tripartite::UNKNOWN);
    }
};

/**
 * Generator for definite (non-UNKNOWN) Tripartite values.
 * Used for properties that only hold for classical boolean values.
 */
Gen<Tripartite> genDefinite() {
    return gen::element(Tripartite::TRUE, Tripartite::FALSE);
}

/**
 * Generator for Proposition with a given truth value.
 */
Gen<Proposition> genProposition(Tripartite value) {
    return gen::map(gen::arbitrary<std::string>(), [value](std::string name) {
        Proposition p;
        p.setPrefix(name.empty() ? "prop" : name.substr(0, 10));
        p.setTruthValue(value);
        return p;
    });
}

/**
 * Generator for Proposition with arbitrary truth value.
 */
template<>
struct Arbitrary<Proposition> {
    static Gen<Proposition> arbitrary() {
        return gen::mapcat(gen::arbitrary<Tripartite>(), [](Tripartite value) {
            return genProposition(value);
        });
    }
};

/**
 * Generator for binary logical operators (excluding NOT and parens).
 */
Gen<LogicalOperator> genBinaryOperator() {
    return gen::element(
        LogicalOperator::AND,
        LogicalOperator::OR,
        LogicalOperator::IMPLIES,
        LogicalOperator::EQUIVALENT
    );
}

} // namespace rc

// ============================================================
// HELPER FUNCTIONS
// ============================================================

/**
 * Create a proposition with the given truth value and name.
 */
Proposition makeProp(Tripartite value, const std::string& name = "p") {
    Proposition p;
    p.setPrefix(name);
    p.setTruthValue(value);
    return p;
}

/**
 * Create a simple binary expression: left op right
 */
Expression makeBinaryExpr(const Proposition& left, LogicalOperator op, const Proposition& right) {
    Expression expr;
    expr.addToken(left);
    expr.addToken(op);
    expr.addToken(right);
    return expr;
}

/**
 * Create a unary NOT expression: NOT prop
 */
Expression makeNotExpr(const Proposition& prop) {
    Expression expr;
    expr.addToken(LogicalOperator::NOT);
    expr.addToken(prop);
    return expr;
}

/**
 * Create expression: NOT (left op right)
 */
Expression makeNotBinaryExpr(const Proposition& left, LogicalOperator op, const Proposition& right) {
    Expression expr;
    expr.addToken(LogicalOperator::NOT);
    expr.openParen();
    expr.addToken(left);
    expr.addToken(op);
    expr.addToken(right);
    expr.closeParen();
    return expr;
}

// ============================================================
// PROPERTY TESTS
// ============================================================

int main() {
    int failures = 0;
    
    std::cout << "Running property-based tests for Expression..." << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // --------------------------------------------------------
    // Double Negation: NOT NOT A = A
    // --------------------------------------------------------
    std::cout << "\nProperty: Double Negation (NOT NOT A = A)" << std::endl;
    auto doubleNegation = rc::check("Double negation: NOT NOT A = A",
        [](Tripartite value) {
            Proposition p = makeProp(value, "A");
            
            // NOT NOT A
            Expression notNotA;
            notNotA.addToken(LogicalOperator::NOT);
            notNotA.addToken(LogicalOperator::NOT);
            notNotA.addToken(p);
            
            RC_ASSERT(notNotA.evaluate() == value);
        });
    if (!doubleNegation) failures++;
    
    // --------------------------------------------------------
    // Idempotence: A && A = A, A || A = A
    // --------------------------------------------------------
    std::cout << "\nProperty: Idempotence (A && A = A)" << std::endl;
    auto idempotentAnd = rc::check("Idempotence AND: A && A = A",
        [](Tripartite value) {
            Proposition p = makeProp(value, "A");
            Expression expr = makeBinaryExpr(p, LogicalOperator::AND, p);
            RC_ASSERT(expr.evaluate() == value);
        });
    if (!idempotentAnd) failures++;
    
    std::cout << "\nProperty: Idempotence (A || A = A)" << std::endl;
    auto idempotentOr = rc::check("Idempotence OR: A || A = A",
        [](Tripartite value) {
            Proposition p = makeProp(value, "A");
            Expression expr = makeBinaryExpr(p, LogicalOperator::OR, p);
            RC_ASSERT(expr.evaluate() == value);
        });
    if (!idempotentOr) failures++;
    
    // --------------------------------------------------------
    // Identity Laws: A && TRUE = A, A || FALSE = A
    // --------------------------------------------------------
    std::cout << "\nProperty: Identity (A && TRUE = A)" << std::endl;
    auto identityAnd = rc::check("Identity AND: A && TRUE = A",
        [](Tripartite value) {
            Proposition a = makeProp(value, "A");
            Proposition t = makeProp(Tripartite::TRUE, "T");
            Expression expr = makeBinaryExpr(a, LogicalOperator::AND, t);
            RC_ASSERT(expr.evaluate() == value);
        });
    if (!identityAnd) failures++;
    
    std::cout << "\nProperty: Identity (A || FALSE = A)" << std::endl;
    auto identityOr = rc::check("Identity OR: A || FALSE = A",
        [](Tripartite value) {
            Proposition a = makeProp(value, "A");
            Proposition f = makeProp(Tripartite::FALSE, "F");
            Expression expr = makeBinaryExpr(a, LogicalOperator::OR, f);
            RC_ASSERT(expr.evaluate() == value);
        });
    if (!identityOr) failures++;
    
    // --------------------------------------------------------
    // Annihilation Laws: A && FALSE = FALSE, A || TRUE = TRUE
    // --------------------------------------------------------
    std::cout << "\nProperty: Annihilation (A && FALSE = FALSE)" << std::endl;
    auto annihilationAnd = rc::check("Annihilation AND: A && FALSE = FALSE",
        [](Tripartite value) {
            Proposition a = makeProp(value, "A");
            Proposition f = makeProp(Tripartite::FALSE, "F");
            Expression expr = makeBinaryExpr(a, LogicalOperator::AND, f);
            RC_ASSERT(expr.evaluate() == Tripartite::FALSE);
        });
    if (!annihilationAnd) failures++;
    
    std::cout << "\nProperty: Annihilation (A || TRUE = TRUE)" << std::endl;
    auto annihilationOr = rc::check("Annihilation OR: A || TRUE = TRUE",
        [](Tripartite value) {
            Proposition a = makeProp(value, "A");
            Proposition t = makeProp(Tripartite::TRUE, "T");
            Expression expr = makeBinaryExpr(a, LogicalOperator::OR, t);
            RC_ASSERT(expr.evaluate() == Tripartite::TRUE);
        });
    if (!annihilationOr) failures++;
    
    // --------------------------------------------------------
    // Commutativity: A && B = B && A, A || B = B || A
    // --------------------------------------------------------
    std::cout << "\nProperty: Commutativity (A && B = B && A)" << std::endl;
    auto commutativeAnd = rc::check("Commutativity AND: A && B = B && A",
        [](Tripartite a, Tripartite b) {
            Proposition propA = makeProp(a, "A");
            Proposition propB = makeProp(b, "B");
            
            Expression ab = makeBinaryExpr(propA, LogicalOperator::AND, propB);
            Expression ba = makeBinaryExpr(propB, LogicalOperator::AND, propA);
            
            RC_ASSERT(ab.evaluate() == ba.evaluate());
        });
    if (!commutativeAnd) failures++;
    
    std::cout << "\nProperty: Commutativity (A || B = B || A)" << std::endl;
    auto commutativeOr = rc::check("Commutativity OR: A || B = B || A",
        [](Tripartite a, Tripartite b) {
            Proposition propA = makeProp(a, "A");
            Proposition propB = makeProp(b, "B");
            
            Expression ab = makeBinaryExpr(propA, LogicalOperator::OR, propB);
            Expression ba = makeBinaryExpr(propB, LogicalOperator::OR, propA);
            
            RC_ASSERT(ab.evaluate() == ba.evaluate());
        });
    if (!commutativeOr) failures++;
    
    // --------------------------------------------------------
    // Complement Laws (for definite values only):
    // A && NOT A = FALSE, A || NOT A = TRUE
    // --------------------------------------------------------
    std::cout << "\nProperty: Complement (A && NOT A = FALSE, for definite values)" << std::endl;
    auto complementAnd = rc::check("Complement AND: A && NOT A = FALSE",
        []() {
            Tripartite value = *rc::genDefinite();
            Proposition a = makeProp(value, "A");
            
            // A && NOT A
            Expression expr;
            expr.addToken(a);
            expr.addToken(LogicalOperator::AND);
            expr.addToken(LogicalOperator::NOT);
            expr.addToken(a);
            
            RC_ASSERT(expr.evaluate() == Tripartite::FALSE);
        });
    if (!complementAnd) failures++;
    
    std::cout << "\nProperty: Complement (A || NOT A = TRUE, for definite values)" << std::endl;
    auto complementOr = rc::check("Complement OR: A || NOT A = TRUE",
        []() {
            Tripartite value = *rc::genDefinite();
            Proposition a = makeProp(value, "A");
            
            // A || NOT A
            Expression expr;
            expr.addToken(a);
            expr.addToken(LogicalOperator::OR);
            expr.addToken(LogicalOperator::NOT);
            expr.addToken(a);
            
            RC_ASSERT(expr.evaluate() == Tripartite::TRUE);
        });
    if (!complementOr) failures++;
    
    // --------------------------------------------------------
    // De Morgan's Laws (for definite values):
    // NOT (A && B) = NOT A || NOT B
    // NOT (A || B) = NOT A && NOT B
    // --------------------------------------------------------
    std::cout << "\nProperty: De Morgan (NOT (A && B) = NOT A || NOT B)" << std::endl;
    auto deMorganAnd = rc::check("De Morgan AND: NOT (A && B) = NOT A || NOT B",
        []() {
            Tripartite a = *rc::genDefinite();
            Tripartite b = *rc::genDefinite();
            Proposition propA = makeProp(a, "A");
            Proposition propB = makeProp(b, "B");
            
            // NOT (A && B)
            Expression lhs = makeNotBinaryExpr(propA, LogicalOperator::AND, propB);
            
            // NOT A || NOT B
            Expression rhs;
            rhs.addToken(LogicalOperator::NOT);
            rhs.addToken(propA);
            rhs.addToken(LogicalOperator::OR);
            rhs.addToken(LogicalOperator::NOT);
            rhs.addToken(propB);
            
            RC_ASSERT(lhs.evaluate() == rhs.evaluate());
        });
    if (!deMorganAnd) failures++;
    
    std::cout << "\nProperty: De Morgan (NOT (A || B) = NOT A && NOT B)" << std::endl;
    auto deMorganOr = rc::check("De Morgan OR: NOT (A || B) = NOT A && NOT B",
        []() {
            Tripartite a = *rc::genDefinite();
            Tripartite b = *rc::genDefinite();
            Proposition propA = makeProp(a, "A");
            Proposition propB = makeProp(b, "B");
            
            // NOT (A || B)
            Expression lhs = makeNotBinaryExpr(propA, LogicalOperator::OR, propB);
            
            // NOT A && NOT B
            Expression rhs;
            rhs.addToken(LogicalOperator::NOT);
            rhs.addToken(propA);
            rhs.addToken(LogicalOperator::AND);
            rhs.addToken(LogicalOperator::NOT);
            rhs.addToken(propB);
            
            RC_ASSERT(lhs.evaluate() == rhs.evaluate());
        });
    if (!deMorganOr) failures++;
    
    // --------------------------------------------------------
    // Implication Definition: A -> B = NOT A || B
    // --------------------------------------------------------
    std::cout << "\nProperty: Implication (A -> B = NOT A || B)" << std::endl;
    auto implicationDef = rc::check("Implication: A -> B = NOT A || B",
        [](Tripartite a, Tripartite b) {
            Proposition propA = makeProp(a, "A");
            Proposition propB = makeProp(b, "B");
            
            // A -> B
            Expression lhs = makeBinaryExpr(propA, LogicalOperator::IMPLIES, propB);
            
            // NOT A || B
            Expression rhs;
            rhs.addToken(LogicalOperator::NOT);
            rhs.addToken(propA);
            rhs.addToken(LogicalOperator::OR);
            rhs.addToken(propB);
            
            RC_ASSERT(lhs.evaluate() == rhs.evaluate());
        });
    if (!implicationDef) failures++;
    
    // --------------------------------------------------------
    // Contraposition: (A -> B) = (NOT B -> NOT A)
    // --------------------------------------------------------
    std::cout << "\nProperty: Contraposition ((A -> B) = (NOT B -> NOT A))" << std::endl;
    auto contraposition = rc::check("Contraposition: (A -> B) = (NOT B -> NOT A)",
        []() {
            Tripartite a = *rc::genDefinite();
            Tripartite b = *rc::genDefinite();
            Proposition propA = makeProp(a, "A");
            Proposition propB = makeProp(b, "B");
            
            // A -> B
            Expression lhs = makeBinaryExpr(propA, LogicalOperator::IMPLIES, propB);
            
            // NOT B -> NOT A
            Proposition notB = makeProp(!b, "notB");
            Proposition notA = makeProp(!a, "notA");
            Expression rhs = makeBinaryExpr(notB, LogicalOperator::IMPLIES, notA);
            
            RC_ASSERT(lhs.evaluate() == rhs.evaluate());
        });
    if (!contraposition) failures++;
    
    // --------------------------------------------------------
    // Evaluation Consistency: evaluating twice gives same result
    // --------------------------------------------------------
    std::cout << "\nProperty: Evaluation Consistency" << std::endl;
    auto evalConsistency = rc::check("Evaluation consistency: evaluate twice gives same result",
        [](Tripartite a, Tripartite b) {
            Proposition propA = makeProp(a, "A");
            Proposition propB = makeProp(b, "B");
            LogicalOperator op = *rc::genBinaryOperator();
            
            Expression expr = makeBinaryExpr(propA, op, propB);
            
            Tripartite first = expr.evaluate();
            Tripartite second = expr.evaluate();
            
            RC_ASSERT(first == second);
        });
    if (!evalConsistency) failures++;
    
    // --------------------------------------------------------
    // Associativity: (A && B) && C = A && (B && C)
    // --------------------------------------------------------
    std::cout << "\nProperty: Associativity ((A && B) && C = A && (B && C))" << std::endl;
    auto associativeAnd = rc::check("Associativity AND: (A && B) && C = A && (B && C)",
        [](Tripartite a, Tripartite b, Tripartite c) {
            Proposition propA = makeProp(a, "A");
            Proposition propB = makeProp(b, "B");
            Proposition propC = makeProp(c, "C");
            
            // (A && B) && C
            Expression lhs;
            lhs.openParen();
            lhs.addToken(propA);
            lhs.addToken(LogicalOperator::AND);
            lhs.addToken(propB);
            lhs.closeParen();
            lhs.addToken(LogicalOperator::AND);
            lhs.addToken(propC);
            
            // A && (B && C)
            Expression rhs;
            rhs.addToken(propA);
            rhs.addToken(LogicalOperator::AND);
            rhs.openParen();
            rhs.addToken(propB);
            rhs.addToken(LogicalOperator::AND);
            rhs.addToken(propC);
            rhs.closeParen();
            
            RC_ASSERT(lhs.evaluate() == rhs.evaluate());
        });
    if (!associativeAnd) failures++;
    
    std::cout << "\nProperty: Associativity ((A || B) || C = A || (B || C))" << std::endl;
    auto associativeOr = rc::check("Associativity OR: (A || B) || C = A || (B || C)",
        [](Tripartite a, Tripartite b, Tripartite c) {
            Proposition propA = makeProp(a, "A");
            Proposition propB = makeProp(b, "B");
            Proposition propC = makeProp(c, "C");
            
            // (A || B) || C
            Expression lhs;
            lhs.openParen();
            lhs.addToken(propA);
            lhs.addToken(LogicalOperator::OR);
            lhs.addToken(propB);
            lhs.closeParen();
            lhs.addToken(LogicalOperator::OR);
            lhs.addToken(propC);
            
            // A || (B || C)
            Expression rhs;
            rhs.addToken(propA);
            rhs.addToken(LogicalOperator::OR);
            rhs.openParen();
            rhs.addToken(propB);
            rhs.addToken(LogicalOperator::OR);
            rhs.addToken(propC);
            rhs.closeParen();
            
            RC_ASSERT(lhs.evaluate() == rhs.evaluate());
        });
    if (!associativeOr) failures++;
    
    // --------------------------------------------------------
    // Distributivity: A && (B || C) = (A && B) || (A && C)
    // --------------------------------------------------------
    std::cout << "\nProperty: Distributivity (A && (B || C) = (A && B) || (A && C))" << std::endl;
    auto distributive = rc::check("Distributivity: A && (B || C) = (A && B) || (A && C)",
        [](Tripartite a, Tripartite b, Tripartite c) {
            Proposition propA = makeProp(a, "A");
            Proposition propB = makeProp(b, "B");
            Proposition propC = makeProp(c, "C");
            
            // A && (B || C)
            Expression lhs;
            lhs.addToken(propA);
            lhs.addToken(LogicalOperator::AND);
            lhs.openParen();
            lhs.addToken(propB);
            lhs.addToken(LogicalOperator::OR);
            lhs.addToken(propC);
            lhs.closeParen();
            
            // (A && B) || (A && C)
            Expression rhs;
            rhs.openParen();
            rhs.addToken(propA);
            rhs.addToken(LogicalOperator::AND);
            rhs.addToken(propB);
            rhs.closeParen();
            rhs.addToken(LogicalOperator::OR);
            rhs.openParen();
            rhs.addToken(propA);
            rhs.addToken(LogicalOperator::AND);
            rhs.addToken(propC);
            rhs.closeParen();
            
            RC_ASSERT(lhs.evaluate() == rhs.evaluate());
        });
    if (!distributive) failures++;
    
    // --------------------------------------------------------
    // Summary
    // --------------------------------------------------------
    std::cout << "\n=============================================" << std::endl;
    if (failures == 0) {
        std::cout << "All property tests PASSED!" << std::endl;
    } else {
        std::cout << failures << " property test(s) FAILED!" << std::endl;
    }
    
    return failures;
}

