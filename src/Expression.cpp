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
  precedence[LogicalOperator::NOT] = 3;
  precedence[LogicalOperator::AND] = 2;
  precedence[LogicalOperator::OR] = 1;
  precedence[LogicalOperator::IMPLIES] = 0;
  precedence[LogicalOperator::EQUIVALENT] = 0;
}

// Helper function to perform logical deduction based on an operator
Tripartite Expression::performDeduction(const Proposition& left,
                                        const Proposition& right,
                                        LogicalOperator op) {

  Tripartite leftValue = left.getTruthValue();
  Tripartite rightValue = right.getTruthValue();

  switch (op) {
    case LogicalOperator::NOT:
      return !leftValue;
    case LogicalOperator::AND:
      return leftValue && rightValue;
    case LogicalOperator::OR:
      return leftValue || rightValue;
    case LogicalOperator::IMPLIES:
      return implies(leftValue, rightValue);
    case LogicalOperator::EQUIVALENT:
      return (implies(leftValue, rightValue) == Tripartite::TRUE &&
              implies(rightValue, leftValue) == Tripartite::TRUE)
                 ? Tripartite::TRUE
                 : Tripartite::FALSE;
    default:
      throw std::invalid_argument("Invalid operator for deduction.");
  }
}

// Convert infix expression to postfix notation using the Shunting-Yard
// algorithm
void Expression::convertToPostfix(std::queue<Tripartite>& postfixQueue,
                                  std::queue<LogicalOperator>& opQueue) {
  std::stack<LogicalOperator> opStack;

  // Push the first operand directly
  postfixQueue.push(operands[0].getTruthValue());

  for (size_t i = 0; i < operators.size(); ++i) {
    LogicalOperator currentOp = operators[i];

    // Move operators with higher or equal precedence from opStack to opQueue
    while (!opStack.empty() &&
           precedence[currentOp] <= precedence[opStack.top()]) {
      opQueue.push(opStack.top());
      opStack.pop();
    }

    // Push the current operator onto the stack
    opStack.push(currentOp);

    // Push the next operand into postfixQueue
    postfixQueue.push(operands[i + 1].getTruthValue());
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

  // Process each operator in opQueue, applying it to the top two elements in
  // evalStack
  while (!opQueue.empty()) {
    if (evalStack.size() < 2) {
      throw std::runtime_error(
          "Insufficient operands in the stack for evaluation.");
    }

    // Pop the top two operands
    Tripartite right = evalStack.top();
    evalStack.pop();
    Tripartite left = evalStack.top();
    evalStack.pop();

    // Apply the current operator
    LogicalOperator op = opQueue.front();
    opQueue.pop();
    Tripartite result = performDeduction(left, right, op);

    // Push the result of the operation back onto the stack
    evalStack.push(result);
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
