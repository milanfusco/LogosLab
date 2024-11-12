#include "Proposition.h"
#include <iostream>

//***  Tripartite Operators ***//

// Equality operator for Tripartite
bool operator==(Tripartite left, Tripartite right) {
  return static_cast<int>(left) == static_cast<int>(right);
}

// Logical AND operator for Tripartite
Tripartite operator&&(Tripartite left, Tripartite right) {
  if (left == Tripartite::FALSE || right == Tripartite::FALSE)
    return Tripartite::FALSE;
  if (left == Tripartite::UNKNOWN || right == Tripartite::UNKNOWN)
    return Tripartite::UNKNOWN;
  return Tripartite::TRUE;
}

// Logical OR operator for Tripartite
Tripartite operator||(Tripartite left, Tripartite right) {
  if (left == Tripartite::TRUE || right == Tripartite::TRUE)
    return Tripartite::TRUE;
  if (left == Tripartite::UNKNOWN || right == Tripartite::UNKNOWN)
    return Tripartite::UNKNOWN;
  return Tripartite::FALSE;
}

// Logical NOT operator for Tripartite
Tripartite operator!(Tripartite value) {
  switch (value) {
    case Tripartite::TRUE:
      return Tripartite::FALSE;
    case Tripartite::FALSE:
      return Tripartite::TRUE;
    case Tripartite::UNKNOWN:
    default:
      return Tripartite::UNKNOWN;
  }
}

// Logical IMPLIES operator for Tripartite
Tripartite implies(Tripartite left, Tripartite right) {
  if (left == Tripartite::FALSE || right == Tripartite::TRUE)
    return Tripartite::TRUE;
  if (left == Tripartite::TRUE && right == Tripartite::FALSE)
    return Tripartite::FALSE;
  return Tripartite::UNKNOWN;
}

//*** Proposition Class ***//

Proposition::Proposition()
    : prefix(""),
      relation(LogicalOperator::NONE),
      antecedent(""),
      antecedentAssertion(Tripartite::UNKNOWN),
      subject(""),
      consequent(""),
      consequentAssertion(Tripartite::UNKNOWN),
      predicate(""),
      truth_value(Tripartite::UNKNOWN),
      proposition_scope(Quantifier::NONE) {}

Proposition::Proposition(Tripartite truth_value) : truth_value(truth_value) {}

Proposition::Proposition(const std::string& prefix, Tripartite truth_value)
    : prefix(prefix),
      relation(LogicalOperator::NONE),
      antecedent(""),
      antecedentAssertion(Tripartite::UNKNOWN),
      subject(""),
      consequent(""),
      consequentAssertion(Tripartite::UNKNOWN),
      predicate(""),
      truth_value(truth_value),
      proposition_scope(Quantifier::NONE) {}

void Proposition::setPrefix(const std::string& prefixToSet) {
  prefix = prefixToSet;
}
void Proposition::setRelation(LogicalOperator relationToSet) {
  relation = relationToSet;
}
void Proposition::setAntecedent(const std::string& antcedentToSet) {
  antecedent = antcedentToSet;
}

void Proposition::setAntecedentAssertion(Tripartite assertionToSet) {
  antecedentAssertion = assertionToSet;
}
void Proposition::setSubject(const std::string& subjectToSet) {
  subject = subjectToSet;
}
void Proposition::setConsequent(const std::string& consequentToSet) {
  consequent = consequentToSet;
}
void Proposition::setConsequentAssertion(Tripartite assertionToSet) {
  consequentAssertion = assertionToSet;
}
void Proposition::setPredicate(const std::string& predicateToSet) {
  predicate = predicateToSet;
}
void Proposition::setTruthValue(Tripartite valueToSet) {
  truth_value = valueToSet;
}
void Proposition::setPropositionScope(Quantifier scopeToSet) {
  proposition_scope = scopeToSet;
}

std::string Proposition::getPrefix() const {
  return prefix;
}
LogicalOperator Proposition::getRelation() const {
  return relation;
}
std::string Proposition::getAntecedent() const {
  return antecedent;
}
Tripartite Proposition::getAntecedentAssertion() const {
  return antecedentAssertion;
}
std::string Proposition::getSubject() const {
  return subject;
}
std::string Proposition::getConsequent() const {
  return consequent;
}
Tripartite Proposition::getConsequentAssertion() const {
  return consequentAssertion;
}
std::string Proposition::getPredicate() const {
  return predicate;
}
Tripartite Proposition::getTruthValue() const {
  return truth_value;
}
Quantifier Proposition::getPropositionScope() const {
  return proposition_scope;
}

// Operator Overloads //

Proposition::operator bool() const {
  return truth_value == Tripartite::TRUE;
}

bool Proposition::operator==(const Proposition& other) const {
  return truth_value == other.truth_value;
}

bool Proposition::operator!=(const Proposition& other) const {
  return truth_value != other.truth_value;
}

Proposition Proposition::operator&&(const Proposition& other) const {
  Proposition result;
  result.setTruthValue(truth_value &&
                       other.truth_value);  // Uses Tripartite AND
  return result;
}

// OR operator
Proposition Proposition::operator||(const Proposition& other) const {
  Proposition result;
  result.setTruthValue(truth_value || other.truth_value);  // Uses Tripartite OR
  return result;
}

// NOT operator
Proposition Proposition::operator!() const {
  Proposition result;
  result.setTruthValue(!truth_value);  // Uses Tripartite NOT
  return result;
}

Tripartite Proposition::implies(const Proposition& other) const {
  return ::implies(truth_value, other.truth_value);
}

Proposition& Proposition::operator=(const Proposition& other) {
  if (this != &other) {
    prefix = other.prefix;
    relation = other.relation;
    antecedent = other.antecedent;
    antecedentAssertion = other.antecedentAssertion;
    subject = other.subject;
    consequent = other.consequent;
    consequentAssertion = other.consequentAssertion;
    predicate = other.predicate;
    truth_value = other.truth_value;
    proposition_scope = other.proposition_scope;
  }
  return *this;
}

std::ostream& operator<<(std::ostream& out, const Proposition& prop) {
  out << "Proposition: [" << prop.prefix << "] Truth: "
      << (prop.truth_value == Tripartite::TRUE    ? "True"
          : prop.truth_value == Tripartite::FALSE ? "False"
                                                  : "Unknown")
      << ", Relation: " << static_cast<int>(prop.relation);
  return out;
}

// Display for debugging
void Proposition::display() const {
  std::cout << *this << std::endl;
}