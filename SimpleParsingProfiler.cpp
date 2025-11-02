//
// SimpleParsingProfiler.cpp
// Phase 2.2 - Simple parsing performance analysis
//

#include <chrono>
#include <iostream>
#include "MiniscriptLexer.h"
#include "MiniscriptParser.h"

using namespace MiniScript;
using namespace std::chrono;

const String TEST_CODE = "sum = 0\nfor i in range(10000)\n  sum = sum + i * 2\nend for\nprint sum";

int main() {
    std::cout << "\n=== Phase 2.2 Parsing Performance Analysis ===" << std::endl;
    
    // Test 1: Lexing Performance
    const int lexing_iterations = 1000;
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < lexing_iterations; i++) {
        Lexer lexer(TEST_CODE);
        
        // Tokenize entire input
        while (!lexer.atEnd()) {
            Token token = lexer.Dequeue();
            volatile auto preventOpt = token.type;
        }
    }
    
    auto end = high_resolution_clock::now();
    double lexingTime = duration_cast<microseconds>(end - start).count() / 1000.0 / lexing_iterations;
    
    // Test 2: Parsing Performance
    const int parsing_iterations = 1000;
    start = high_resolution_clock::now();
    
    for (int i = 0; i < parsing_iterations; i++) {
        Parser parser;
        parser.Parse(TEST_CODE);
        
        volatile auto preventOpt = parser.outputStack.Count();
    }
    
    end = high_resolution_clock::now();
    double parsingTime = duration_cast<microseconds>(end - start).count() / 1000.0 / parsing_iterations;
    
    // Results
    std::cout << "1. Pure Lexing Time: " << lexingTime << " ms" << std::endl;
    std::cout << "2. Total Parsing Time (includes lexing): " << parsingTime << " ms" << std::endl;
    std::cout << "3. Pure Parsing Time: " << (parsingTime - lexingTime) << " ms" << std::endl;
    
    // Compare to Phase 2.1 findings of ~20ms total
    const double PHASE21_TOTAL_TIME = 20.0;  // From our findings
    
    std::cout << "\n=== Performance Breakdown vs Phase 2.1 Total (20ms) ===" << std::endl;
    std::cout << "Lexing: " << lexingTime << " ms (" 
              << (lexingTime/PHASE21_TOTAL_TIME*100) << "% of total)" << std::endl;
    std::cout << "Parsing: " << (parsingTime - lexingTime) << " ms (" 
              << ((parsingTime - lexingTime)/PHASE21_TOTAL_TIME*100) << "% of total)" << std::endl;
    std::cout << "Remaining (execution + I/O): " << (PHASE21_TOTAL_TIME - parsingTime) << " ms ("
              << ((PHASE21_TOTAL_TIME - parsingTime)/PHASE21_TOTAL_TIME*100) << "% of total)" << std::endl;
    
    std::cout << "\n=== Phase 2.2 Optimization Priorities ===" << std::endl;
    
    if (parsingTime / PHASE21_TOTAL_TIME > 0.20) {
        std::cout << "🎯 HIGH PRIORITY: Parsing optimization (" << (parsingTime/PHASE21_TOTAL_TIME*100) << "% of total)" << std::endl;
        
        if (lexingTime > (parsingTime - lexingTime)) {
            std::cout << "   Primary target: LEXER optimization (character-by-character tokenization)" << std::endl;
            std::cout << "   Secondary target: Parser optimization (AST construction)" << std::endl;
        } else {
            std::cout << "   Primary target: PARSER optimization (AST construction)" << std::endl;
            std::cout << "   Secondary target: Lexer optimization (tokenization)" << std::endl;
        }
    } else {
        std::cout << "ℹ️  LOWER PRIORITY: Parsing is only " << (parsingTime/PHASE21_TOTAL_TIME*100) << "% of total" << std::endl;
        std::cout << "   Recommendation: Focus on I/O, startup, or execution optimization first" << std::endl;
    }
    
    // Lexer analysis
    std::cout << "\n=== Lexer Implementation Analysis ===" << std::endl;
    std::cout << "Current approach: Character-by-character with String operations" << std::endl;
    std::cout << "Bottlenecks identified:" << std::endl;
    std::cout << "  - SkipWhitespaceAndComment() called repeatedly" << std::endl;
    std::cout << "  - String.SubstringB() for every token" << std::endl;
    std::cout << "  - Linear character classification (c >= '0' && c <= '9')" << std::endl;
    std::cout << "  - Dequeue() processes one token at a time" << std::endl;
    
    std::cout << "\n=== Parser Implementation Analysis ===" << std::endl;
    std::cout << "Current approach: Recursive descent with many allocations" << std::endl;
    std::cout << "Bottlenecks identified:" << std::endl;
    std::cout << "  - ParseState::Add() creates new TACLine objects" << std::endl;
    std::cout << "  - List<T>::Add() resizes backing array frequently" << std::endl;
    std::cout << "  - Recursive ParseExpression() calls create stack pressure" << std::endl;
    std::cout << "  - Token objects created/destroyed frequently" << std::endl;
    
    return 0;
}