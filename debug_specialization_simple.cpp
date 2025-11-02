#include "MiniScript-cpp/src/MiniScript/MiniscriptParser.h"
#include "MiniScript-cpp/src/MiniScript/TypeSpecializationEngine.h"
#include <iostream>

using namespace MiniScript;

// Simple operation name helper
const char* GetOpName(TACLine::Op op) {
    switch (op) {
        case TACLine::Op::APlusB: return "A + B";
        case TACLine::Op::AMinusB: return "A - B";
        case TACLine::Op::ATimesB: return "A * B";
        case TACLine::Op::ADividedByB: return "A / B";
        case TACLine::Op::AEqualB: return "A == B";
        case TACLine::Op::ADD_NUM_NUM: return "ADD_NUM_NUM";
        case TACLine::Op::SUB_NUM_NUM: return "SUB_NUM_NUM";
        case TACLine::Op::MUL_NUM_NUM: return "MUL_NUM_NUM";
        case TACLine::Op::DIV_NUM_NUM: return "DIV_NUM_NUM";
        case TACLine::Op::EQ_NUM_NUM: return "EQ_NUM_NUM";
        case TACLine::Op::ADD_STR_STR: return "ADD_STR_STR";
        case TACLine::Op::AssignA: return "AssignA";
        case TACLine::Op::AssignImplicit: return "AssignImplicit";
        case TACLine::Op::CopyA: return "CopyA";
        case TACLine::Op::ElemBofA: return "ElemBofA";
        case TACLine::Op::PushParam: return "PushParam";
        case TACLine::Op::CallFunctionA: return "CallFunctionA";
        default: {
            static char buffer[32];
            snprintf(buffer, sizeof(buffer), "Op_%d", (int)op);
            return buffer;
        }
    }
}

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
            std::cout << i << ": " << GetOpName(line.op);
            
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
            std::cout << i << ": " << GetOpName(line.op);
            
            // Check if this line changed
            if (i < originalCode.Count() && line.op != originalCode[i].op) {
                std::cout << " (CHANGED from " << GetOpName(originalCode[i].op) << ")";
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
            
            // Let's also test with a more obvious case
            std::cout << "\nTesting more obvious numeric case: 5 + 10\n";
            String numericCode = "result = 5 + 10";
            
            Parser numParser;
            numParser.Parse(numericCode);
            
            std::cout << "Before specialization:\n";
            for (long i = 0; i < numParser.output->code.Count(); ++i) {
                TACLine& line = numParser.output->code[i];
                std::cout << i << ": " << GetOpName(line.op) << "\n";
            }
            
            engine.specializeFunction(numParser.output->code);
            
            std::cout << "After specialization:\n";
            for (long i = 0; i < numParser.output->code.Count(); ++i) {
                TACLine& line = numParser.output->code[i];
                std::cout << i << ": " << GetOpName(line.op) << "\n";
            }
        }
        
    } catch (...) {
        std::cout << "❌ Parse failed\n";
    }
}

int main() {
    debugSpecializationFailure();
    return 0;
}