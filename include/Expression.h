#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <map>
#include <queue>
#include <stack>
#include <vector>
#include "Proposition.h"

/**
 * Token represents either an operand (Proposition) or an operator in an expression.
 */
struct Token {
  bool isOperand;
  Proposition prop;       // Valid if isOperand == true
  LogicalOperator op;     // Valid if isOperand == false
  
  // Construct operand token
  explicit Token(const Proposition& p) : isOperand(true), prop(p), op(LogicalOperator::NONE) {}
  
  // Construct operator token
  explicit Token(LogicalOperator o) : isOperand(false), prop(), op(o) {}
};

class Expression {
 private:
  std::vector<Token> tokens;                 // Unified token stream
  std::vector<Proposition> operands;         // Legacy: Holds propositions
  std::vector<LogicalOperator> operators;    // Legacy: Holds logical operators
  std::map<LogicalOperator, int> precedence; // Precedence map for logical operators
  Tripartite evaluatedValue;                 // Holds the evaluated value of the expression
  bool isEvaluated;                          // Flag to check if the expression has been evaluated
  std::string prefix;                        // Prefix for the expression
  bool useTokenStream;                       // True if using new token-based API

  // Helper method to initialize operator precedence
  void initializePrecedence();

  // Helper to check if an operator is unary
  static bool isUnaryOperator(LogicalOperator op);

  // Helper to check if token is a parenthesis
  static bool isLeftParen(LogicalOperator op);
  static bool isRightParen(LogicalOperator op);

  // Helper method to perform a unary logical operation
  static Tripartite applyUnaryOperator(Tripartite value, LogicalOperator op);

  // Helper method to perform a binary logical operation
  static Tripartite applyBinaryOperator(Tripartite left, Tripartite right, LogicalOperator op);

  // Helper method to convert token stream to postfix (new API)
  Tripartite evaluateTokenStream();

  // Legacy: Helper method to convert the expression to postfix notation
  void convertToPostfix(std::queue<Tripartite>& postfixQueue,
                        std::queue<LogicalOperator>& opQueue);

  // Legacy: Helper method to evaluate the postfix expression
  Tripartite evaluatePostfix(std::queue<Tripartite>& postfixQueue,
                             std::queue<LogicalOperator>& opQueue);

 public:
  // Constructors
  Expression();

  // Constructor for a simple two-operand expression
  Expression(const Proposition& left,
             const Proposition& right,
             LogicalOperator op);

  ~Expression();

  void setPrefix(const std::string& p);

  // Methods to add operands and operators (legacy API)
  void addOperand(const Proposition& prop);
  void addOperator(LogicalOperator op);
  
  // Token-based API for expressions with parentheses
  void addToken(const Proposition& prop);    // Add operand token
  void addToken(LogicalOperator op);         // Add operator token
  
  // Convenience methods for parentheses (uses token API)
  void openParen();   // Add LPAREN
  void closeParen();  // Add RPAREN

  // Evaluate the logical expression and return the result
  Tripartite evaluate();

  // Get the evaluated value of the expression
  Tripartite getEvaluatedValue() const;

  // Get the prefix of the expression
  const std::string& getPrefix() const;

  // Reset expression
  void reset();
};

#endif  // EXPRESSION_H
