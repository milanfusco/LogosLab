#include "Expression.h"
#include "Proposition.h"
#include <cassert>
#include <iostream>

void testSimpleExpression() {
    // Test a simple expression: TRUE AND FALSE
    std::cout << "Testing a simple expression: P AND Q\n";
    Proposition pTrue("P", Tripartite::TRUE);
    Proposition pFalse("Q", Tripartite::FALSE);
    Expression expr(pTrue, pFalse, LogicalOperator::AND);

    // Evaluate the expression
    Tripartite result = expr.evaluate();
    std::cout << "Expected: P = TRUE, Q = FALSE. P AND Q = FALSE\n";
    std::cout << "P = " << static_cast<int>(pTrue.getTruthValue()) << ", Q = " << static_cast<int>(pFalse.getTruthValue()) 
              << ". P AND Q = " << static_cast<int>(result) << std::endl;
    assert(result == Tripartite::FALSE);
    assert(expr.getEvaluatedValue() == Tripartite::FALSE);

    std::cout << "testSimpleExpression passed.\n";
}

void testComplexExpression() {
    std::cout << "Testing a complex expression: (P AND Q) OR P\n";
    // Test a complex expression: (TRUE AND FALSE) OR TRUE
    Proposition pTrue("P", Tripartite::TRUE);
    Proposition pFalse("Q", Tripartite::FALSE);

    Expression expr;
    expr.reset();
    expr.addOperand(pTrue);
    expr.addOperator(LogicalOperator::AND);
    expr.addOperand(pFalse);
    expr.addOperator(LogicalOperator::OR);
    expr.addOperand(pTrue);
    Tripartite result = expr.evaluate();

    // Evaluate the expression
    std::cout << "Expected: P = TRUE, Q = FALSE. (P AND Q) OR P = TRUE\n";
    std::cout << "P = " << static_cast<int>(pTrue.getTruthValue()) << ", Q = " << static_cast<int>(pFalse.getTruthValue()) 
              << ". (P AND Q) OR P = " << static_cast<int>(result) << std::endl;
    assert(result == Tripartite::TRUE); // (TRUE AND FALSE) OR TRUE should be TRUE
    assert(expr.getEvaluatedValue() == Tripartite::TRUE);

    std::cout << "testComplexExpression passed.\n";
}

void testPrefixSetting() {
    // Test setting and retrieving prefix
    std::cout << "Testing setting and retrieving prefix for an expression\n";
    Expression expr;
    expr.setPrefix("expr1");
    std::cout << "Prefix set: " << expr.getPrefix() << std::endl;
    assert(expr.getPrefix() == "expr1");

    std::cout << "testPrefixSetting passed.\n";
}

void testOperatorPrecedence() {
    std::cout << "Testing operator precedence in an expression\n";
    // Test expression with precedence: TRUE OR FALSE AND FALSE
    Proposition pTrue("P", Tripartite::TRUE);
    Proposition pFalse("Q", Tripartite::FALSE);

    Expression expr;
    expr.reset();
    expr.addOperand(pTrue);
    expr.addOperator(LogicalOperator::OR);
    expr.addOperand(pFalse);
    expr.addOperator(LogicalOperator::AND);
    expr.addOperand(pFalse);

    // Evaluate the expression
    Tripartite result = expr.evaluate();
    std::cout << "Expected: P = TRUE, Q = FALSE. P OR (Q AND Q) = TRUE\n";
    std::cout << "P = " << static_cast<int>(pTrue.getTruthValue()) << ", Q = " << static_cast<int>(pFalse.getTruthValue()) 
              << ". P OR (Q AND Q) = " << static_cast<int>(result) << std::endl;
    assert(result == Tripartite::TRUE); // TRUE OR (FALSE AND FALSE) should be TRUE
    assert(expr.getEvaluatedValue() == Tripartite::TRUE);

    std::cout << "testOperatorPrecedence passed.\n";
}

void testExpressionReset() {
    // Test resetting an expression
    std::cout << "Testing resetting an expression\n";
    Proposition pTrue("P", Tripartite::TRUE);
    Proposition pFalse("Q", Tripartite::FALSE);

    Expression expr(pTrue, pFalse, LogicalOperator::OR);
    Tripartite initialResult = expr.evaluate();
    std::cout << "Initial evaluation: P OR Q = " << static_cast<int>(initialResult) << std::endl;
    assert(initialResult == Tripartite::TRUE); // Initial evaluation: TRUE OR FALSE is TRUE

    expr.reset(); // Reset the expression
    assert(expr.getEvaluatedValue() == Tripartite::UNKNOWN); // Check that `evaluatedValue` is reset

    expr.addOperand(pTrue);
    expr.addOperator(LogicalOperator::AND);
    expr.addOperand(pFalse);
    Tripartite finalResult = expr.evaluate();
    std::cout << "After adding operands: P AND Q = " << static_cast<int>(finalResult) << std::endl;
    assert(finalResult == Tripartite::FALSE); // TRUE AND FALSE should now be FALSE after reset

    std::cout << "testExpressionReset passed.\n";
}

void testUnaryNotOperator() {
    // Test unary NOT operator: NOT TRUE = FALSE
    std::cout << "Testing unary NOT operator\n";
    
    // Test 1: Simple NOT TRUE = FALSE
    {
        Proposition pTrue("P", Tripartite::TRUE);
        Expression expr;
        expr.addOperator(LogicalOperator::NOT);
        expr.addOperand(pTrue);
        Tripartite result = expr.evaluate();
        std::cout << "NOT TRUE = " << static_cast<int>(result) << " (expected FALSE=1)\n";
        assert(result == Tripartite::FALSE);
    }
    
    // Test 2: NOT FALSE = TRUE
    {
        Proposition pFalse("Q", Tripartite::FALSE);
        Expression expr;
        expr.addOperator(LogicalOperator::NOT);
        expr.addOperand(pFalse);
        Tripartite result = expr.evaluate();
        std::cout << "NOT FALSE = " << static_cast<int>(result) << " (expected TRUE=0)\n";
        assert(result == Tripartite::TRUE);
    }
    
    // Test 3: NOT UNKNOWN = UNKNOWN
    {
        Proposition pUnknown("R", Tripartite::UNKNOWN);
        Expression expr;
        expr.addOperator(LogicalOperator::NOT);
        expr.addOperand(pUnknown);
        Tripartite result = expr.evaluate();
        std::cout << "NOT UNKNOWN = " << static_cast<int>(result) << " (expected UNKNOWN=-1)\n";
        assert(result == Tripartite::UNKNOWN);
    }
    
    // Test 4: NOT P AND Q (should be (NOT P) AND Q due to precedence)
    // NOT TRUE AND TRUE = FALSE AND TRUE = FALSE
    {
        Proposition pTrue("P", Tripartite::TRUE);
        Proposition qTrue("Q", Tripartite::TRUE);
        Expression expr;
        expr.addOperator(LogicalOperator::NOT);
        expr.addOperand(pTrue);
        expr.addOperator(LogicalOperator::AND);
        expr.addOperand(qTrue);
        Tripartite result = expr.evaluate();
        std::cout << "NOT TRUE AND TRUE = " << static_cast<int>(result) << " (expected FALSE=1)\n";
        assert(result == Tripartite::FALSE);
    }
    
    // Test 5: P AND NOT Q
    // TRUE AND NOT FALSE = TRUE AND TRUE = TRUE
    {
        Proposition pTrue("P", Tripartite::TRUE);
        Proposition qFalse("Q", Tripartite::FALSE);
        Expression expr;
        expr.addOperand(pTrue);
        expr.addOperator(LogicalOperator::AND);
        expr.addOperator(LogicalOperator::NOT);
        expr.addOperand(qFalse);
        Tripartite result = expr.evaluate();
        std::cout << "TRUE AND NOT FALSE = " << static_cast<int>(result) << " (expected TRUE=0)\n";
        assert(result == Tripartite::TRUE);
    }
    
    std::cout << "testUnaryNotOperator passed.\n";
}

void testParenthesesGrouping() {
    std::cout << "Testing parentheses grouping\n";
    
    // Test 1: (A OR B) AND C using token API
    // (FALSE OR TRUE) AND TRUE = TRUE AND TRUE = TRUE
    {
        Proposition pFalse("A", Tripartite::FALSE);
        Proposition pTrue1("B", Tripartite::TRUE);
        Proposition pTrue2("C", Tripartite::TRUE);
        
        Expression expr;
        expr.openParen();                           // (
        expr.addToken(pFalse);                      // A
        expr.addToken(LogicalOperator::OR);         // OR
        expr.addToken(pTrue1);                      // B
        expr.closeParen();                          // )
        expr.addToken(LogicalOperator::AND);        // AND
        expr.addToken(pTrue2);                      // C
        
        Tripartite result = expr.evaluate();
        std::cout << "(FALSE OR TRUE) AND TRUE = " << static_cast<int>(result) << " (expected TRUE=0)\n";
        assert(result == Tripartite::TRUE);
    }
    
    // Test 2: Compare with non-parenthesized version
    // A OR B AND C where A=TRUE, B=FALSE, C=FALSE
    // = TRUE OR (FALSE AND FALSE) = TRUE OR FALSE = TRUE
    // (A OR B) AND C = (TRUE OR FALSE) AND FALSE = TRUE AND FALSE = FALSE
    {
        Proposition pTrue("A", Tripartite::TRUE);
        Proposition pFalse1("B", Tripartite::FALSE);
        Proposition pFalse2("C", Tripartite::FALSE);
        
        // Without parentheses: A OR B AND C (legacy API)
        Expression exprNoParen;
        exprNoParen.addOperand(pTrue);
        exprNoParen.addOperator(LogicalOperator::OR);
        exprNoParen.addOperand(pFalse1);
        exprNoParen.addOperator(LogicalOperator::AND);
        exprNoParen.addOperand(pFalse2);
        Tripartite resultNoParen = exprNoParen.evaluate();
        std::cout << "TRUE OR FALSE AND FALSE (no parens) = " << static_cast<int>(resultNoParen) << " (expected TRUE=0)\n";
        assert(resultNoParen == Tripartite::TRUE);
        
        // With parentheses: (A OR B) AND C (token API)
        Expression exprParen;
        exprParen.openParen();                        // (
        exprParen.addToken(pTrue);                    // A
        exprParen.addToken(LogicalOperator::OR);      // OR
        exprParen.addToken(pFalse1);                  // B
        exprParen.closeParen();                       // )
        exprParen.addToken(LogicalOperator::AND);     // AND
        exprParen.addToken(pFalse2);                  // C
        Tripartite resultParen = exprParen.evaluate();
        std::cout << "(TRUE OR FALSE) AND FALSE (with parens) = " << static_cast<int>(resultParen) << " (expected FALSE=1)\n";
        assert(resultParen == Tripartite::FALSE);
    }
    
    // Test 3: Nested parentheses: ((A AND B) OR C) AND D
    // ((TRUE AND FALSE) OR TRUE) AND TRUE = (FALSE OR TRUE) AND TRUE = TRUE AND TRUE = TRUE
    {
        Proposition pTrue1("A", Tripartite::TRUE);
        Proposition pFalse("B", Tripartite::FALSE);
        Proposition pTrue2("C", Tripartite::TRUE);
        Proposition pTrue3("D", Tripartite::TRUE);
        
        Expression expr;
        expr.openParen();                             // (
        expr.openParen();                             // (
        expr.addToken(pTrue1);                        // A
        expr.addToken(LogicalOperator::AND);          // AND
        expr.addToken(pFalse);                        // B
        expr.closeParen();                            // )
        expr.addToken(LogicalOperator::OR);           // OR
        expr.addToken(pTrue2);                        // C
        expr.closeParen();                            // )
        expr.addToken(LogicalOperator::AND);          // AND
        expr.addToken(pTrue3);                        // D
        
        Tripartite result = expr.evaluate();
        std::cout << "((TRUE AND FALSE) OR TRUE) AND TRUE = " << static_cast<int>(result) << " (expected TRUE=0)\n";
        assert(result == Tripartite::TRUE);
    }
    
    // Test 4: NOT with parentheses: NOT (A AND B)
    // NOT (TRUE AND FALSE) = NOT FALSE = TRUE
    {
        Proposition pTrue("A", Tripartite::TRUE);
        Proposition pFalse("B", Tripartite::FALSE);
        
        Expression expr;
        expr.addToken(LogicalOperator::NOT);          // NOT
        expr.openParen();                             // (
        expr.addToken(pTrue);                         // A
        expr.addToken(LogicalOperator::AND);          // AND
        expr.addToken(pFalse);                        // B
        expr.closeParen();                            // )
        
        Tripartite result = expr.evaluate();
        std::cout << "NOT (TRUE AND FALSE) = " << static_cast<int>(result) << " (expected TRUE=0)\n";
        assert(result == Tripartite::TRUE);
    }
    
    // Test 5: Compare NOT A AND B vs NOT (A AND B) using token API for both
    // NOT TRUE AND FALSE = (NOT TRUE) AND FALSE = FALSE AND FALSE = FALSE
    // NOT (TRUE AND FALSE) = NOT FALSE = TRUE
    {
        Proposition pTrue("A", Tripartite::TRUE);
        Proposition pFalse("B", Tripartite::FALSE);
        
        // NOT A AND B (no parens) = (NOT A) AND B using token API
        Expression exprNoParen;
        exprNoParen.addToken(LogicalOperator::NOT);   // NOT
        exprNoParen.addToken(pTrue);                  // A
        exprNoParen.addToken(LogicalOperator::AND);   // AND
        exprNoParen.addToken(pFalse);                 // B
        Tripartite resultNoParen = exprNoParen.evaluate();
        std::cout << "NOT TRUE AND FALSE (no parens, token API) = " << static_cast<int>(resultNoParen) << " (expected FALSE=1)\n";
        assert(resultNoParen == Tripartite::FALSE);
        
        // NOT (A AND B) (with parens)
        Expression exprParen;
        exprParen.addToken(LogicalOperator::NOT);     // NOT
        exprParen.openParen();                        // (
        exprParen.addToken(pTrue);                    // A
        exprParen.addToken(LogicalOperator::AND);     // AND
        exprParen.addToken(pFalse);                   // B
        exprParen.closeParen();                       // )
        Tripartite resultParen = exprParen.evaluate();
        std::cout << "NOT (TRUE AND FALSE) (with parens) = " << static_cast<int>(resultParen) << " (expected TRUE=0)\n";
        assert(resultParen == Tripartite::TRUE);
    }
    
    std::cout << "testParenthesesGrouping passed.\n";
}

int main() {
    std::cout << "Running tests for Expression class...\n";
    testSimpleExpression();
    testComplexExpression();
    testPrefixSetting();
    testOperatorPrecedence();
    testExpressionReset();
    testUnaryNotOperator();
    testParenthesesGrouping();

    std::cout << "All tests passed successfully.\n";
    return 0;
}
