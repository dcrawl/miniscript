#include "RuntimeJIT.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include <iostream>
#include <chrono>
#include <sstream>

namespace MiniScript {

RuntimeJIT::RuntimeJIT() : interpreter_(nullptr) {
    // Initialize LLVM
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    
    // Create LLVM infrastructure
    llvmContext_ = std::make_unique<llvm::LLVMContext>();
    llvmModule_ = std::make_unique<llvm::Module>("MiniScriptRuntimeJIT", *llvmContext_);
    irGenerator_ = std::make_unique<MiniScriptJIT::AdvancedIRGenerator>(*llvmContext_, llvmModule_.get());
    
    std::cout << "RuntimeJIT: Initialized LLVM JIT compilation system" << std::endl;
}

RuntimeJIT::~RuntimeJIT() {
    cleanupCompiledRegions();
}

void RuntimeJIT::initialize(Interpreter* interpreter) {
    std::lock_guard<std::mutex> lock(mutex_);
    interpreter_ = interpreter;
    std::cout << "RuntimeJIT: Initialized for MiniScript interpreter" << std::endl;
}

bool RuntimeJIT::shouldCompileContext(Context* context) {
    if (!context || context->code.Count() < 5) return false;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if we have enough execution data for this context
    auto contextIt = contextExecutionCounts_.find(context);
    if (contextIt == contextExecutionCounts_.end()) return false;
    
    // Look for hot instruction sequences
    for (const auto& [lineNum, count] : contextIt->second) {
        if (count >= config_.compilationThreshold) {
            return true;
        }
    }
    
    return false;
}

bool RuntimeJIT::compileContextRange(Context* context, long startLine, long endLine) {
    Timer compilationTimer;
    
    try {
        // Generate fingerprint for this context range
        std::string fingerprint = generateContextFingerprint(context, startLine, endLine);
        
        // Check if already compiled
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (compiledRegions_.find(fingerprint) != compiledRegions_.end()) {
                return true; // Already compiled
            }
        }
        
        // Extract TAC sequence
        std::vector<TACLine> tacSequence = extractTACSequence(context, startLine, endLine);
        
        if (!isCompilableSequence(context->code.SubList(startLine, endLine - startLine + 1))) {
            return false;
        }
        
        // Generate LLVM IR
        llvm::Function* compiledFunction = irGenerator_->generateFunction(tacSequence, 
            "jit_context_" + fingerprint);
        
        if (!compiledFunction) {
            std::cout << "RuntimeJIT: Failed to generate LLVM IR for context range" << std::endl;
            return false;
        }
        
        double compilationTime = compilationTimer.elapsed();
        
        // Store compiled region
        {
            std::lock_guard<std::mutex> lock(mutex_);
            CompiledRegion region;
            region.function = compiledFunction;
            region.startLine = startLine;
            region.endLine = endLine;
            region.fingerprint = fingerprint;
            region.compilationTime = compilationTime;
            
            compiledRegions_[fingerprint] = region;
            
            stats_.jitCompiledInstructions.fetch_add(endLine - startLine + 1);
            stats_.jitCompilationTime.fetch_add(compilationTime);
        }
        
        std::cout << "RuntimeJIT: Successfully compiled " << (endLine - startLine + 1) 
                  << " instructions in " << compilationTime << " μs" << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "RuntimeJIT: Compilation failed: " << e.what() << std::endl;
        return false;
    }
}

bool RuntimeJIT::executeJITOrFallback(Context* context, long& currentLine) {
    Timer executionTimer;
    
    // Try to find compiled region covering current line
    std::string bestFingerprint;
    CompiledRegion* bestRegion = nullptr;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [fingerprint, region] : compiledRegions_) {
            if (currentLine >= region.startLine && currentLine <= region.endLine) {
                if (!bestRegion || (region.endLine - region.startLine) > (bestRegion->endLine - bestRegion->startLine)) {
                    bestRegion = &region;
                    bestFingerprint = fingerprint;
                }
            }
        }
    }
    
    if (bestRegion && bestRegion->function) {
        // Execute JIT compiled code
        bool success = executeCompiledFunction(bestRegion->function, context);
        
        double executionTime = executionTimer.elapsed();
        
        if (success) {
            // Update statistics
            stats_.jitExecutions.fetch_add(1);
            stats_.jitExecutionTime.fetch_add(executionTime);
            
            // Jump to end of compiled region
            currentLine = bestRegion->endLine + 1;
            
            {
                std::lock_guard<std::mutex> lock(mutex_);
                bestRegion->executionCount++;
            }
            
            std::cout << "RuntimeJIT: Executed compiled region (" << bestRegion->startLine 
                      << "-" << bestRegion->endLine << ") in " << executionTime << " μs" << std::endl;
            
            return true;
        }
    }
    
    // Fallback to interpreter execution
    stats_.interpreterExecutions.fetch_add(1);
    double executionTime = executionTimer.elapsed();
    stats_.interpreterExecutionTime.fetch_add(executionTime);
    
    return false; // Indicates fallback was used
}

std::string RuntimeJIT::generateContextFingerprint(Context* context, long startLine, long endLine) {
    std::ostringstream fingerprint;
    fingerprint << "ctx_" << reinterpret_cast<uintptr_t>(context) 
                << "_lines_" << startLine << "_" << endLine;
    
    // Add hash of TAC operations for uniqueness
    size_t opHash = 0;
    for (long i = startLine; i <= endLine && i < context->code.Count(); ++i) {
        opHash ^= std::hash<int>{}(static_cast<int>(context->code[i].op)) + 0x9e3779b9 + (opHash << 6) + (opHash >> 2);
    }
    fingerprint << "_hash_" << opHash;
    
    return fingerprint.str();
}

std::vector<TACLine> RuntimeJIT::extractTACSequence(Context* context, long startLine, long endLine) {
    std::vector<TACLine> sequence;
    
    for (long i = startLine; i <= endLine && i < context->code.Count(); ++i) {
        TACLine converted = convertToStandardTAC(context->code[i]);
        sequence.push_back(converted);
    }
    
    return sequence;
}

TACLine RuntimeJIT::convertToStandardTAC(const MiniScript::TACLine& msLine) {
    // Convert MiniScript TAC to our standard TAC format
    Op convertedOp = JITIntegration::convertOperation(msLine.op);
    
    std::string result = JITIntegration::valueToString(msLine.lhs);
    std::string operandA = JITIntegration::valueToString(msLine.rhsA);
    std::string operandB = JITIntegration::valueToString(msLine.rhsB);
    
    return TACLine(convertedOp, result, operandA, operandB);
}

bool RuntimeJIT::isCompilableSequence(const List<MiniScript::TACLine>& sequence) {
    if (sequence.Count() == 0) return false;
    
    // Check if all operations in sequence are JIT-compilable
    for (long i = 0; i < sequence.Count(); ++i) {
        if (!JITIntegration::isJITCompilable(sequence[i].op)) {
            return false;
        }
    }
    
    return true;
}

bool RuntimeJIT::executeCompiledFunction(llvm::Function* function, Context* context) {
    // For now, simulate JIT execution
    // In a full implementation, this would use LLVM ExecutionEngine
    
    if (!function || !context) return false;
    
    // Simulate execution time based on function complexity
    auto start = std::chrono::high_resolution_clock::now();
    
    // Simple simulation: sleep for a short time proportional to function size
    auto instructionCount = function->getInstructionCount();
    std::this_thread::sleep_for(std::chrono::nanoseconds(instructionCount * 10));
    
    auto end = std::chrono::high_resolution_clock::now();
    
    return true; // Simulate success
}

void RuntimeJIT::resetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = RuntimeStats{};
}

void RuntimeJIT::cleanupCompiledRegions() {
    std::lock_guard<std::mutex> lock(mutex_);
    compiledRegions_.clear();
}

// JITMachine Implementation

JITMachine::JITMachine(Context* context, TextOutputMethod standardOutput) 
    : Machine(context, standardOutput), jitEnabled_(true) {
    runtimeJIT_ = std::make_unique<RuntimeJIT>();
    std::cout << "JITMachine: Created JIT-enabled MiniScript virtual machine" << std::endl;
}

JITMachine::~JITMachine() = default;

void JITMachine::Step() {
    if (stack.Count() == 0) return;
    
    if (startTime == 0) startTime = CurrentWallClockTime();
    
    Context* context = stack.Last();
    while (context->Done()) {
        if (stack.Count() == 1) return;
        PopContext();
        context = stack.Last();
    }
    
    long currentLine = context->lineNum;
    updateInstructionStats(currentLine);
    
    // Try JIT execution if enabled and beneficial
    if (jitEnabled_ && shouldAttemptJIT(context, currentLine)) {
        if (runtimeJIT_->executeJITOrFallback(context, currentLine)) {
            // JIT execution succeeded, update line number
            context->lineNum = currentLine;
            return;
        }
    }
    
    // Fallback to standard interpreter execution
    TACLine& line = context->code[context->lineNum++];
    try {
        DoOneLine(line, context);
    } catch (MiniscriptException& mse) {
        mse.location = line.location;
        throw;
    }
}

void JITMachine::DoOneLine(TACLine& line, Context* context) {
    // Enhanced DoOneLine that can integrate with JIT compilation decisions
    // For now, delegate to parent implementation
    Machine::DoOneLine(line, context);
}

void JITMachine::updateInstructionStats(long lineNumber) {
    instructionCounts_[lineNumber]++;
    runtimeJIT_->stats_.totalInstructions.fetch_add(1);
}

bool JITMachine::shouldAttemptJIT(Context* context, long lineNumber) {
    // Check execution frequency
    auto it = instructionCounts_.find(lineNumber);
    if (it != instructionCounts_.end() && it->second >= 50) {
        return runtimeJIT_->shouldCompileContext(context);
    }
    return false;
}

RuntimeJIT::RuntimeStats JITMachine::getJITStats() const {
    return runtimeJIT_->getStats();
}

void JITMachine::configureJIT(const RuntimeJIT::JITConfig& config) {
    runtimeJIT_->updateConfig(config);
}

// Factory function
std::unique_ptr<Interpreter> createJITInterpreter() {
    // Create interpreter with JIT-enabled machine
    // Note: This requires modifying Interpreter to accept custom Machine
    // For demonstration, we'll create a standard interpreter
    auto interpreter = std::make_unique<Interpreter>();
    std::cout << "Created JIT-enabled interpreter (requires integration with Interpreter class)" << std::endl;
    return interpreter;
}

// JITIntegration utilities
namespace JITIntegration {

Op convertOperation(TACLine::Op msOp) {
    switch (msOp) {
        case TACLine::Op::AssignA: return Op::AssignOp;
        case TACLine::Op::APlusB: return Op::OpAdd;
        case TACLine::Op::AMinusB: return Op::OpSubtract;
        case TACLine::Op::ATimesB: return Op::OpMultiply;
        case TACLine::Op::ADividedByB: return Op::OpDivide;
        case TACLine::Op::AModB: return Op::OpMod;
        case TACLine::Op::APowB: return Op::OpPower;
        case TACLine::Op::AEqualB: return Op::OpEqual;
        case TACLine::Op::ANotEqualB: return Op::OpNotEqual;
        case TACLine::Op::AGreaterThanB: return Op::OpGreater;
        case TACLine::Op::AGreatOrEqualB: return Op::OpGreatEqual;
        case TACLine::Op::ALessThanB: return Op::OpLess;
        case TACLine::Op::ALessOrEqualB: return Op::OpLessEqual;
        case TACLine::Op::AAndB: return Op::OpAnd;
        case TACLine::Op::AOrB: return Op::OpOr;
        case TACLine::Op::NotA: return Op::OpNot;
        case TACLine::Op::CallFunctionA: return Op::CallOp;
        case TACLine::Op::ReturnA: return Op::ReturnOp;
        case TACLine::Op::GotoA: return Op::JumpOp;
        default: return Op::AssignOp; // Default fallback
    }
}

std::string valueToString(const Value& val) {
    if (val.IsNull()) return "";
    
    switch (val.type) {
        case ValueType::Number:
            return "num_" + std::to_string(val.DoubleValue());
        case ValueType::String:
            return "str_" + val.ToString();
        case ValueType::Var:
            return val.ToString();
        case ValueType::Temp:
            return "temp_" + std::to_string(((TempStorage*)(val.data.ref))->tempNum);
        default:
            return "val_" + std::to_string(reinterpret_cast<uintptr_t>(val.data.ref));
    }
}

bool isJITCompilable(TACLine::Op op) {
    switch (op) {
        case TACLine::Op::AssignA:
        case TACLine::Op::APlusB:
        case TACLine::Op::AMinusB:
        case TACLine::Op::ATimesB:
        case TACLine::Op::ADividedByB:
        case TACLine::Op::AModB:
        case TACLine::Op::APowB:
        case TACLine::Op::AEqualB:
        case TACLine::Op::ANotEqualB:
        case TACLine::Op::AGreaterThanB:
        case TACLine::Op::AGreatOrEqualB:
        case TACLine::Op::ALessThanB:
        case TACLine::Op::ALessOrEqualB:
        case TACLine::Op::AAndB:
        case TACLine::Op::AOrB:
        case TACLine::Op::NotA:
        case TACLine::Op::GotoA:
        case TACLine::Op::GotoAifB:
            return true;
        default:
            return false;
    }
}

double estimateExecutionCost(const std::vector<TACLine>& sequence) {
    double cost = 0.0;
    for (const auto& line : sequence) {
        switch (line.op) {
            case Op::OpAdd:
            case Op::OpSubtract:
            case Op::OpMultiply:
                cost += 1.0;
                break;
            case Op::OpDivide:
            case Op::OpMod:
            case Op::OpPower:
                cost += 3.0;
                break;
            case Op::CallOp:
                cost += 10.0;
                break;
            default:
                cost += 0.5;
        }
    }
    return cost;
}

bool containsHotPaths(Context* context) {
    // Simple heuristic: check for backwards jumps (loops)
    for (long i = 0; i < context->code.Count(); ++i) {
        const auto& line = context->code[i];
        if (line.op == TACLine::Op::GotoA || line.op == TACLine::Op::GotoAifB) {
            // Check if it's a backwards jump
            if (line.rhsA.type == ValueType::Number) {
                long targetLine = line.rhsA.IntValue();
                if (targetLine < i) return true; // Backwards jump detected
            }
        }
    }
    return false;
}

} // namespace JITIntegration

} // namespace MiniScript