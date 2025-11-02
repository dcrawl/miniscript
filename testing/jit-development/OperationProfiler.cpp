//
// OperationProfiler.cpp
// Phase 2.3: Operation Frequency Analysis for Type Specialization
//

#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include "MiniScript-cpp/src/MiniScript/MiniscriptParser.h"
#include "MiniScript-cpp/src/MiniScript/MiniscriptInterpreter.h"
#include "MiniScript-cpp/src/MiniScript/MiniscriptTAC.h"
#include "MiniScript-cpp/src/MiniScript/SimpleString.h"

using namespace MiniScript;

class OperationProfiler {
private:
    std::map<TACLine::Op, long> operation_counts;
    std::map<std::pair<TACLine::Op, ValueType>, long> operation_type_counts;
    std::map<std::pair<TACLine::Op, std::pair<ValueType, ValueType>>, long> binary_op_type_counts;
    long total_operations = 0;
    
    // Hook into TACLine::Evaluate to count operations
    static OperationProfiler* instance;
    
public:
    static OperationProfiler* getInstance() {
        if (!instance) {
            instance = new OperationProfiler();
        }
        return instance;
    }
    
    void recordOperation(TACLine::Op op, const Value* operandA = nullptr, const Value* operandB = nullptr) {
        operation_counts[op]++;
        total_operations++;
        
        if (operandA) {
            operation_type_counts[{op, operandA->type}]++;
            
            if (operandB) {
                binary_op_type_counts[{op, {operandA->type, operandB->type}}]++;
            }
        }
    }
    
    void analyzeProgram(const String& sourceCode) {
        try {
            Parser parser;
            parser.Parse(sourceCode);
            
            Interpreter interp;
            interp.Reset(sourceCode);
            interp.Compile();
            
            // Analyze the compiled TAC code
            Context* ctx = interp.vm->GetGlobalContext();
            std::cout << "Analyzing " << ctx->code.Count() << " TAC instructions..." << std::endl;
            
            for (long i = 0; i < ctx->code.Count(); i++) {
                const TACLine& line = ctx->code[i];
                
                // Count the operation
                recordOperation(line.op);
                
                // Try to infer operand types from the code structure
                analyzeOperandTypes(line);
            }
            
        } catch (MiniscriptException& e) {
            std::cerr << "Error analyzing program: " << e.message.c_str() << std::endl;
        }
    }
    
    void analyzeOperandTypes(const TACLine& line) {
        // Simplified type analysis for Phase 2.3
        // This would be expanded with proper type inference in a full implementation
        
        if (line.op == TACLine::Op::APlusB || line.op == TACLine::Op::AMinusB || 
            line.op == TACLine::Op::ATimesB || line.op == TACLine::Op::ADividedByB) {
            // Arithmetic operations - prime candidates for numeric specialization
        }
    }
    
    void printResults() {
        std::cout << "\n=== Phase 2.3 Operation Frequency Analysis ===" << std::endl;
        std::cout << "Total operations analyzed: " << total_operations << std::endl;
        
        // Sort operations by frequency
        std::vector<std::pair<TACLine::Op, long>> sorted_ops;
        for (const auto& pair : operation_counts) {
            sorted_ops.push_back(pair);
        }
        
        std::sort(sorted_ops.begin(), sorted_ops.end(), 
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        std::cout << "\n=== Most Common Operations ===" << std::endl;
        std::cout << std::setw(20) << "Operation" << std::setw(10) << "Count" << std::setw(10) << "Percent" << std::endl;
        std::cout << std::string(40, '-') << std::endl;
        
        for (size_t i = 0; i < std::min(size_t(15), sorted_ops.size()); i++) {
            const auto& pair = sorted_ops[i];
            double percent = (double)pair.second / total_operations * 100.0;
            
            std::cout << std::setw(20) << opToString(pair.first).c_str() 
                     << std::setw(10) << pair.second 
                     << std::setw(9) << std::fixed << std::setprecision(1) << percent << "%" << std::endl;
        }
        
        // Identify specialization candidates
        std::cout << "\n=== Type Specialization Candidates ===" << std::endl;
        identifySpecializationCandidates(sorted_ops);
    }
    
    void identifySpecializationCandidates(const std::vector<std::pair<TACLine::Op, long>>& sorted_ops) {
        std::cout << "\nHigh-Priority Candidates (>5% of total operations):" << std::endl;
        
        for (const auto& pair : sorted_ops) {
            double percent = (double)pair.second / total_operations * 100.0;
            if (percent < 5.0) break;
            
            TACLine::Op op = pair.first;
            String recommendation = getSpecializationRecommendation(op);
            
            std::cout << "• " << opToString(op).c_str() << " (" << std::fixed << std::setprecision(1) << percent << "%) - " 
                     << recommendation.c_str() << std::endl;
        }
        
        std::cout << "\nRecommended Specialized Instructions:" << std::endl;
        std::cout << "• NUMBER_ADD, NUMBER_SUB, NUMBER_MUL, NUMBER_DIV" << std::endl;
        std::cout << "• STRING_CONCAT (for string + string)" << std::endl;
        std::cout << "• MAP_GET_STRING (for map[\"string\"])" << std::endl;
        std::cout << "• LIST_GET_INDEX (for list[number])" << std::endl;
        std::cout << "• ASSIGN_LOCAL (for local variable assignment)" << std::endl;
    }
    
    String getSpecializationRecommendation(TACLine::Op op) {
        switch (op) {
            case TACLine::Op::APlusB:
                return "NUMBER_ADD for numbers, STRING_CONCAT for strings";
            case TACLine::Op::AMinusB:
            case TACLine::Op::ATimesB:
            case TACLine::Op::ADividedByB:
                return "NUMBER_OP specialized for numeric operands";
            case TACLine::Op::AssignA:
                return "ASSIGN_LOCAL for local variables";
            case TACLine::Op::ElemBofA:
            case TACLine::Op::ElemBofIterA:
                return "MAP_GET_STRING for string keys, LIST_GET_INDEX for numeric indices";
            case TACLine::Op::AEqualB:
            case TACLine::Op::ANotEqualB:
                return "COMPARE_NUMBER, COMPARE_STRING for type-specific comparisons";
            default:
                return "Analyze for type-specific optimizations";
        }
    }
    
    String opToString(TACLine::Op op) {
        switch (op) {
            case TACLine::Op::Noop: return "Noop";
            case TACLine::Op::AssignA: return "AssignA";
            case TACLine::Op::AssignImplicit: return "AssignImplicit";
            case TACLine::Op::APlusB: return "APlusB";
            case TACLine::Op::AMinusB: return "AMinusB";
            case TACLine::Op::ATimesB: return "ATimesB";
            case TACLine::Op::ADividedByB: return "ADividedByB";
            case TACLine::Op::AModB: return "AModB";
            case TACLine::Op::APowB: return "APowB";
            case TACLine::Op::ElemBofA: return "ElemBofA";
            case TACLine::Op::ElemBofIterA: return "ElemBofIterA";
            case TACLine::Op::AEqualB: return "AEqualB";
            case TACLine::Op::ANotEqualB: return "ANotEqualB";
            case TACLine::Op::AGreaterThanB: return "AGreaterThanB";
            case TACLine::Op::AGreatOrEqualB: return "AGreatOrEqualB";
            case TACLine::Op::ALessThanB: return "ALessThanB";
            case TACLine::Op::ALessOrEqualB: return "ALessOrEqualB";
            case TACLine::Op::CallFunctionA: return "CallFunctionA";
            case TACLine::Op::GotoA: return "GotoA";
            case TACLine::Op::GotoAifB: return "GotoAifB";
            case TACLine::Op::GotoAifTrulyB: return "GotoAifTrulyB";
            case TACLine::Op::GotoAifNotB: return "GotoAifNotB";
            case TACLine::Op::PushParam: return "PushParam";
            case TACLine::Op::ReturnA: return "ReturnA";
            default: return "Unknown";
        }
    }
};

OperationProfiler* OperationProfiler::instance = nullptr;

// Test programs to analyze
const char* TEST_PROGRAMS[] = {
    // Arithmetic-heavy program
    R"(
        sum = 0
        for i in range(1000)
            sum = sum + i * 2
            result = sum / (i + 1)
        end for
        print sum
    )",
    
    // String manipulation program  
    R"(
        name = "John"
        greeting = "Hello " + name + "!"
        for i in range(10)
            message = greeting + " #" + str(i)
            parts = message.split(" ")
        end for
    )",
    
    // Map-heavy program
    R"(
        data = {}
        for i in range(100)
            key = "item" + str(i)
            data[key] = i * i
            value = data[key]
        end for
        
        for key in data.keys
            result = data[key] + 1
        end for
    )",
    
    // Mixed operations program
    R"(
        numbers = [1, 2, 3, 4, 5]
        total = 0
        for num in numbers
            total = total + num
            doubled = num * 2
            text = "Number: " + str(doubled)
        end for
        
        result = {"total": total, "count": numbers.len}
        final = result["total"] / result["count"]
    )"
};

int main() {
    std::cout << "=== Phase 2.3: MiniScript Operation Frequency Profiler ===" << std::endl;
    
    OperationProfiler* profiler = OperationProfiler::getInstance();
    
    // Analyze each test program
    for (size_t i = 0; i < sizeof(TEST_PROGRAMS) / sizeof(TEST_PROGRAMS[0]); i++) {
        std::cout << "\nAnalyzing test program " << (i + 1) << "..." << std::endl;
        profiler->analyzeProgram(TEST_PROGRAMS[i]);
    }
    
    // Print comprehensive results
    profiler->printResults();
    
    std::cout << "\n=== Phase 2.3 Next Steps ===" << std::endl;
    std::cout << "1. Implement type inference to detect when specialization is safe" << std::endl;
    std::cout << "2. Create specialized Op codes for the most common operations" << std::endl;
    std::cout << "3. Build specialized evaluation functions that skip type checks" << std::endl;
    std::cout << "4. Integrate specialization into the compilation pipeline" << std::endl;
    
    return 0;
}