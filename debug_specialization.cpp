#include "MiniScript-cpp/src/MiniScript/MiniscriptParser.h"
#include "MiniScript-cpp/src/MiniScript/TypeSpecializationEngine.h"
#include <iostream>

using namespace MiniScript;

// Debug why specialization isn't working
void debugSpecializationFailure() {
    std::cout << "=== DEBUGGING SPECIALIZATION ENGINE INTEGRATION ===\n\n";
    
    // Simple test case that should definitely be specialized
    String simpleCode = "x = 5\ny = 10\nresult = x + y";
    
    std::cout << "Test code:\n" << simpleCode.c_str() << "\n\n";
    
    try {
        Parser parser;
        parser.Parse(simpleCode);
        
        std::cout << "Parse successful. TAC operations:\n";
        
        // Show the TAC before specialization
        for (long i = 0; i < parser.output->code.Count(); ++i) {
            TACLine& line = parser.output->code[i];
            std::cout << i << ": " << TACLine::OpName(line.op).c_str();
            
            if (!line.rhsA.IsNull()) {
                std::cout << " A=" << line.rhsA.ToString().c_str();
            }
            if (!line.rhsB.IsNull()) {
                std::cout << " B=" << line.rhsB.ToString().c_str();
            }
            if (!line.lhs.IsNull()) {
                std::cout << " -> " << line.lhs.ToString().c_str();
            }
            
            std::cout << "\n";
        }
        
        // Check if TypeSpecializationEngine is being called
        std::cout << "\nTesting manual TypeSpecializationEngine call:\n";
        
        TypeSpecializationEngine engine;
        
        // Create a copy to test manual specialization
        List<TACLine> originalCode = parser.output->code;
        
        // Call specialization manually
        engine.specializeFunction(parser.output->code);
        
        std::cout << "\nAfter manual specialization:\n";
        
        bool anyChanges = false;
        for (long i = 0; i < parser.output->code.Count(); ++i) {
            TACLine& line = parser.output->code[i];
            std::cout << i << ": " << TACLine::OpName(line.op).c_str();
            
            // Check if this line changed
            if (i < originalCode.Count() && line.op != originalCode[i].op) {
                std::cout << " (CHANGED from " << TACLine::OpName(originalCode[i].op).c_str() << ")";
                anyChanges = true;
            }
            
            if (!line.rhsA.IsNull()) {
                std::cout << " A=" << line.rhsA.ToString().c_str();
            }
            if (!line.rhsB.IsNull()) {
                std::cout << " B=" << line.rhsB.ToString().c_str();
            }
            if (!line.lhs.IsNull()) {
                std::cout << " -> " << line.lhs.ToString().c_str();
            }
            
            std::cout << "\n";
        }
        
        if (anyChanges) {
            std::cout << "\n✅ Specialization engine IS working manually!\n";
            std::cout << "🔍 Issue: Parser integration hooks may not be triggering correctly\n";
        } else {
            std::cout << "\n❌ Specialization engine is not working even manually\n";
            std::cout << "🔍 Issue: Type inference is not detecting number types\n";
        }
        
    } catch (...) {
        std::cout << "❌ Parse failed\n";
    }
}

// Test if parser hooks are being called
void testParserIntegration() {
    std::cout << "\n=== TESTING PARSER INTEGRATION HOOKS ===\n\n";
    
    String functionCode = R"(
myFunction = function(a, b)
    return a + b
end function
)";
    
    std::cout << "Function test code:\n" << functionCode.c_str() << "\n\n";
    
    try {
        Parser parser;
        parser.Parse(functionCode);
        
        std::cout << "Function parsed successfully.\n";
        std::cout << "Functions found: " << parser.output->functions.Count() << "\n";
        
        if (parser.output->functions.Count() > 0) {
            Function* func = parser.output->functions.GetValue(0);
            if (func != nullptr) {
                std::cout << "First function TAC operations: " << func->code.Count() << "\n";
                
                for (long i = 0; i < func->code.Count(); ++i) {
                    TACLine& line = func->code[i];
                    std::cout << i << ": " << TACLine::OpName(line.op).c_str() << "\n";
                }
            }
        }
        
    } catch (...) {
        std::cout << "❌ Function parse failed\n";
    }
}

int main() {
    debugSpecializationFailure();
    testParserIntegration();
    return 0;
}