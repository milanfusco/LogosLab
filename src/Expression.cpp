#include "Expression.h"
#include <queue>
#include <stack>

// Default constructor
Expression::Expression()
    : evaluatedValue(Tripartite::UNKNOWN), isEvaluated(false) {
  initializePrecedence();
}

// Constructor for a simple two-operand expression
Expression::Expression(const Proposition& left,
                       const Proposition& right,
                       LogicalOperator op)
    : evaluatedValue(Tripartite::UNKNOWN), isEvaluated(false) {
  operands.push_back(left);
  operands.push_back(right);
  operators.push_back(op);
  initializePrecedence();
}

// Destructor
Expression::~Expression() {
  reset();
}

// Set the prefix for the expression
void Expression::setPrefix(const std::string& p) {
  prefix = p;
}

// Add a Proposition as an operand
void Expression::addOperand(const Proposition& prop) {
  operands.push_back(prop);
}

// Add a LogicalOperator as an operator
void Expression::addOperator(LogicalOperator op) {
  operators.push_back(op);
}

// Initialize precedence for logical operators
void Expression::initializePrecedence() {
  precedence[LogicalOperator::NOT] = 3;       // Highest precedence (unary)
  precedence[LogicalOperator::AND] = 2;
  precedence[LogicalOperator::OR] = 1;
  precedence[LogicalOperator::IMPLIES] = 0;
  precedence[LogicalOperator::EQUIVALENT] = 0;
}

// Check if an operator is unary
bool Expression::isUnaryOperator(LogicalOperator op) {
  return op == LogicalOperator::NOT;
}

// Apply a unary operator to a single value
Tripartite Expression::applyUnaryOperator(Tripartite value, LogicalOperator op) {
  switch (op) {
    case LogicalOperator::NOT:
      return !value;
    default:
      throw std::invalid_argument("Invalid unary operator.");
  }
}

// Apply a binary operator to two values
Tripartite Expression::applyBinaryOperator(Tripartite left, Tripartite right, LogicalOperator op) {
  switch (op) {
    case LogicalOperator::AND:
      return left && right;
    case LogicalOperator::OR:
      return left || right;
    case LogicalOperator::IMPLIES:
      return implies(left, right);
    case LogicalOperator::EQUIVALENT:
      return (implies(left, right) == Tripartite::TRUE &&
              implies(right, left) == Tripartite::TRUE)
                 ? Tripartite::TRUE
                 : Tripartite::FALSE;
    default:
      throw std::invalid_argument("Invalid binary operator.");
  }
}

// Convert infix expression to postfix notation using the Shunting-Yard
// algorithm. Handles both unary (NOT) and binary operators.
//
// For unary operators like NOT:
//   - NOT is right-associative and applies to the operand immediately after it
//   - Expression "NOT A AND B" means "(NOT A) AND B"
//   - The NOT operator is pushed onto stack, then when the operand is processed,
//     we immediately pop and apply NOT since it has highest precedence
//
void Expression::convertToPostfix(std::queue<Tripartite>& postfixQueue,
                                  std::queue<LogicalOperator>& opQueue) {
  std::stack<LogicalOperator> opStack;
  size_t operandIndex = 0;
  
  for (size_t i = 0; i < operators.size(); ++i) {
    LogicalOperator currentOp = operators[i];
    
    if (isUnaryOperator(currentOp)) {
      // Unary operator (NOT): push to stack, it will be applied to the next operand
      // Right-associative: don't pop other NOTs yet
      opStack.push(currentOp);
    } else {
      // Binary operator: first push the left operand if we haven't yet
      if (operandIndex < operands.size()) {
        postfixQueue.push(operands[operandIndex].getTruthValue());
        operandIndex++;
        
        // After pushing operand, apply any pending unary operators
        while (!opStack.empty() && isUnaryOperator(opStack.top())) {
          opQueue.push(opStack.top());
          opStack.pop();
        }
      }
      
      // Move operators with higher or equal precedence from opStack to opQueue
      while (!opStack.empty() && !isUnaryOperator(opStack.top()) &&
             precedence[currentOp] <= precedence[opStack.top()]) {
        opQueue.push(opStack.top());
        opStack.pop();
      }
      
      // Push the current binary operator onto the stack
      opStack.push(currentOp);
    }
  }
  
  // Push remaining operands
  while (operandIndex < operands.size()) {
    postfixQueue.push(operands[operandIndex].getTruthValue());
    operandIndex++;
    
    // After pushing operand, apply any pending unary operators
    while (!opStack.empty() && isUnaryOperator(opStack.top())) {
      opQueue.push(opStack.top());
      opStack.pop();
    }
  }

  // Move remaining operators in opStack to opQueue
  while (!opStack.empty()) {
    opQueue.push(opStack.top());
    opStack.pop();
  }
}

Tripartite Expression::evaluatePostfix(std::queue<Tripartite>& postfixQueue,
                                       std::queue<LogicalOperator>& opQueue) {
  std::stack<Tripartite> evalStack;

  // Push all operands onto evalStack in the order they appear in postfixQueue
  while (!postfixQueue.empty()) {
    evalStack.push(postfixQueue.front());
    postfixQueue.pop();
  }

  // Process each operator in opQueue
  while (!opQueue.empty()) {
    LogicalOperator op = opQueue.front();
    opQueue.pop();

    if (isUnaryOperator(op)) {
      // Unary operator: pop one operand
      if (evalStack.empty()) {
        throw std::runtime_error(
            "Insufficient operands in the stack for unary operator.");
      }
      Tripartite operand = evalStack.top();
      evalStack.pop();
      
      Tripartite result = applyUnaryOperator(operand, op);
      evalStack.push(result);
    } else {
      // Binary operator: pop two operands
      if (evalStack.size() < 2) {
        throw std::runtime_error(
            "Insufficient operands in the stack for binary operator.");
      }
      Tripartite right = evalStack.top();
      evalStack.pop();
      Tripartite left = evalStack.top();
      evalStack.pop();
      
      Tripartite result = applyBinaryOperator(left, right, op);
      evalStack.push(result);
    }
  }

  // The final result should be the only remaining value on the stack
  if (evalStack.size() == 1) {
    return evalStack.top();
  } else {
    throw std::runtime_error(
        "Invalid postfix expression: unbalanced operands/operators.");
  }
}

// Main evaluate function that combines convertToPostfix() and evaluatePostfix()
Tripartite Expression::evaluate() {
  if (isEvaluated)
    return evaluatedValue;

  if (operands.empty()) {
    evaluatedValue = Tripartite::UNKNOWN;
    isEvaluated = true;
    return evaluatedValue;
  }

  std::queue<Tripartite> postfixQueue;  // Queue to hold operands
  std::queue<LogicalOperator> opQueue;  // Queue to hold operators

  convertToPostfix(postfixQueue, opQueue);  // Convert to postfix
  evaluatedValue =
      evaluatePostfix(postfixQueue, opQueue);  // Evaluate postfix expression

  isEvaluated = true;
  return evaluatedValue;
}

// Get the evaluated value of the expression
Tripartite Expression::getEvaluatedValue() const {
  return evaluatedValue;
}

// Get the prefix of the expression
const std::string& Expression::getPrefix() const {
  return prefix;
}

// Reset the expression
void Expression::reset() {
  operands.clear();
  operators.clear();
  isEvaluated = false;
  evaluatedValue = Tripartite::UNKNOWN;
}
