# LogosLab

**LogosLab** is a logical reasoning framework designed to model, manipulate, and evaluate logical propositions and expressions programmatically. It provides tools for building and testing logical statements, facilitating tasks in symbolic logic, automated reasoning, and computational philosophy.

## Features
- **Expression Handling**: Define, evaluate, and manipulate logical expressions programmatically.
- **Proposition Management**: Model logical propositions and analyze their validity or equivalence.
- **Automated Reasoning**: Use the `Ratiocinator` component to perform logical deductions and inferences.
- **Testing Framework**: Includes a comprehensive suite for validating core logic and functionalities.
- 
## Project Structure

### Core Components
- **Expression**:
   - `Expression.h/cpp`: Defines and implements the structure and behavior of logical expressions.
 - **Proposition**:
   - `Proposition.h/cpp`: Models logical propositions and provides utilities for their analysis.
- **Ratiocinator**:
   - `Ratiocinator.h/cpp`: Core reasoning engine for logical inference and deduction.


### Tests
- `tests/`: Contains unit tests for each component:
   - `testExpression.cpp`
   - `testProposition.cpp`
   - `testRatiocinator.cpp`

### Configuration and Resources
- `CMakeLists.txt`: Build system configuration.
  - `assumptions_file.txt`: A file defining assumptions for logical evaluation.
  - `facts_file.txt`: A file containing predefined logical facts for reasoning tasks.

## Getting Started

### Prerequisites
- **C++ Compiler**: Requires a modern C++ compiler (C++17 or later).
- **CMake**: For building the project.

### Building the Project
1. Clone the repository: ```git clone https://github.com/milanfusco/LogosLab.git```
2. Navigate to the project directory: ```cd LogosLab```
3. Build using CMake:
   - ```mkdir build```
   - ```cd build```
   - ```cmake ..```
   - ``` make```

### Running the Application
Run the main program: ```./LogosLab```

### Testing
Run the test cases:
- ```./tests/testExpression```
- ```./tests/testProposition```
- ```./tests/testRatiocinator```

## License
[MIT License](LICENSE)

## Acknowledgments
- Inspired by classic approaches to symbolic logic and reasoning.
- Tools and frameworks used: CMake, modern C++ development practices.
