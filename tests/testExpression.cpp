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

int main() {
    std::cout << "Running tests for Expression class...\n";
    testSimpleExpression();
    testComplexExpression();
    testPrefixSetting();
    testOperatorPrecedence();
    testExpressionReset();
    testUnaryNotOperator();

    std::cout << "All tests passed successfully.\n";
    return 0;
}
