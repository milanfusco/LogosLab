#include "Expression.h"
#include <queue>
#include <stack>

// Named constants for operator precedence
namespace {
    constexpr int PRECEDENCE_NOT = 3;        // Highest: unary negation
    constexpr int PRECEDENCE_AND = 2;        // Conjunction
    constexpr int PRECEDENCE_OR = 1;         // Disjunction
    constexpr int PRECEDENCE_IMPLIES = 0;    // Material implication
    constexpr int PRECEDENCE_EQUIVALENT = 0; // Biconditional
}

// Default constructor
Expression::Expression()
    : evaluatedValue(Tripartite::UNKNOWN), isEvaluated(false), useTokenStream(false) {
  initializePrecedence();
}

// Constructor for a simple two-operand expression
Expression::Expression(const Proposition& left,
                       const Proposition& right,
                       LogicalOperator op)
    : evaluatedValue(Tripartite::UNKNOWN), isEvaluated(false), useTokenStream(false) {
  operands.push_back(left);
  operands.push_back(right);
  operators.push_back(op);
  initializePrecedence();
}

// Set the prefix for the expression
void Expression::setPrefix(const std::string& p) {
  prefix = p;
}

// Add a Proposition as an operand
void Expression::addOperand(const Proposition& prop) {
  operands.push_back(prop);
}

// Add a LogicalOperator as an operator (legacy API)
void Expression::addOperator(LogicalOperator op) {
  operators.push_back(op);
}

// Token-based API: add operand token
void Expression::addToken(const Proposition& prop) {
  tokens.push_back(Token(prop));
  useTokenStream = true;
}

// Token-based API: add operator token
void Expression::addToken(LogicalOperator op) {
  tokens.push_back(Token(op));
  useTokenStream = true;
}

// Convenience method to add left parenthesis (uses token API)
void Expression::openParen() {
  addToken(LogicalOperator::LPAREN);
}

// Convenience method to add right parenthesis (uses token API)
void Expression::closeParen() {
  addToken(LogicalOperator::RPAREN);
}

// Initialize precedence for logical operators
void Expression::initializePrecedence() {
  precedence[LogicalOperator::NOT] = PRECEDENCE_NOT;
  precedence[LogicalOperator::AND] = PRECEDENCE_AND;
  precedence[LogicalOperator::OR] = PRECEDENCE_OR;
  precedence[LogicalOperator::IMPLIES] = PRECEDENCE_IMPLIES;
  precedence[LogicalOperator::EQUIVALENT] = PRECEDENCE_EQUIVALENT;
}

// Check if an operator is unary
bool Expression::isUnaryOperator(LogicalOperator op) {
  return op == LogicalOperator::NOT;
}

// Check if token is left parenthesis
bool Expression::isLeftParen(LogicalOperator op) {
  return op == LogicalOperator::LPAREN;
}

// Check if token is right parenthesis
bool Expression::isRightParen(LogicalOperator op) {
  return op == LogicalOperator::RPAREN;
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
// algorithm. Handles unary (NOT), binary operators, and parentheses.
//
// The key insight is that we need to track when to consume operands.
// Operands are consumed:
//   1. Before a binary operator (as its left operand)
//   2. After a unary operator (as its single operand)
//   3. Before a closing parenthesis (the last operand in the group)
//   4. At the end of the expression
//
// We track: needsOperand - true when we're expecting an operand next
//
void Expression::convertToPostfix(std::queue<Tripartite>& postfixQueue,
                                  std::queue<LogicalOperator>& opQueue) {
  std::stack<LogicalOperator> opStack;
  size_t operandIndex = 0;
  bool needsOperand = true;  // We start expecting an operand (or unary op or LPAREN)
  
  // Helper to push an operand and apply pending unary operators
  auto pushOperand = [&]() {
    if (operandIndex < operands.size()) {
      postfixQueue.push(operands[operandIndex].getTruthValue());
      operandIndex++;
      
      // After pushing operand, apply any pending unary operators
      // Stop at LPAREN since unary ops inside parens stay with their operand
      while (!opStack.empty() && isUnaryOperator(opStack.top())) {
        opQueue.push(opStack.top());
        opStack.pop();
      }
      needsOperand = false;  // We just consumed an operand
    }
  };
  
  for (size_t i = 0; i < operators.size(); ++i) {
    LogicalOperator currentOp = operators[i];
    
    if (isLeftParen(currentOp)) {
      // Left parenthesis: push onto stack, expect operand inside
      opStack.push(currentOp);
      needsOperand = true;
      
    } else if (isRightParen(currentOp)) {
      // Right parenthesis: consume the last operand in the group if needed
      if (needsOperand) {
        pushOperand();
      }
      
      // Pop operators until we find the matching left parenthesis
      while (!opStack.empty() && !isLeftParen(opStack.top())) {
        if (!isUnaryOperator(opStack.top())) {
          opQueue.push(opStack.top());
        }
        opStack.pop();
      }
      
      // Pop the left parenthesis (don't add to output)
      if (!opStack.empty() && isLeftParen(opStack.top())) {
        opStack.pop();
      }
      
      // After closing paren, apply any pending unary operators from before the group
      while (!opStack.empty() && isUnaryOperator(opStack.top())) {
        opQueue.push(opStack.top());
        opStack.pop();
      }
      
      needsOperand = false;  // The parenthesized expression is a complete operand
      
    } else if (isUnaryOperator(currentOp)) {
      // Unary operator (NOT): push to stack, still need an operand after it
      opStack.push(currentOp);
      // needsOperand stays true - we need the operand this unary applies to
      
    } else {
      // Binary operator: first consume the left operand if we haven't yet
      if (needsOperand) {
        pushOperand();
      }
      
      // Move operators with higher or equal precedence from opStack to opQueue
      // Stop at LPAREN (it acts as a barrier)
      while (!opStack.empty() && 
             !isLeftParen(opStack.top()) && 
             !isUnaryOperator(opStack.top()) &&
             precedence[currentOp] <= precedence[opStack.top()]) {
        opQueue.push(opStack.top());
        opStack.pop();
      }
      
      // Push the current binary operator onto the stack
      opStack.push(currentOp);
      needsOperand = true;  // Now we need the right operand
    }
  }
  
  // Push remaining operands (the final operand)
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
    // Skip any unmatched parentheses (shouldn't happen with valid input)
    if (!isLeftParen(opStack.top()) && !isRightParen(opStack.top())) {
      opQueue.push(opStack.top());
    }
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

// Evaluate token stream using Shunting-Yard algorithm with parentheses support
Tripartite Expression::evaluateTokenStream() {
  std::stack<LogicalOperator> opStack;
  std::stack<Tripartite> valueStack;
  
  // Helper to apply an operator from opStack to values in valueStack
  auto applyOperator = [&]() {
    if (opStack.empty()) return;
    
    LogicalOperator op = opStack.top();
    opStack.pop();
    
    if (isUnaryOperator(op)) {
      if (valueStack.empty()) {
        throw std::runtime_error("Insufficient operands for unary operator");
      }
      Tripartite operand = valueStack.top();
      valueStack.pop();
      valueStack.push(applyUnaryOperator(operand, op));
    } else {
      if (valueStack.size() < 2) {
        throw std::runtime_error("Insufficient operands for binary operator");
      }
      Tripartite right = valueStack.top();
      valueStack.pop();
      Tripartite left = valueStack.top();
      valueStack.pop();
      valueStack.push(applyBinaryOperator(left, right, op));
    }
  };
  
  for (const Token& token : tokens) {
    if (token.isOperand) {
      // Push operand value onto value stack
      valueStack.push(token.prop.getTruthValue());
      
      // After operand, apply any pending unary operators
      while (!opStack.empty() && isUnaryOperator(opStack.top())) {
        applyOperator();
      }
      
    } else if (isLeftParen(token.op)) {
      // Push left paren onto operator stack
      opStack.push(token.op);
      
    } else if (isRightParen(token.op)) {
      // Pop and apply operators until we find matching left paren
      while (!opStack.empty() && !isLeftParen(opStack.top())) {
        applyOperator();
      }
      // Pop the left paren
      if (!opStack.empty() && isLeftParen(opStack.top())) {
        opStack.pop();
      }
      // After closing paren, apply any pending unary operators
      while (!opStack.empty() && isUnaryOperator(opStack.top())) {
        applyOperator();
      }
      
    } else if (isUnaryOperator(token.op)) {
      // Push unary operator, it will be applied after the next operand
      opStack.push(token.op);
      
    } else {
      // Binary operator: apply higher-precedence operators first
      while (!opStack.empty() && 
             !isLeftParen(opStack.top()) && 
             !isUnaryOperator(opStack.top()) &&
             precedence[token.op] <= precedence[opStack.top()]) {
        applyOperator();
      }
      opStack.push(token.op);
    }
  }
  
  // Apply remaining operators
  while (!opStack.empty()) {
    if (!isLeftParen(opStack.top()) && !isRightParen(opStack.top())) {
      applyOperator();
    } else {
      opStack.pop();  // Skip unmatched parens
    }
  }
  
  if (valueStack.size() == 1) {
    return valueStack.top();
  } else if (valueStack.empty()) {
    return Tripartite::UNKNOWN;
  } else {
    throw std::runtime_error("Invalid expression: too many operands");
  }
}

// Main evaluate function
Tripartite Expression::evaluate() {
  if (isEvaluated)
    return evaluatedValue;

  if (useTokenStream) {
    // Use new token-based evaluation
    if (tokens.empty()) {
      evaluatedValue = Tripartite::UNKNOWN;
    } else {
      evaluatedValue = evaluateTokenStream();
    }
  } else {
    // Use legacy evaluation
    if (operands.empty()) {
      evaluatedValue = Tripartite::UNKNOWN;
    } else {
      std::queue<Tripartite> postfixQueue;
      std::queue<LogicalOperator> opQueue;
      convertToPostfix(postfixQueue, opQueue);
      evaluatedValue = evaluatePostfix(postfixQueue, opQueue);
    }
  }

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
  tokens.clear();
  isEvaluated = false;
  evaluatedValue = Tripartite::UNKNOWN;
  useTokenStream = false;
}
