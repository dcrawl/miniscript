#include "MiniScript-cpp/src/MiniScript/MiniscriptParser.h"
#include "MiniScript-cpp/src/MiniScript/MiniscriptLexer.h"
#include "MiniScript-cpp/src/MiniScript/MiniscriptTAC.h"
#include "MiniScript-cpp/src/MiniScript/TypeSpecializationEngine.h"
#include <iostream>
#include <vector>

using namespace MiniScript;

// Test cases representing common MiniScript patterns
struct TestCase {
    String name;
    String code;
    int expectedSpecializations;
    String description;
};

int main() {
    std::cout << "=== Type Inference Enhancement Potential Analysis ===\n\n";
    
    std::vector<TestCase> testCases = {
        // BASIC LITERAL ARITHMETIC
        {
            "Direct Literals",
            "result = 5.0 + 3.0",
            1, // Should specialize APlusB -> ADD_NUM_NUM
            "Direct numeric literals - should be easily detectable"
        },
        
        // SIMPLE VARIABLE ARITHMETIC  
        {
            "Simple Variables",
            "a = 10.0\nb = 20.0\nc = a + b",
            1, // Should specialize APlusB -> ADD_NUM_NUM if we track variable types
            "Variables assigned from literals - requires basic type propagation"
        },
        
        // CHAINED ARITHMETIC
        {
            "Chained Operations", 
            "a = 5.0\nb = 3.0\nc = a + b\nd = c * 2.0",
            2, // Should specialize both + and *
            "Multiple operations on typed variables - requires type flow analysis"
        },
        
        // STRING OPERATIONS
        {
            "String Concatenation",
            "name = \"John\"\ngreeting = \"Hello \" + name",
            1, // Should specialize APlusB -> ADD_STR_STR
            "String concatenation - requires string type detection"
        },
        
        // MIXED TYPES (SHOULD NOT SPECIALIZE)
        {
            "Mixed Types",
            "a = 5.0\nb = \"hello\"\nc = a + b", 
            0, // Should NOT specialize - mixed types
            "Mixed numeric/string - should remain generic for safety"
        },
        
        // FUNCTION PARAMETERS (HARDER)
        {
            "Function Parameters",
            "function add(x, y)\n  return x + y\nend function\nresult = add(5.0, 3.0)",
            0, // Currently no cross-function inference
            "Function parameters - requires interprocedural analysis"
        },
        
        // LOOPS (HARDER)
        {
            "Loop Variables",
            "sum = 0.0\nfor i in range(10)\n  sum = sum + i\nend for",
            0, // Currently no loop-aware inference  
            "Loop variables - requires flow-sensitive analysis"
        },
        
        // MAP/LIST ACCESS  
        {
            "Container Access",
            "data = {\"key\": 42}\nvalue = data[\"key\"]",
            0, // Should potentially specialize to MAP_GET_STR
            "Map access with string literal key"
        },
        
        // COMPLEX EXPRESSIONS
        {
            "Complex Expression",
            "a = 2.0\nb = 3.0\nresult = (a + b) * (a - b)",
            0, // Currently limited expression analysis
            "Complex expressions - requires expression-level type inference"
        }
    };
    
    int totalSpecializations = 0;
    int potentialSpecializations = 0;
    int totalOperations = 0;
    
    for (const auto& testCase : testCases) {
        std::cout << "Testing: " << testCase.name.c_str() << "\n";
        std::cout << "Description: " << testCase.description.c_str() << "\n";
        std::cout << "Code: " << testCase.code.c_str() << "\n";
        
        try {
            Parser parser;
            parser.Parse(testCase.code);
            
            // Count arithmetic/comparison operations before specialization
            int operations = 0;
            for (long i = 0; i < parser.output->code.Count(); ++i) {
                TACLine::Op op = parser.output->code[i].op;
                if (op == TACLine::Op::APlusB || op == TACLine::Op::AMinusB ||
                    op == TACLine::Op::ATimesB || op == TACLine::Op::ADividedByB ||
                    op == TACLine::Op::AEqualB || op == TACLine::Op::ANotEqualB) {
                    operations++;
                }
            }
            
            // Apply current specialization
            TypeSpecializationEngine engine;
            auto stats = engine.specializeFunction(parser.output->code);
            
            // Count specialized operations after
            int specialized = 0;
            for (long i = 0; i < parser.output->code.Count(); ++i) {
                TACLine::Op op = parser.output->code[i].op;
                if (op == TACLine::Op::ADD_NUM_NUM || op == TACLine::Op::SUB_NUM_NUM ||
                    op == TACLine::Op::MUL_NUM_NUM || op == TACLine::Op::DIV_NUM_NUM ||
                    op == TACLine::Op::ADD_STR_STR || op == TACLine::Op::EQ_NUM_NUM) {
                    specialized++;
                }
            }
            
            std::cout << "Operations found: " << operations 
                      << ", Specialized: " << specialized 
                      << ", Expected: " << testCase.expectedSpecializations << "\n";
            
            if (specialized < testCase.expectedSpecializations) {
                std::cout << "❌ MISSED OPPORTUNITY: Could specialize " 
                          << (testCase.expectedSpecializations - specialized) << " more operations\n";
            } else if (specialized >= testCase.expectedSpecializations) {
                std::cout << "✅ GOOD: Met or exceeded expectations\n";
            }
            
            totalOperations += operations;
            totalSpecializations += specialized;
            potentialSpecializations += testCase.expectedSpecializations;
            
        } catch (...) {
            std::cout << "❌ Parse error\n";
        }
        
        std::cout << "\n";
    }
    
    // Summary Analysis
    std::cout << "=== SUMMARY ANALYSIS ===\n";
    std::cout << "Total operations analyzed: " << totalOperations << "\n";
    std::cout << "Currently specialized: " << totalSpecializations << "\n";
    std::cout << "Potential specializations: " << potentialSpecializations << "\n";
    
    if (totalOperations > 0) {
        double currentRate = (double)totalSpecializations / totalOperations * 100.0;
        double potentialRate = (double)potentialSpecializations / totalOperations * 100.0;
        double improvement = potentialRate - currentRate;
        
        std::cout << "\nSpecialization Rate:\n";
        std::cout << "  Current: " << currentRate << "%\n";
        std::cout << "  Potential: " << potentialRate << "%\n";
        std::cout << "  Improvement Opportunity: " << improvement << "%\n";
        
        if (improvement > 0) {
            std::cout << "\n🚀 ENHANCEMENT POTENTIAL: " << improvement 
                      << "% improvement in specialization rate\n";
            std::cout << "This could translate to significant performance gains!\n";
        }
    }
    
    std::cout << "\n=== KEY LIMITATIONS IDENTIFIED ===\n";
    std::cout << "1. ❌ No literal propagation through simple assignments\n";
    std::cout << "2. ❌ No type flow analysis across multiple statements\n"; 
    std::cout << "3. ❌ No expression-level type inference\n";
    std::cout << "4. ❌ No interprocedural analysis (function parameters)\n";
    std::cout << "5. ❌ No container operation specialization\n";
    std::cout << "6. ❌ Limited string operation detection\n";
    
    std::cout << "\n=== RECOMMENDED ENHANCEMENTS ===\n";
    std::cout << "Priority 1 (HIGH IMPACT, LOW COMPLEXITY):\n";
    std::cout << "  • Literal propagation for simple assignments (a = 5.0; b = a + 3.0)\n";
    std::cout << "  • String literal detection and concatenation specialization\n";
    std::cout << "  • Container access with literal keys (map[\"key\"], list[0])\n";
    std::cout << "\nPriority 2 (MEDIUM IMPACT, MEDIUM COMPLEXITY):\n";
    std::cout << "  • Multi-statement type flow analysis\n";
    std::cout << "  • Expression result type propagation\n";
    std::cout << "  • Constant folding integration\n";
    std::cout << "\nPriority 3 (HIGH IMPACT, HIGH COMPLEXITY):\n";
    std::cout << "  • Interprocedural analysis for function parameters\n";
    std::cout << "  • Flow-sensitive analysis for loops and conditionals\n";
    std::cout << "  • Advanced container type tracking\n";
    
    return 0;
}