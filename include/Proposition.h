#ifndef PROPOSITION_H
#define PROPOSITION_H

#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

/**
 * Enum class to represent logical operators for propositions.
 * LPAREN and RPAREN are used for grouping in expressions.
 */
enum class LogicalOperator { NONE, AND, OR, NOT, IMPLIES, EQUIVALENT, LPAREN, RPAREN };

/**
 * Enum to represent a three-valued logic: true, false, unknown.
 */
enum class Tripartite { TRUE = 0, FALSE = 1, UNKNOWN = -1 };

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
 * InferenceProvenance tracks how a truth value was derived.
 * Used for explanation generation and debugging inference chains.
 */
struct InferenceProvenance {
  std::string ruleFired;              ///< Name of the inference rule (e.g., "ModusPonens")
  std::vector<std::string> premises;  ///< Names of propositions used as premises
  std::chrono::steady_clock::time_point timestamp;  ///< When the inference was made
  float confidence;                   ///< Confidence score (0.0 to 1.0)

  InferenceProvenance()
      : ruleFired(""),
        premises(),
        timestamp(std::chrono::steady_clock::now()),
        confidence(1.0f) {}

  InferenceProvenance(const std::string& rule,
                      const std::vector<std::string>& prem,
                      float conf = 1.0f)
      : ruleFired(rule),
        premises(prem),
        timestamp(std::chrono::steady_clock::now()),
        confidence(conf) {}
};

/**
 * Conflict records when a truth value is overwritten by a different value.
 * Tracks the old and new values along with their provenance for diagnostics.
 */
struct Conflict {
  Tripartite oldValue;                ///< The value being overwritten
  Tripartite newValue;                ///< The new value being set
  InferenceProvenance oldProvenance;  ///< How the old value was derived
  InferenceProvenance newProvenance;  ///< How the new value was derived
  std::chrono::steady_clock::time_point timestamp;  ///< When the conflict occurred

  Conflict(Tripartite oldVal, Tripartite newVal,
           const InferenceProvenance& oldProv,
           const InferenceProvenance& newProv)
      : oldValue(oldVal),
        newValue(newVal),
        oldProvenance(oldProv),
        newProvenance(newProv),
        timestamp(std::chrono::steady_clock::now()) {}
};

/**
 * Class to represent a logical proposition with a prefix, relation, antecedent,
 * subject, consequent, predicate, and truth value.
 */
class Proposition {
 private:
  std::string prefix;        ///< Symbol or identifier (e.g., "n")
  LogicalOperator relation;  ///< Type of relation (e.g., IMPLIES, NOT)
  std::string antecedent;    ///< The antecedent in a relation (e.g., "big-bang")
  Tripartite antecedentAssertion;  ///< Assertion of the antecedent
  std::string subject;             ///< Context (e.g., "occurred")
  std::string consequent;    ///< Consequent in a relation (e.g., "microwave-radiation")
  Tripartite consequentAssertion;  ///< Assertion of the consequent
  std::string predicate;           ///< State or outcome (e.g., "present")
  Tripartite truth_value;          ///< Truth value of the proposition
  Quantifier proposition_scope;    ///< Scope of the proposition

  // Inference tracking
  std::optional<InferenceProvenance> provenance_;  ///< How current truth value was derived
  std::vector<Conflict> conflicts_;                ///< History of value conflicts

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
  void setTruthValue(Tripartite value, const InferenceProvenance& provenance);
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

  // Inference tracking getters
  const std::optional<InferenceProvenance>& getProvenance() const;
  bool hasProvenance() const;
  const std::vector<Conflict>& getConflicts() const;
  bool hasConflicts() const;
  void clearConflicts();

  explicit operator bool() const;  ///< Conversion to bool

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