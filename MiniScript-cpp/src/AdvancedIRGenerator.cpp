#include "AdvancedIRGenerator.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include <iostream>

namespace MiniScriptJIT {

AdvancedIRGenerator::AdvancedIRGenerator(llvm::LLVMContext& context, llvm::Module* module)
    : context_(context), module_(module), currentFunction_(nullptr) {
    builder_ = std::make_unique<llvm::IRBuilder<>>(context_);
    setupRuntimeTypes();
    declareRuntimeFunctions();
    setupDebugInfo();
}

void AdvancedIRGenerator::setupRuntimeTypes() {
    // MiniScript Value type - represents any MiniScript value with type tag
    std::vector<llvm::Type*> valueFields = {
        llvm::Type::getInt32Ty(context_),    // type tag (number, string, list, map, etc.)
        llvm::Type::getDoubleTy(context_),   // number value
        llvm::PointerType::get(llvm::Type::getInt8Ty(context_), 0), // string/object pointer
        llvm::Type::getInt64Ty(context_),    // reference count
        llvm::Type::getInt32Ty(context_)     // flags (immutable, etc.)
    };
    
    valueType_ = llvm::StructType::create(context_, valueFields, "MiniScriptValue");
    valuePtr_ = llvm::PointerType::get(valueType_, 0);
    
    // Basic types
    numberType_ = llvm::Type::getDoubleTy(context_);
    stringPtr_ = llvm::PointerType::get(llvm::Type::getInt8Ty(context_), 0);
    boolType_ = llvm::Type::getInt1Ty(context_);
}

void AdvancedIRGenerator::declareRuntimeFunctions() {
    // Runtime arithmetic functions
    auto voidType = llvm::Type::getVoidTy(context_);
    auto int32Type = llvm::Type::getInt32Ty(context_);
    
    // Binary operations: Value* op(Value* a, Value* b)
    auto binaryOpType = llvm::FunctionType::get(valuePtr_, {valuePtr_, valuePtr_}, false);
    addNumbersFunc_ = llvm::Function::Create(binaryOpType, 
        llvm::Function::ExternalLinkage, "miniscript_add", module_);
    
    // Comparison: int32 compare(Value* a, Value* b)  
    auto compareType = llvm::FunctionType::get(int32Type, {valuePtr_, valuePtr_}, false);
    compareValuesFunc_ = llvm::Function::Create(compareType,
        llvm::Function::ExternalLinkage, "miniscript_compare", module_);
        
    // Function calls: Value* call(Value* func, Value** args, int32 argCount)
    auto callType = llvm::FunctionType::get(valuePtr_, 
        {valuePtr_, llvm::PointerType::get(valuePtr_, 0), int32Type}, false);
    callFunctionFunc_ = llvm::Function::Create(callType,
        llvm::Function::ExternalLinkage, "miniscript_call", module_);
    
    // List operations: Value* get_list_item(Value* list, Value* index)
    getListItemFunc_ = llvm::Function::Create(binaryOpType,
        llvm::Function::ExternalLinkage, "miniscript_list_get", module_);
    
    // void set_list_item(Value* list, Value* index, Value* value)
    auto setListType = llvm::FunctionType::get(voidType, 
        {valuePtr_, valuePtr_, valuePtr_}, false);
    setListItemFunc_ = llvm::Function::Create(setListType,
        llvm::Function::ExternalLinkage, "miniscript_list_set", module_);
    
    // Map operations
    getMapItemFunc_ = llvm::Function::Create(binaryOpType,
        llvm::Function::ExternalLinkage, "miniscript_map_get", module_);
    setMapItemFunc_ = llvm::Function::Create(setListType,
        llvm::Function::ExternalLinkage, "miniscript_map_set", module_);
    
    // Creation functions: Value* create_list(), Value* create_map()
    auto createType = llvm::FunctionType::get(valuePtr_, {}, false);
    createListFunc_ = llvm::Function::Create(createType,
        llvm::Function::ExternalLinkage, "miniscript_create_list", module_);
    createMapFunc_ = llvm::Function::Create(createType,
        llvm::Function::ExternalLinkage, "miniscript_create_map", module_);
}

llvm::Function* AdvancedIRGenerator::generateFunction(const std::vector<TACLine>& tac, 
                                                     const std::string& functionName) {
    // Create function signature: Value* function(Context* ctx)
    auto contextPtrType = llvm::PointerType::get(llvm::Type::getInt8Ty(context_), 0);
    auto funcType = llvm::FunctionType::get(valuePtr_, {contextPtrType}, false);
    
    currentFunction_ = llvm::Function::Create(funcType, 
        llvm::Function::ExternalLinkage, functionName, module_);
    
    // Create entry basic block
    auto entryBlock = llvm::BasicBlock::Create(context_, "entry", currentFunction_);
    builder_->SetInsertPoint(entryBlock);
    
    // Clear state for new function
    tacVariables_.clear();
    labelBlocks_.clear();
    breakTargets_.clear();
    continueTargets_.clear();
    
    // First pass: create all label blocks
    for (const auto& line : tac) {
        if (line.op == LabelOp) {
            auto labelBlock = llvm::BasicBlock::Create(context_, 
                "label_" + line.lhs, currentFunction_);
            labelBlocks_[line.lhs] = labelBlock;
        }
    }
    
    // Second pass: generate IR for each TAC instruction
    for (const auto& line : tac) {
        // Handle label placement
        if (line.op == LabelOp) {
            auto labelBlock = labelBlocks_[line.lhs];
            if (!builder_->GetInsertBlock()->getTerminator()) {
                builder_->CreateBr(labelBlock);
            }
            builder_->SetInsertPoint(labelBlock);
            continue;
        }
        
        // Generate IR for instruction
        auto result = convertTACLine(line);
        
        // Store result if needed
        if (!line.lhs.empty() && result) {
            tacVariables_[line.lhs] = result;
        }
    }
    
    // Ensure function has return
    if (!builder_->GetInsertBlock()->getTerminator()) {
        auto nullValue = llvm::Constant::getNullValue(valuePtr_);
        builder_->CreateRet(nullValue);
    }
    
    // Verify function
    std::string errorMsg;
    llvm::raw_string_ostream errorStream(errorMsg);
    if (llvm::verifyFunction(*currentFunction_, &errorStream)) {
        std::cerr << "Function verification failed: " << errorMsg << std::endl;
        currentFunction_->eraseFromParent();
        return nullptr;
    }
    
    return currentFunction_;
}

llvm::Value* AdvancedIRGenerator::convertTACLine(const TACLine& line) {
    switch (line.op) {
        case AssignOp:
            return generateAssignment(line);
            
        case OpAdd:
        case OpSubtract:  
        case OpMultiply:
        case OpDivide:
        case OpMod:
        case OpPower:
            return generateArithmeticOp(line);
            
        case OpEqual:
        case OpNotEqual:
        case OpGreater:
        case OpGreatEqual:
        case OpLess:
        case OpLessEqual:
            return generateComparisonOp(line);
            
        case OpAnd:
        case OpOr:
        case OpNot:
            return generateLogicalOp(line);
            
        case CallOp:
            return generateFunctionCall(line);
            
        case ReturnOp:
            {
                llvm::Value* retValue = nullptr;
                if (!line.rhsA.empty()) {
                    retValue = tacVariables_[line.rhsA];
                }
                if (!retValue) {
                    retValue = llvm::Constant::getNullValue(valuePtr_);
                }
                builder_->CreateRet(retValue);
                return nullptr;
            }
            
        case JumpOp:
            generateControlFlow(line);
            return nullptr;
            
        default:
            // Fallback for unhandled operations
            std::cout << "Warning: Unhandled TAC operation in IR generation" << std::endl;
            return llvm::Constant::getNullValue(valuePtr_);
    }
}

llvm::Value* AdvancedIRGenerator::generateArithmeticOp(const TACLine& line) {
    auto lhs = tacVariables_[line.rhsA];
    auto rhs = tacVariables_[line.rhsB];
    
    if (!lhs || !rhs) {
        return llvm::Constant::getNullValue(valuePtr_);
    }
    
    // For now, use runtime function calls for all arithmetic
    // Later optimization can specialize for known number types
    switch (line.op) {
        case OpAdd:
            return callRuntimeFunction(addNumbersFunc_, {lhs, rhs});
            
        default:
            // TODO: Implement other arithmetic operations
            return callRuntimeFunction(addNumbersFunc_, {lhs, rhs});
    }
}

llvm::Value* AdvancedIRGenerator::generateComparisonOp(const TACLine& line) {
    auto lhs = tacVariables_[line.rhsA];
    auto rhs = tacVariables_[line.rhsB];
    
    if (!lhs || !rhs) {
        return llvm::Constant::getNullValue(valuePtr_);
    }
    
    // Call runtime comparison function
    auto compareResult = callRuntimeFunction(compareValuesFunc_, {lhs, rhs});
    
    // Convert comparison result to boolean value based on operation
    llvm::Value* condition = nullptr;
    auto zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0);
    
    switch (line.op) {
        case OpEqual:
            condition = builder_->CreateICmpEQ(compareResult, zero);
            break;
        case OpNotEqual:
            condition = builder_->CreateICmpNE(compareResult, zero);
            break;
        case OpLess:
            condition = builder_->CreateICmpSLT(compareResult, zero);
            break;
        case OpLessEqual:
            condition = builder_->CreateICmpSLE(compareResult, zero);
            break;
        case OpGreater:
            condition = builder_->CreateICmpSGT(compareResult, zero);
            break;
        case OpGreatEqual:
            condition = builder_->CreateICmpSGE(compareResult, zero);
            break;
        default:
            condition = llvm::ConstantInt::get(boolType_, 0);
    }
    
    // Convert boolean to MiniScript value
    return createTypedValue(condition, 2); // 2 = boolean type
}

llvm::Value* AdvancedIRGenerator::generateLogicalOp(const TACLine& line) {
    switch (line.op) {
        case OpAnd:
        case OpOr:
            {
                auto lhs = tacVariables_[line.rhsA];
                auto rhs = tacVariables_[line.rhsB];
                
                if (!lhs || !rhs) {
                    return llvm::Constant::getNullValue(valuePtr_);
                }
                
                // Convert to boolean values
                auto lhsBool = convertToBool(lhs);
                auto rhsBool = convertToBool(rhs);
                
                llvm::Value* result = nullptr;
                if (line.op == OpAnd) {
                    result = builder_->CreateAnd(lhsBool, rhsBool);
                } else {
                    result = builder_->CreateOr(lhsBool, rhsBool);
                }
                
                return createTypedValue(result, 2); // 2 = boolean type
            }
            
        case OpNot:
            {
                auto operand = tacVariables_[line.rhsA];
                if (!operand) {
                    return llvm::Constant::getNullValue(valuePtr_);
                }
                
                auto operandBool = convertToBool(operand);
                auto result = builder_->CreateNot(operandBool);
                
                return createTypedValue(result, 2); // 2 = boolean type
            }
            
        default:
            return llvm::Constant::getNullValue(valuePtr_);
    }
}

llvm::Value* AdvancedIRGenerator::generateAssignment(const TACLine& line) {
    // Simple assignment: lhs = rhs
    if (tacVariables_.find(line.rhsA) != tacVariables_.end()) {
        return tacVariables_[line.rhsA];
    }
    
    // TODO: Handle literal values, complex assignments
    return llvm::Constant::getNullValue(valuePtr_);
}

llvm::Value* AdvancedIRGenerator::generateFunctionCall(const TACLine& line) {
    auto func = tacVariables_[line.rhsA];
    if (!func) {
        return llvm::Constant::getNullValue(valuePtr_);
    }
    
    // For now, simple function call with no arguments
    // TODO: Handle argument passing
    auto nullArgs = llvm::Constant::getNullValue(llvm::PointerType::get(valuePtr_, 0));
    auto argCount = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0);
    
    return callRuntimeFunction(callFunctionFunc_, {func, nullArgs, argCount});
}

llvm::Value* AdvancedIRGenerator::generateListOperation(const TACLine& line) {
    // TODO: Implement comprehensive list operations
    return llvm::Constant::getNullValue(valuePtr_);
}

llvm::Value* AdvancedIRGenerator::generateMapOperation(const TACLine& line) {
    // TODO: Implement comprehensive map operations  
    return llvm::Constant::getNullValue(valuePtr_);
}

void AdvancedIRGenerator::generateControlFlow(const TACLine& line) {
    if (line.op == JumpOp) {
        if (labelBlocks_.find(line.rhsA) != labelBlocks_.end()) {
            builder_->CreateBr(labelBlocks_[line.rhsA]);
        }
    }
    // TODO: Handle conditional jumps
}

llvm::Value* AdvancedIRGenerator::convertToBool(llvm::Value* value) {
    // Extract boolean value from MiniScript Value
    // For now, simple implementation
    return llvm::ConstantInt::get(boolType_, 1);
}

llvm::Value* AdvancedIRGenerator::createTypedValue(llvm::Value* rawValue, int type) {
    // Create MiniScript Value structure
    auto value = builder_->CreateAlloca(valueType_);
    
    // Set type tag
    auto typePtr = builder_->CreateStructGEP(valueType_, value, 0);
    auto typeConstant = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), type);
    builder_->CreateStore(typeConstant, typePtr);
    
    // Store the actual value based on type
    if (type == 0) { // number
        auto numberPtr = builder_->CreateStructGEP(valueType_, value, 1);
        if (rawValue->getType()->isDoubleTy()) {
            builder_->CreateStore(rawValue, numberPtr);
        }
    } else if (type == 2) { // boolean  
        auto numberPtr = builder_->CreateStructGEP(valueType_, value, 1);
        auto boolAsDouble = builder_->CreateUIToFP(rawValue, numberType_);
        builder_->CreateStore(boolAsDouble, numberPtr);
    }
    
    return value;
}

llvm::Value* AdvancedIRGenerator::callRuntimeFunction(llvm::Function* func, 
                                                     const std::vector<llvm::Value*>& args) {
    return builder_->CreateCall(func, args);
}

void AdvancedIRGenerator::setupOptimizationPipeline() {
    // Modern LLVM optimization pipeline setup
    // This will be called after IR generation is complete
}

void AdvancedIRGenerator::setupDebugInfo() {
    // Debug information setup for better debugging experience
    // TODO: Implement debug info generation
}

} // namespace MiniScriptJIT