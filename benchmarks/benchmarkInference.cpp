/**
 * Benchmarks for LogosLab inference engine using Google Benchmark.
 * 
 * Measures performance of:
 * - Individual inference rules (Modus Ponens, Modus Tollens, etc.)
 * - Expression evaluation
 * - Full deduction cycles with varying knowledge base sizes
 * - Lexer tokenization
 */

#include <benchmark/benchmark.h>
#include <string>
#include <vector>
#include <sstream>
#include "Ratiocinator.h"
#include "InferenceEngine.h"
#include "Expression.h"
#include "Lexer.h"

// ============================================================
// HELPER FUNCTIONS
// ============================================================

/**
 * Create a proposition with given truth value.
 */
Proposition makeProp(const std::string& name, Tripartite value, 
                     LogicalOperator rel = LogicalOperator::NONE,
                     const std::string& ante = "", const std::string& cons = "") {
    Proposition p;
    p.setPrefix(name);
    p.setTruthValue(value);
    if (rel != LogicalOperator::NONE) {
        p.setRelation(rel);
        p.setAntecedent(ante);
        p.setConsequent(cons);
    }
    return p;
}

/**
 * Create an implication proposition: antecedent -> consequent
 */
Proposition makeImplication(const std::string& name, 
                            const std::string& antecedent, 
                            const std::string& consequent) {
    Proposition p;
    p.setPrefix(name);
    p.setRelation(LogicalOperator::IMPLIES);
    p.setAntecedent(antecedent);
    p.setConsequent(consequent);
    return p;
}

/**
 * Create a disjunction proposition: left OR right
 */
Proposition makeDisjunction(const std::string& name,
                            const std::string& left,
                            const std::string& right) {
    Proposition p;
    p.setPrefix(name);
    p.setRelation(LogicalOperator::OR);
    p.setAntecedent(left);
    p.setConsequent(right);
    return p;
}

// ============================================================
// MODUS PONENS BENCHMARKS
// ============================================================

/**
 * Benchmark: Single Modus Ponens inference
 * Setup: P=TRUE, P->Q
 * Measure: Time to derive Q=TRUE
 */
static void BM_ModusPonens_Single(benchmark::State& state) {
    for (auto _ : state) {
        std::unordered_map<std::string, Proposition> props;
        std::vector<Expression> exprs;
        
        props["P"] = makeProp("P", Tripartite::TRUE);
        props["Q"] = makeImplication("imp_PQ", "P", "Q");
        
        InferenceEngine engine;
        engine.deduceAll(props, exprs);
        
        benchmark::DoNotOptimize(props["Q"].getTruthValue());
    }
}
BENCHMARK(BM_ModusPonens_Single);

/**
 * Benchmark: Chain of Modus Ponens inferences
 * Setup: A=TRUE, A->B, B->C, ..., Y->Z
 * Measure: Time to derive all consequents
 */
static void BM_ModusPonens_Chain(benchmark::State& state) {
    const int chainLength = state.range(0);
    
    for (auto _ : state) {
        std::unordered_map<std::string, Proposition> props;
        std::vector<Expression> exprs;
        
        // First proposition is TRUE
        props["P0"] = makeProp("P0", Tripartite::TRUE);
        
        // Create chain: P0->P1, P1->P2, ..., P(n-1)->Pn
        for (int i = 1; i <= chainLength; ++i) {
            std::string prev = "P" + std::to_string(i - 1);
            std::string curr = "P" + std::to_string(i);
            props[curr] = makeImplication("imp_" + prev + "_" + curr, prev, curr);
        }
        
        InferenceEngine engine;
        engine.deduceAll(props, exprs);
        
        std::string last = "P" + std::to_string(chainLength);
        benchmark::DoNotOptimize(props[last].getTruthValue());
    }
    
    state.SetComplexityN(chainLength);
}
BENCHMARK(BM_ModusPonens_Chain)->Range(2, 64)->Complexity();

// ============================================================
// MODUS TOLLENS BENCHMARKS
// ============================================================

/**
 * Benchmark: Single Modus Tollens inference
 * Setup: Q=FALSE, P->Q
 * Measure: Time to derive P=FALSE
 */
static void BM_ModusTollens_Single(benchmark::State& state) {
    for (auto _ : state) {
        std::unordered_map<std::string, Proposition> props;
        std::vector<Expression> exprs;
        
        props["P"] = makeImplication("imp_PQ", "P", "Q");
        props["Q"] = makeProp("Q", Tripartite::FALSE);
        
        InferenceEngine engine;
        engine.deduceAll(props, exprs);
        
        benchmark::DoNotOptimize(props["P"].getTruthValue());
    }
}
BENCHMARK(BM_ModusTollens_Single);

/**
 * Benchmark: Chain of Modus Tollens inferences (backward)
 * Setup: Pn=FALSE, P0->P1, P1->P2, ..., P(n-1)->Pn
 * Measure: Time to derive P0=FALSE via backward chaining
 */
static void BM_ModusTollens_Chain(benchmark::State& state) {
    const int chainLength = state.range(0);
    
    for (auto _ : state) {
        std::unordered_map<std::string, Proposition> props;
        std::vector<Expression> exprs;
        
        // Create chain: P0->P1, P1->P2, ..., P(n-1)->Pn
        for (int i = 1; i <= chainLength; ++i) {
            std::string prev = "P" + std::to_string(i - 1);
            std::string curr = "P" + std::to_string(i);
            props[curr] = makeImplication("imp_" + prev + "_" + curr, prev, curr);
        }
        
        // Last proposition is FALSE
        std::string last = "P" + std::to_string(chainLength);
        props[last].setTruthValue(Tripartite::FALSE);
        
        InferenceEngine engine;
        engine.deduceAll(props, exprs);
        
        benchmark::DoNotOptimize(props["P0"].getTruthValue());
    }
    
    state.SetComplexityN(chainLength);
}
BENCHMARK(BM_ModusTollens_Chain)->Range(2, 64)->Complexity();

// ============================================================
// HYPOTHETICAL SYLLOGISM BENCHMARKS
// ============================================================

/**
 * Benchmark: Hypothetical Syllogism
 * Setup: P=TRUE, P->Q, Q->R
 * Measure: Time to derive R=TRUE via HS
 */
static void BM_HypotheticalSyllogism(benchmark::State& state) {
    for (auto _ : state) {
        std::unordered_map<std::string, Proposition> props;
        std::vector<Expression> exprs;
        
        props["P"] = makeProp("P", Tripartite::TRUE);
        props["Q"] = makeImplication("imp_PQ", "P", "Q");
        props["R"] = makeImplication("imp_QR", "Q", "R");
        
        InferenceEngine engine;
        engine.deduceAll(props, exprs);
        
        benchmark::DoNotOptimize(props["R"].getTruthValue());
    }
}
BENCHMARK(BM_HypotheticalSyllogism);

// ============================================================
// DISJUNCTIVE SYLLOGISM BENCHMARKS
// ============================================================

/**
 * Benchmark: Disjunctive Syllogism
 * Setup: P=FALSE, P OR Q
 * Measure: Time to derive Q=TRUE
 */
static void BM_DisjunctiveSyllogism(benchmark::State& state) {
    for (auto _ : state) {
        std::unordered_map<std::string, Proposition> props;
        std::vector<Expression> exprs;
        
        props["P"] = makeProp("P", Tripartite::FALSE);
        props["disjPQ"] = makeDisjunction("disjPQ", "P", "Q");
        props["Q"] = makeProp("Q", Tripartite::UNKNOWN);
        
        InferenceEngine engine;
        engine.deduceAll(props, exprs);
        
        benchmark::DoNotOptimize(props["Q"].getTruthValue());
    }
}
BENCHMARK(BM_DisjunctiveSyllogism);

// ============================================================
// EXPRESSION EVALUATION BENCHMARKS
// ============================================================

/**
 * Benchmark: Simple binary expression evaluation
 * Measure: A && B evaluation time
 */
static void BM_Expression_SimpleBinary(benchmark::State& state) {
    Proposition a = makeProp("A", Tripartite::TRUE);
    Proposition b = makeProp("B", Tripartite::FALSE);
    
    for (auto _ : state) {
        Expression expr;
        expr.addToken(a);
        expr.addToken(LogicalOperator::AND);
        expr.addToken(b);
        
        benchmark::DoNotOptimize(expr.evaluate());
    }
}
BENCHMARK(BM_Expression_SimpleBinary);

/**
 * Benchmark: Complex nested expression
 * Measure: (A && B) || (C && D) evaluation time
 */
static void BM_Expression_Nested(benchmark::State& state) {
    Proposition a = makeProp("A", Tripartite::TRUE);
    Proposition b = makeProp("B", Tripartite::TRUE);
    Proposition c = makeProp("C", Tripartite::FALSE);
    Proposition d = makeProp("D", Tripartite::TRUE);
    
    for (auto _ : state) {
        Expression expr;
        expr.openParen();
        expr.addToken(a);
        expr.addToken(LogicalOperator::AND);
        expr.addToken(b);
        expr.closeParen();
        expr.addToken(LogicalOperator::OR);
        expr.openParen();
        expr.addToken(c);
        expr.addToken(LogicalOperator::AND);
        expr.addToken(d);
        expr.closeParen();
        
        benchmark::DoNotOptimize(expr.evaluate());
    }
}
BENCHMARK(BM_Expression_Nested);

/**
 * Benchmark: Expression with N operands
 * Measure: A && B && C && ... (N operands)
 */
static void BM_Expression_NOperands(benchmark::State& state) {
    const int numOperands = state.range(0);
    
    // Pre-create propositions
    std::vector<Proposition> props;
    for (int i = 0; i < numOperands; ++i) {
        props.push_back(makeProp("P" + std::to_string(i), Tripartite::TRUE));
    }
    
    for (auto _ : state) {
        Expression expr;
        for (int i = 0; i < numOperands; ++i) {
            if (i > 0) {
                expr.addToken(LogicalOperator::AND);
            }
            expr.addToken(props[i]);
        }
        
        benchmark::DoNotOptimize(expr.evaluate());
    }
    
    state.SetComplexityN(numOperands);
}
BENCHMARK(BM_Expression_NOperands)->Range(2, 128)->Complexity();

// ============================================================
// LEXER BENCHMARKS
// ============================================================

/**
 * Benchmark: Lexer tokenization of simple expression
 */
static void BM_Lexer_Simple(benchmark::State& state) {
    Lexer lexer;
    std::string input = "A && B || C";
    
    for (auto _ : state) {
        auto tokens = lexer.tokenize(input);
        benchmark::DoNotOptimize(tokens);
    }
}
BENCHMARK(BM_Lexer_Simple);

/**
 * Benchmark: Lexer tokenization of complex expression
 */
static void BM_Lexer_Complex(benchmark::State& state) {
    Lexer lexer;
    std::string input = "(A && B) || (C -> D) <-> (!E && F)";
    
    for (auto _ : state) {
        auto tokens = lexer.tokenize(input);
        benchmark::DoNotOptimize(tokens);
    }
}
BENCHMARK(BM_Lexer_Complex);

/**
 * Benchmark: Lexer with varying input size
 */
static void BM_Lexer_Size(benchmark::State& state) {
    Lexer lexer;
    const int numTerms = state.range(0);
    
    // Build expression: A0 && A1 && A2 && ...
    std::ostringstream oss;
    for (int i = 0; i < numTerms; ++i) {
        if (i > 0) oss << " && ";
        oss << "A" << i;
    }
    std::string input = oss.str();
    
    for (auto _ : state) {
        auto tokens = lexer.tokenize(input);
        benchmark::DoNotOptimize(tokens);
    }
    
    state.SetComplexityN(numTerms);
}
BENCHMARK(BM_Lexer_Size)->Range(2, 256)->Complexity();

// ============================================================
// FULL DEDUCTION CYCLE BENCHMARKS
// ============================================================

/**
 * Benchmark: Full deduction with N propositions
 * Measure: Time for complete deduction cycle
 */
static void BM_DeduceAll_Size(benchmark::State& state) {
    const int numProps = state.range(0);
    
    for (auto _ : state) {
        std::unordered_map<std::string, Proposition> props;
        std::vector<Expression> exprs;
        
        // Create base facts
        props["P0"] = makeProp("P0", Tripartite::TRUE);
        props["Q0"] = makeProp("Q0", Tripartite::FALSE);
        
        // Create implications: Pi -> Pi+1 for half, Qi -> Qi+1 for other half
        for (int i = 1; i < numProps / 2; ++i) {
            std::string prevP = "P" + std::to_string(i - 1);
            std::string currP = "P" + std::to_string(i);
            props[currP] = makeImplication("imp_" + currP, prevP, currP);
            
            std::string prevQ = "Q" + std::to_string(i - 1);
            std::string currQ = "Q" + std::to_string(i);
            props[currQ] = makeImplication("imp_" + currQ, prevQ, currQ);
        }
        
        InferenceEngine engine;
        engine.deduceAll(props, exprs);
        
        benchmark::DoNotOptimize(props.size());
    }
    
    state.SetComplexityN(numProps);
}
BENCHMARK(BM_DeduceAll_Size)->Range(4, 256)->Complexity();

/**
 * Benchmark: Ratiocinator full workflow
 * Measure: Load + deduce + format results
 */
static void BM_Ratiocinator_Workflow(benchmark::State& state) {
    const int numProps = state.range(0);
    
    for (auto _ : state) {
        Ratiocinator engine;
        
        // Manually set up propositions (simulating file load)
        engine.setPropositionTruthValue("P0", Tripartite::TRUE);
        
        for (int i = 1; i < numProps; ++i) {
            std::string prev = "P" + std::to_string(i - 1);
            std::string curr = "P" + std::to_string(i);
            Proposition imp = makeImplication("imp_" + curr, prev, curr);
            engine.setProposition(curr, imp);
        }
        
        engine.deduce();
        
        std::string results = engine.formatResults();
        benchmark::DoNotOptimize(results);
    }
    
    state.SetComplexityN(numProps);
}
BENCHMARK(BM_Ratiocinator_Workflow)->Range(4, 128)->Complexity();

/**
 * Benchmark: Inference tracing overhead
 * Measure: Time to trace inference chain
 */
static void BM_TraceInference(benchmark::State& state) {
    const int chainLength = state.range(0);
    
    // Set up once outside the benchmark loop
    Ratiocinator engine;
    engine.setPropositionTruthValue("P0", Tripartite::TRUE);
    
    for (int i = 1; i <= chainLength; ++i) {
        std::string prev = "P" + std::to_string(i - 1);
        std::string curr = "P" + std::to_string(i);
        Proposition imp = makeImplication("imp_" + curr, prev, curr);
        engine.setProposition(curr, imp);
    }
    
    engine.deduce();
    
    std::string target = "P" + std::to_string(chainLength);
    
    for (auto _ : state) {
        auto trace = engine.traceInference(target);
        benchmark::DoNotOptimize(trace);
    }
    
    state.SetComplexityN(chainLength);
}
BENCHMARK(BM_TraceInference)->Range(2, 64)->Complexity();

/**
 * Benchmark: Result filtering overhead
 */
static void BM_ResultFiltering(benchmark::State& state) {
    const int numProps = state.range(0);
    
    // Set up once outside the benchmark loop
    Ratiocinator engine;
    
    for (int i = 0; i < numProps; ++i) {
        std::string name = "P" + std::to_string(i);
        Tripartite value = (i % 3 == 0) ? Tripartite::TRUE : 
                          (i % 3 == 1) ? Tripartite::FALSE : 
                                         Tripartite::UNKNOWN;
        engine.setPropositionTruthValue(name, value);
    }
    
    ResultFilter filter = ResultFilter::trueOnly().withSort(ResultSortOrder::ALPHABETICAL);
    
    for (auto _ : state) {
        std::string results = engine.formatResults(filter);
        benchmark::DoNotOptimize(results);
    }
    
    state.SetComplexityN(numProps);
}
BENCHMARK(BM_ResultFiltering)->Range(10, 1000)->Complexity();

// ============================================================
// MAIN
// ============================================================

BENCHMARK_MAIN();

