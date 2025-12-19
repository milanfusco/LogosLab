#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <map>
#include <queue>
#include <stack>
#include <vector>
#include "Proposition.h"

class Expression {
 private:
  std::vector<Proposition> operands;         // Holds propositions
  std::vector<LogicalOperator> operators;    // Holds logical operators
  std::map<LogicalOperator, int> precedence; // Precedence map for logical operators
  Tripartite evaluatedValue;                 // Holds the evaluated value of the expression
  bool isEvaluated;                          // Flag to check if the expression has been evaluated
  std::string prefix;                        // Prefix for the expression

  // Helper method to initialize operator precedence
  void initializePrecedence();

  // Helper method to perform a logical deduction based on an operator
  Tripartite performDeduction(const Proposition& left,
                              const Proposition& right,
                              LogicalOperator op);

  // Helper method to convert the expression to postfix notation using the
  // Shunting-Yard Algorithm
  void convertToPostfix(std::queue<Tripartite>& postfixQueue,
                        std::queue<LogicalOperator>& opQueue);

  // Helper method to evaluate the postfix expression
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

  // Methods to add operands and operators
  void addOperand(const Proposition& prop);
  void addOperator(LogicalOperator op);

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
