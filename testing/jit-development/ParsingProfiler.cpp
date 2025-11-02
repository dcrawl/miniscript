//
// ParsingProfiler.cpp
// Phase 2.2 - Isolate parsing performance bottlenecks
//
// This benchmark separates lexing, parsing, and execution performance to
// identify the primary bottleneck revealed by Phase 2.1 findings.
//

#include <chrono>
#include <iostream>
#include "MiniscriptLexer.h"
#include "MiniscriptParser.h"
#include "MiniscriptInterpreter.h"

using namespace MiniScript;
using namespace std::chrono;

const String TEST_CODE = "sum = 0\nfor i in range(10000)\n  sum = sum + i * 2\nend for\nprint sum";

class ParsingProfiler {
public:
    static void RunBenchmark() {
        std::cout << "\n=== Phase 2.2 Parsing Performance Analysis ===" << std::endl;
        
        // Test 1: Lexing Performance
        double lexingTime = BenchmarkLexing();
        std::cout << "1. Lexing Time: " << lexingTime << " ms" << std::endl;
        
        // Test 2: Parsing Performance (includes lexing)
        double parsingTime = BenchmarkParsing();
        std::cout << "2. Total Parsing Time: " << parsingTime << " ms" << std::endl;
        
        // Test 3: Execution Performance (after compilation)
        double executionTime = BenchmarkExecution();
        std::cout << "3. Execution Time (post-compile): " << executionTime << " ms" << std::endl;
        
        // Test 4: Full Pipeline (from our Phase 2.1 findings)
        double fullTime = BenchmarkFullPipeline();
        std::cout << "4. Full Pipeline Time: " << fullTime << " ms" << std::endl;
        
        std::cout << "\n=== Performance Breakdown ===" << std::endl;
        std::cout << "Pure Lexing: " << lexingTime << " ms (" 
                  << (lexingTime/fullTime*100) << "% of total)" << std::endl;
        std::cout << "Pure Parsing: " << (parsingTime - lexingTime) << " ms (" 
                  << ((parsingTime - lexingTime)/fullTime*100) << "% of total)" << std::endl;
        std::cout << "Pure Execution: " << executionTime << " ms (" 
                  << (executionTime/fullTime*100) << "% of total)" << std::endl;
        std::cout << "I/O + Overhead: " << (fullTime - parsingTime - executionTime) << " ms ("
                  << ((fullTime - parsingTime - executionTime)/fullTime*100) << "% of total)" << std::endl;
        
        AnalyzeBottlenecks(lexingTime, parsingTime - lexingTime, executionTime, 
                          fullTime - parsingTime - executionTime);
    }
    
private:
    static double BenchmarkLexing() {
        const int iterations = 1000;
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < iterations; i++) {
            Lexer lexer(TEST_CODE);
            
            // Tokenize entire input
            while (!lexer.atEnd()) {
                Token token = lexer.Dequeue();
                // Prevent optimization
                volatile auto preventOpt = token.type;
            }
        }
        
        auto end = high_resolution_clock::now();
        return duration_cast<microseconds>(end - start).count() / 1000.0 / iterations;
    }
    
    static double BenchmarkParsing() {
        const int iterations = 1000;
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < iterations; i++) {
            Parser parser;
            parser.Parse(TEST_CODE);
            
            // Prevent optimization
            volatile auto preventOpt = parser.outputStack.Count();
        }
        
        auto end = high_resolution_clock::now();
        return duration_cast<microseconds>(end - start).count() / 1000.0 / iterations;
    }
    
    static double BenchmarkExecution() {
        const int iterations = 100;  // Lower due to full compilation cost
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < iterations; i++) {
            // Create fresh interpreter (simulates execution only after compilation)
            Interpreter interp;
            interp.Reset(TEST_CODE);
            interp.Compile();
            
            // Time just the execution phase
            auto exec_start = high_resolution_clock::now();
            while (!interp.Done()) {
                interp.RunUntilDone();
            }
            auto exec_end = high_resolution_clock::now();
            
            // Only count execution time, not compilation
            if (i == 0) {
                start = exec_start;  // Reset timer to exclude first compilation
            }
        }
        
        auto end = high_resolution_clock::now();
        return duration_cast<microseconds>(end - start).count() / 1000.0 / (iterations - 1);
    }
    
    static double BenchmarkFullPipeline() {
        const int iterations = 100;  // Lower count due to expense
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < iterations; i++) {
            Interpreter interp;
            interp.Reset(TEST_CODE);
            interp.Compile();
            while (!interp.Done()) {
                interp.RunUntilDone();
            }
        }
        
        auto end = high_resolution_clock::now();
        return duration_cast<microseconds>(end - start).count() / 1000.0 / iterations;
    }
    
    static void AnalyzeBottlenecks(double lexing, double parsing, double execution, double overhead) {
        std::cout << "\n=== Bottleneck Analysis ===" << std::endl;
        
        double total = lexing + parsing + execution + overhead;
        
        if (lexing > parsing && lexing > execution && lexing > overhead) {
            std::cout << "🎯 PRIMARY BOTTLENECK: Lexing (" << (lexing/total*100) << "%)" << std::endl;
            std::cout << "   Optimization Target: Character-by-character tokenization" << std::endl;
            std::cout << "   Strategy: Bulk lexing, lookup tables, reduced string ops" << std::endl;
        } else if (parsing > lexing && parsing > execution && parsing > overhead) {
            std::cout << "🎯 PRIMARY BOTTLENECK: Parsing (" << (parsing/total*100) << "%)" << std::endl;
            std::cout << "   Optimization Target: AST construction and allocations" << std::endl;
            std::cout << "   Strategy: Object pooling, table-driven parsing" << std::endl;
        } else if (execution > lexing && execution > parsing && execution > overhead) {
            std::cout << "🎯 PRIMARY BOTTLENECK: Execution (" << (execution/total*100) << "%)" << std::endl;
            std::cout << "   Note: Phase 2.1 already optimized this - investigate regression" << std::endl;
        } else {
            std::cout << "🎯 PRIMARY BOTTLENECK: I/O and Overhead (" << (overhead/total*100) << "%)" << std::endl;
            std::cout << "   Optimization Target: System calls, memory allocation, startup" << std::endl;
            std::cout << "   Strategy: Output buffering, lazy loading, fast paths" << std::endl;
        }
        
        std::cout << "\nRecommended Phase 2.2 Priority:" << std::endl;
        if (lexing + parsing > execution + overhead) {
            std::cout << "   1. Focus on compile-time optimizations (lexer/parser)" << std::endl;
            std::cout << "   2. Target: " << ((lexing + parsing)/total*100) << "% potential improvement" << std::endl;
        } else {
            std::cout << "   1. Focus on runtime optimizations (I/O, startup, execution)" << std::endl;
            std::cout << "   2. Target: " << ((execution + overhead)/total*100) << "% potential improvement" << std::endl;
        }
    }
};

int main() {
    try {
        ParsingProfiler::RunBenchmark();
        return 0;
    } catch (const MiniscriptException& e) {
        std::cerr << "Error: " << e.message.c_str() << std::endl;
        return 1;
    }
}