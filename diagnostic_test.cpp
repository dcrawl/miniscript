#include "MiniScript-cpp/src/MiniScript/MiniscriptParser.h"
#include "MiniScript-cpp/src/MiniScript/TypeSpecializationEngine.h"
#include <iostream>

using namespace MiniScript;

void diagnosticTest() {
    std::cout << "=== DIAGNOSTIC TEST FOR TYPE INFERENCE ISSUES ===\n\n";
    
    // Test simple assignment + operation
    String code = "x = 5\ny = x + 10";
    
    try {
        Parser parser;
        parser.Parse(code);
        
        std::cout << "Parsed code successfully. TAC lines:\n";
        for (long i = 0; i < parser.output->code.Count(); ++i) {
            TACLine& line = parser.output->code[i];
            std::cout << i << ": Op=" << (int)line.op;
            if (!line.rhsA.IsNull()) {
                std::cout << " A=" << line.rhsA.type << ":" << const_cast<Value&>(line.rhsA).ToString().c_str();
            }
            if (!line.rhsB.IsNull()) {
                std::cout << " B=" << line.rhsB.type << ":" << const_cast<Value&>(line.rhsB).ToString().c_str();
            }
            if (!line.lhs.IsNull()) {
                std::cout << " -> " << line.lhs.type << ":" << const_cast<Value&>(line.lhs).ToString().c_str();
            }
            std::cout << "\n";
        }
        
        // Test manual TypeSpecializationEngine
        std::cout << "\nTesting TypeSpecializationEngine:\n";
        TypeSpecializationEngine engine;
        auto stats = engine.specializeFunction(parser.output->code);
        
        std::cout << "Specialization stats:\n";
        std::cout << "- Total instructions: " << stats.total_instructions << "\n";
        std::cout << "- Specialized: " << stats.total_specialized << "\n";
        std::cout << "- Rate: " << stats.getSpecializationRate() << "%\n";
        
    } catch (...) {
        std::cout << "Parse failed!\n";
    }
}

int main() {
    diagnosticTest();
    return 0;
}