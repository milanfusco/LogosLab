#ifndef PROPOSITION_H
#define PROPOSITION_H

#include <iostream>
#include <string>

/**
 * Enum class to represent logical operators for propositions.
 */
enum class LogicalOperator { NONE, AND, OR, NOT, IMPLIES, EQUIVALENT };

/**
 * Enum to represent a three-valued logic: true, false, unknown.
 */
enum class Tripartite { TRUE = 0, FALSE = 1, UNKNOWN = -1 };

/**
 * @overload operator==
 * @brief Equality operator for Tripartite values.
 * @param left The left Tripartite value
 * @param right The right Tripartite value
 * @return True if the values are equal, false otherwise
 */
bool operator==(Tripartite left, Tripartite right);

/**
 * @overload operator&&
 * @brief Logical AND operator for Tripartite values.
 * @param left The left Tripartite value
 * @param right The right Tripartite value
 * @return True if both values are true, false otherwise
 */
Tripartite operator&&(Tripartite left, Tripartite right);

/**
 * @overload operator||
 * @brief Logical OR operator for Tripartite values.
 * @param left The left Tripartite value
 * @param right The right Tripartite value
 * @return True if either value is true, false otherwise
 */
Tripartite operator||(Tripartite left, Tripartite right);

/**
 * @overload operator!
 * @brief Logical NOT operator for Tripartite values.
 * @param value The Tripartite value to negate
 * @return The negated Tripartite value
 */
Tripartite operator!(Tripartite value);

/**
 * @overload implies
 * @brief Logical IMPLIES operator for Tripartite values.
 * @param left The left Tripartite value
 * @param right The right Tripartite value
 * @return True if left implies right, false otherwise
 */
Tripartite implies(Tripartite left, Tripartite right);

/**
 * Enum to represent quantifiers for propositions.
 */
enum class Quantifier {
  UNIVERSAL_AFFIRMATIVE,
  UNIVERSAL_NEGATIVE,
  PARTICULAR_AFFIRMATIVE,
  PARTICULAR_NEGATIVE,
  NONE
};

/**
 * Class to represent a logical proposition with a prefix, relation, antecedent,
 * subject, consequent, predicate, and truth value.
 */
class Proposition {
 private:
  std::string prefix;        ///> Symbol or identifier (e.g., "n")
  LogicalOperator relation;  ///> Type of relation (e.g., IMPLIES, NOT)
  std::string antecedent;  ///> The antecedent in a relation (e.g., "big-bang")
  Tripartite antecedentAssertion;  ///> Assertion of the antecedent
  std::string subject;             ///> Context (e.g., "occurred")
  std::string
      consequent;  ///> Consequent in a relation (e.g., "microwave-radiation")
  Tripartite consequentAssertion;  ///> Assertion of the consequent
  std::string predicate;           ///> State or outcome (e.g., "present")
  Tripartite truth_value;          ///> Truth value of the proposition
  Quantifier proposition_scope;    ///> Scope of the proposition

 public:
  ///> Default constructor
  Proposition();

  ///> Simplified constructor for intermediate results
  Proposition(Tripartite truth_value);

  ///> Simplified constructor for propositions with a prefix and truth value
  Proposition(const std::string& prefix, Tripartite truth_value);

  // Setters for updates
  void setPrefix(const std::string& p);
  void setRelation(LogicalOperator rel);
  void setAntecedent(const std::string& ant);
  void setAntecedentAssertion(Tripartite assert);
  void setSubject(const std::string& subj);
  void setConsequent(const std::string& cons);
  void setConsequentAssertion(Tripartite assert);
  void setPredicate(const std::string& pred);
  void setTruthValue(Tripartite value);
  void setPropositionScope(Quantifier scope);

  // Getters
  std::string getPrefix() const;
  LogicalOperator getRelation() const;
  std::string getAntecedent() const;
  Tripartite getAntecedentAssertion() const;
  std::string getSubject() const;
  std::string getConsequent() const;
  Tripartite getConsequentAssertion() const;
  std::string getPredicate() const;
  Tripartite getTruthValue() const;
  Quantifier getPropositionScope() const;

  explicit operator bool() const;  ///> Conversion to bool

  bool operator==(
      const Proposition& other) const;  ///> Logical EQUIVALENT operator

  bool operator!=(
      const Proposition& other) const;  ///> Logical NOT EQUAL operator

  Proposition operator&&(
      const Proposition& other) const;  ///> Logical AND operator

  Proposition operator||(
      const Proposition& other) const;  ///> Logical OR operator

  Proposition operator!() const;  ///> Logical NOT operator

  Proposition implies(
      const Proposition& other) const;  ///> Logical IMPLIES operator

  Proposition& operator=(const Proposition& other);  ///> Assignment operator

  friend std::ostream& operator<<(
      std::ostream& out,
      const Proposition& prop);  ///> Output operator

  void display() const;  ///> Display for debugging
};

#endif  // PROPOSITION_H