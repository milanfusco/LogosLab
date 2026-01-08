# LogosLab

**LogosLab** is a powerful logical reasoning framework designed for automated deduction, symbolic logic evaluation, and knowledge base inference. It implements a complete reasoning engine with three-valued logic (TRUE, FALSE, UNKNOWN), multiple inference rules, and an extensible architecture for building intelligent systems.

## Features

### Core Capabilities
- **Tripartite Logic**: Support for TRUE, FALSE, and UNKNOWN truth values with proper semantics
- **Advanced Inference Engine**: 
  - Modus Ponens: P -> Q, P therefore Q
  - Modus Tollens: P -> Q, !Q therefore !P
  - Hypothetical Syllogism: P -> Q, Q -> R therefore P -> R
  - Disjunctive Syllogism: P || Q, !P therefore Q
  - Resolution: P || Q, !P || R therefore Q || R
- **Expression Evaluation**: Full support for complex logical expressions with parentheses
- **Inference Tracing**: Track how conclusions were derived with complete provenance information
- **Extensible Parser**: Plugin-style relation handlers for custom knowledge representation

### Logical Operators
- **AND** (`&&`, `and`): Logical conjunction
- **OR** (`||`, `or`): Logical disjunction
- **NOT** (`!`, `~`, `not`): Logical negation
- **IMPLIES** (`->`, `implies`): Material implication
- **EQUIVALENT** (`<->`, `==`, `iff`): Biconditional

### Command-Line Interface
Advanced filtering and sorting options for query results:
- Filter by truth value (true/false/unknown)
- Filter by derivation (axioms vs derived propositions)
- Pattern matching (prefix, contains)
- Sort by name, truth value, or derivation
- Limit results and show inference traces

## Project Structure

### Core Components

#### `Expression` (`Expression.h/cpp`)
Evaluates complex logical expressions using the Shunting-Yard algorithm:
- Handles operator precedence and associativity
- Supports parentheses for grouping
- Token-based expression building
- Efficient postfix evaluation

#### `Proposition` (`Proposition.h/cpp`)
Models logical propositions with rich metadata:
- Three-valued logic support
- Inference provenance tracking
- Conflict detection (when values are overwritten)
- Quantifier support (universal/particular, affirmative/negative)

#### `Lexer` (`Lexer.h/cpp`)
Tokenizes input with detailed error reporting:
- Multi-character operator support
- Identifier parsing with hyphens (e.g., `big-bang`)
- Line/column tracking for error messages
- Configurable keyword handling

#### `Parser` (`Parser.h/cpp`)
Parses knowledge base files with extensible handlers:
- Built-in relations: `implies`, `some`, `not`, `discovered`
- Custom relation registration via `RelationHandler` API
- Expression parsing from strings
- Supports both assumptions and facts files

#### `InferenceEngine` (`InferenceEngine.h/cpp`)
Applies inference rules until fixed-point:
- Forward and backward chaining
- Handles complex inference chains
- Safe proposition lookup (no crashes on missing data)
- Optimized iteration to prevent redundant work

#### `Ratiocinator` (`Ratiocinator.h/cpp`)
High-level facade for the reasoning system:
- Load assumptions and facts from files
- Run inference to deduce new knowledge
- Query and filter results
- Format and export reasoning traces

### Tests
Comprehensive test suite in `tests/`:
- `testExpression.cpp`: Expression evaluation and operator precedence
- `testExpressionProperties.cpp`: Property-based expression tests
- `testLexer.cpp`: Tokenization and error handling
- `testProposition.cpp`: Proposition logic and operations
- `testRatiocinator.cpp`: End-to-end reasoning scenarios

### Build System
CMake-based build with multiple presets:
- `dev-debug`: Development build with debug symbols
- `dev-release`: Optimized development build
- `asan`: AddressSanitizer (memory errors)
- `tsan`: ThreadSanitizer (race conditions)
- `ubsan`: UndefinedBehaviorSanitizer
- `msan`: MemorySanitizer (uninitialized reads)
- `clang-tidy`: Static analysis
- `cppcheck`: Additional static analysis
- `ci-release`: CI/CD release build
- `bench-release`: Performance benchmarking

## Getting Started

### Prerequisites
- **C++ Compiler**: C++17 or later (GCC 8+, Clang 7+, MSVC 2019+)
- **CMake**: Version 3.15 or later

### Building the Project

#### Quick Start
```bash
git clone https://github.com/milanfusco/LogosLab.git
cd LogosLab
cmake --preset dev-debug
cmake --build --preset dev-debug
```

#### Using Build Presets
```bash
# List available presets
cmake --list-presets

# Configure with a specific preset
cmake --preset dev-release

# Build
cmake --build --preset dev-release

# Run tests
ctest --preset dev-release
```

#### Traditional CMake Build
```bash
mkdir build
cd build
cmake ..
make
```

### Running the Application

#### Basic Usage
```bash
# Run with assumptions and facts files
./build/dev-debug/main assumptions_file.txt facts_file.txt

# Or use the demo files
./build/dev-debug/main demo_assumptions.txt demo_facts.txt
```

#### Command-Line Options
```bash
# Show only TRUE propositions with traces
./main --true-only --traces assumptions.txt facts.txt

# Filter by prefix and limit results
./main --prefix=user_ --limit=10 assumptions.txt facts.txt

# Show derived propositions sorted alphabetically
./main --derived-only --sort=alpha assumptions.txt facts.txt

# Multiple filters
./main --known-only --prefix=big --verbose assumptions.txt facts.txt
```

Available options:
- `--traces`: Include inference traces in output
- `--true-only`, `--false-only`, `--unknown-only`: Filter by truth value
- `--known-only`: Show only TRUE and FALSE (exclude UNKNOWN)
- `--derived-only`: Show only inferred propositions
- `--axioms-only`: Show only direct assertions
- `--prefix=PREFIX`: Filter by name prefix
- `--contains=STR`: Filter by substring in name
- `--limit=N`: Limit number of results
- `--sort=ORDER`: Sort by `alpha`, `alpha-desc`, `truth`, or `derivation`
- `--verbose`: Print results to console
- `--help`: Show help message

### File Formats

#### Assumptions File
Define logical relations and implications:
```
n, implies(big-bang, occurred, microwave-radiation, present)
s, some(gravity, exists)
r, not(perpetual-motion)
d, discovered(cosmic-background, radiation)
```

#### Facts File
Assert truth values and logical expressions:
```
# Simple assertions
big-bang
!perpetual-motion

# Compound expressions with assignment
t = p && n
result = (a || b) && !c

# Expressions without assignment
p && q
!r || s
```

### Testing

#### Run All Tests
```bash
ctest --preset dev-debug
```

#### Run Individual Test Suites
```bash
./build/dev-debug/testExpression
./build/dev-debug/testProposition
./build/dev-debug/testRatiocinator
./build/dev-debug/testLexer
```

#### Run with Sanitizers
```bash
# Memory error detection
cmake --preset asan
cmake --build --preset asan
ctest --preset asan

# Thread safety
cmake --preset tsan
cmake --build --preset tsan
ctest --preset tsan
```

### Benchmarking
```bash
cmake --preset bench-release
cmake --build --preset bench-release
./build/bench-release/benchmarkInference
```

Results are saved to `benchmarks/results/` with system information.

## Usage Examples

### Example 1: Simple Implication
**assumptions.txt:**
```
i1, implies(rain, falls, ground, wet)
```

**facts.txt:**
```
rain
```

**Result:** The system infers `ground = TRUE` via Modus Ponens.

### Example 2: Chained Reasoning
**assumptions.txt:**
```
i1, implies(A, true, B, true)
i2, implies(B, true, C, true)
```

**facts.txt:**
```
A
```

**Result:** The system infers `B = TRUE` (via Modus Ponens) and `C = TRUE` (via Hypothetical Syllogism).

### Example 3: Disjunctive Syllogism
**facts.txt:**
```
result = p || q
!p
```

**Result:** The system infers `q = TRUE` via Disjunctive Syllogism.

## API Usage (Programmatic)

```cpp
#include "Ratiocinator.h"

int main() {
    Ratiocinator engine;
    
    // Load knowledge base
    engine.loadAssumptions("assumptions.txt");
    engine.loadFacts("facts.txt");
    
    // Run inference
    engine.deduce();
    
    // Query results with filtering
    ResultFilter filter = ResultFilter::trueOnly()
        .withPrefix("user_")
        .withLimit(10)
        .withTraces(true);
    
    std::string results = engine.formatResults(filter);
    std::cout << results;
    
    // Trace specific proposition
    if (engine.hasInferenceProvenance("conclusion")) {
        std::cout << engine.formatTrace("conclusion");
    }
    
    return 0;
}
```

## Architecture

### Design Principles
- **Separation of Concerns**: Lexer, Parser, and Inference Engine are independent
- **Extensibility**: Plugin-style relation handlers and custom filters
- **Safety**: Null-safe lookups, no iterator invalidation, comprehensive error handling
- **Performance**: Fixed-point iteration, lazy evaluation, optimized data structures
- **Observability**: Complete provenance tracking and inference traces

### Inference Process
1. **Parse**: Load assumptions (structure) and facts (truth values)
2. **Evaluate**: Calculate truth values of compound expressions
3. **Infer**: Apply inference rules until no more deductions can be made
4. **Report**: Format results with optional filtering and tracing

## Contributing
Contributions are welcome! Please ensure:
- Code follows existing style conventions
- All tests pass (`ctest`)
- New features include test coverage
- Documentation is updated (comments + README)

## License
[MIT License](LICENSE)
