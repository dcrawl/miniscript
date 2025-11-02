#pragma once

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/Casting.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

// Forward declarations for MiniScript types
struct Value;
struct Context;

// Simple TAC operation enumeration
enum Op {
    AssignOp, OpAdd, OpSubtract, OpMultiply, OpDivide, OpMod, OpPower,
    OpEqual, OpNotEqual, OpGreater, OpGreatEqual, OpLess, OpLessEqual,
    OpAnd, OpOr, OpNot, CallOp, ReturnOp, JumpOp, JumpIfFalse, LabelOp
};

// Simple TAC line structure  
struct TACLine {
    Op op;
    std::string lhs;
    std::string rhsA;
    std::string rhsB;
    
    TACLine(Op operation, const std::string& result, const std::string& a = "", const std::string& b = "")
        : op(operation), lhs(result), rhsA(a), rhsB(b) {}
};

namespace MiniScriptJIT {

    /// Advanced IR Generator for comprehensive MiniScript operation support
    class AdvancedIRGenerator {
    public:
        AdvancedIRGenerator(llvm::LLVMContext& context, llvm::Module* module);
        
        /// Generate LLVM IR for a sequence of TAC instructions
        llvm::Function* generateFunction(const std::vector<TACLine>& tac, 
                                       const std::string& functionName = "miniscript_function");
        
        /// Convert single TAC instruction to LLVM IR
        llvm::Value* convertTACLine(const TACLine& line);
        
        /// Runtime type system integration
        void setupRuntimeTypes();
        
        /// Optimization pipeline setup
        void setupOptimizationPipeline();
        
    private:
        llvm::LLVMContext& context_;
        llvm::Module* module_;
        std::unique_ptr<llvm::IRBuilder<>> builder_;
        llvm::Function* currentFunction_;
        
        // Type system for MiniScript values
        llvm::StructType* valueType_;          // MiniScript Value wrapper
        llvm::PointerType* valuePtr_;          // Value*
        llvm::Type* numberType_;               // double for numbers
        llvm::Type* stringType_;               // string representation
        llvm::PointerType* stringPtr_;         // char* for strings
        llvm::Type* boolType_;                 // bool for booleans
        
        // Runtime function declarations
        llvm::Function* addNumbersFunc_;       // Runtime number addition
        llvm::Function* addStringsFunc_;       // Runtime string concatenation
        llvm::Function* compareValuesFunc_;    // Runtime value comparison
        llvm::Function* callFunctionFunc_;     // Runtime function calls
        llvm::Function* getListItemFunc_;      // Runtime list access
        llvm::Function* setListItemFunc_;      // Runtime list assignment
        llvm::Function* getMapItemFunc_;       // Runtime map access
        llvm::Function* setMapItemFunc_;       // Runtime map assignment
        llvm::Function* createListFunc_;       // Runtime list creation
        llvm::Function* createMapFunc_;        // Runtime map creation
        
        // Value tracking for TAC variables
        std::unordered_map<std::string, llvm::Value*> tacVariables_;
        std::unordered_map<std::string, llvm::BasicBlock*> labelBlocks_;
        
        // Control flow state
        std::vector<llvm::BasicBlock*> breakTargets_;
        std::vector<llvm::BasicBlock*> continueTargets_;
        
        // Advanced IR generation methods
        llvm::Value* generateArithmeticOp(const TACLine& line);
        llvm::Value* generateComparisonOp(const TACLine& line);
        llvm::Value* generateLogicalOp(const TACLine& line);
        llvm::Value* generateAssignment(const TACLine& line);
        llvm::Value* generateFunctionCall(const TACLine& line);
        llvm::Value* generateListOperation(const TACLine& line);
        llvm::Value* generateMapOperation(const TACLine& line);
        void generateControlFlow(const TACLine& line);
        
        // Type conversion and checking
        llvm::Value* convertToNumber(llvm::Value* value);
        llvm::Value* convertToString(llvm::Value* value);
        llvm::Value* convertToBool(llvm::Value* value);
        llvm::Value* checkValueType(llvm::Value* value, int expectedType);
        
        // Dynamic typing support
        llvm::Value* createTypedValue(llvm::Value* rawValue, int type);
        llvm::Value* extractTypedValue(llvm::Value* typedValue, int expectedType);
        llvm::Value* getValueType(llvm::Value* typedValue);
        
        // Runtime helper integration
        void declareRuntimeFunctions();
        llvm::Value* callRuntimeFunction(llvm::Function* func, 
                                       const std::vector<llvm::Value*>& args);
        
        // Memory management
        llvm::Value* allocateValue();
        void deallocateValue(llvm::Value* value);
        
        // Optimization helpers
        bool isConstantExpression(const TACLine& line);
        llvm::Value* foldConstantExpression(const TACLine& line);
        void optimizeBasicBlock(llvm::BasicBlock* block);
        
        // Error handling
        void generateRuntimeCheck(llvm::Value* condition, const std::string& errorMsg);
        llvm::BasicBlock* createErrorBlock(const std::string& errorMsg);
        
        // Debug information support
        void attachDebugInfo(llvm::Instruction* inst, const TACLine& line);
        void setupDebugInfo();
    };
    
    /// TAC Operation Types for comprehensive support
    enum class TACOpType {
        // Arithmetic operations
        Add, Subtract, Multiply, Divide, Modulo, Power,
        
        // Comparison operations  
        Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual,
        
        // Logical operations
        And, Or, Not,
        
        // Assignment operations
        Assign, AssignList, AssignMap,
        
        // Function operations
        Call, Return, Param, LocalVar,
        
        // Control flow
        Jump, JumpIfTrue, JumpIfFalse, Label,
        
        // List operations
        ListCreate, ListGet, ListSet, ListAdd, ListInsert, ListRemove,
        
        // Map operations
        MapCreate, MapGet, MapSet, MapHas, MapRemove, MapKeys,
        
        // Object operations
        MemberGet, MemberSet, MemberCall,
        
        // Type operations
        TypeCheck, TypeCast, IsA,
        
        // String operations
        StringConcat, StringSlice, StringFind, StringReplace,
        
        // Advanced operations
        Yield, Await, Throw, Try, Catch
    };
    
    /// Enhanced TAC Line structure for advanced operations
    struct AdvancedTACLine {
        TACOpType op;
        std::string result;
        std::vector<std::string> operands;
        std::unordered_map<std::string, std::string> metadata;
        int lineNumber = 0;
        std::string sourceFile;
        
        // Type hints for optimization
        std::string resultType;
        std::vector<std::string> operandTypes;
        
        // Runtime profiling data
        mutable size_t executionCount = 0;
        mutable double averageTime = 0.0;
    };

} // namespace MiniScriptJIT