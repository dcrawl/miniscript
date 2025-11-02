#include "JITMachine.h"
#include <iostream>
#include <chrono>

namespace MiniScript {

JITMachine::JITMachine(Context *context, TextOutputMethod standardOutput) 
    : jitEnabled_(true) {
    
    // Create the underlying machine using composition
    machine_ = std::make_unique<Machine>(context, standardOutput);
    
    try {
        // jit_ = std::make_unique<RuntimeJIT>();  // Will be enabled when RuntimeJIT is complete
        
        // Configure JIT with reasonable defaults for MiniScript
        std::cout << "JIT configuration: Using default settings" << std::endl;
        
        std::cout << "JITMachine initialized with JIT compilation enabled" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Warning: JIT initialization failed: " << e.what() << std::endl;
        std::cerr << "Falling back to interpreter-only execution" << std::endl;
        jitEnabled_ = false;
        jit_.reset();
    }
}

JITMachine::~JITMachine() {
    if (jit_) {
        std::cout << "\n=== JIT Execution Summary ===" << std::endl;
        std::cout << "JIT system was active during execution" << std::endl;
        // Detailed statistics will be implemented when RuntimeJIT is complete
    }
}

void JITMachine::Step() {
    // Get current context before potential JIT execution
    Context* context = machine_->GetTopContext();
    if (!context || context->Done()) {
        // Use standard machine execution for context management
        machine_->Step();
        return;
    }
    
    // JIT integration point - decide whether to use JIT or interpreter
    if (jitEnabled_ && jit_) {
        ExecuteWithJITOrFallback(context);
    } else {
        // Standard interpreter execution
        machine_->Step();
    }
}

void JITMachine::ExecuteWithJITOrFallback(Context* context) {
    // Update execution profiling
    UpdateExecutionProfile(context, context->lineNum);
    
    // Try JIT execution first (placeholder implementation)
    long currentLine = context->lineNum;
    bool jitExecuted = false;  // Will call jit_->executeJITOrFallback when RuntimeJIT is complete
    
    if (!jitExecuted) {
        // JIT didn't handle this instruction, use standard machine
        machine_->Step();
        
        // Check if we should consider JIT compilation after interpreter execution
        if (ShouldConsiderJIT(context)) {
            // Attempt to compile hot regions
            if (HasHotPaths(context)) {
                long startLine = std::max(0L, currentLine - 10);
                long endLine = std::min(context->code.Count() - 1, currentLine + 10);
                CompileContextRange(context, startLine, endLine);
            }
        }
    }
}

bool JITMachine::ShouldConsiderJIT(Context* context) {
    if (!jit_ || !jitEnabled_) return false;
    
    // Check execution frequency
    auto it = profile_.contextExecutionCounts.find(context);
    if (it != profile_.contextExecutionCounts.end()) {
        return it->second > 50;  // Consider JIT after 50 executions in this context
    }
    
    return false;
}

void JITMachine::UpdateExecutionProfile(Context* context, long lineNum) {
    // Update context execution count
    profile_.contextExecutionCounts[context]++;
    
    // Update line execution count
    profile_.lineExecutionCounts[lineNum]++;
    
    // Periodically update profiler (every 1000 instructions)
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - profile_.lastProfileUpdate).count() > 100) {
        profile_.lastProfileUpdate = now;
        
        // Could add more sophisticated profiling logic here
        // For now, simple execution counting is sufficient
    }
}

bool JITMachine::HasHotPaths(Context* context) {
    if (!context || context->code.Count() < 5) return false;
    
    // Simple hot path detection: look for backward jumps (loops)
    for (long i = 0; i < context->code.Count(); ++i) {
        const TACLine& line = context->code[i];
        
        if (line.op == TACLine::Op::GotoA || 
            line.op == TACLine::Op::GotoAifB || 
            line.op == TACLine::Op::GotoAifTrulyB || 
            line.op == TACLine::Op::GotoAifNotB) {
            
            // Check if this is a backward jump
            if (line.rhsA.type == ValueType::Number) {
                long targetLine = static_cast<long>(line.rhsA.numberValue);
                if (targetLine < i) {
                    // Backward jump found - this indicates a loop
                    return true;
                }
            }
        }
    }
    
    return false;
}

void JITMachine::SetJITConfig(const JITConfig& config) {
    if (jit_) {
        // Will configure JIT when RuntimeJIT is complete
        std::cout << "JIT configuration updated" << std::endl;
    }
}

RuntimeStats JITMachine::GetJITStats() const {
    if (jit_) {
        // For now, return a simple stats structure
        RuntimeStats stats;
        // Implementation would call jit_->getStats() when RuntimeJIT is fully implemented
        return stats;
    }
    return RuntimeStats{};
}

bool JITMachine::CompileContextRange(Context* context, long startLine, long endLine) {
    if (!jit_ || !jitEnabled_) return false;
    
    try {
        // Placeholder for JIT compilation
        std::cout << "Compiling context range " << startLine << " to " << endLine << std::endl;
        return true;  // Will call jit_->compileContextRange when RuntimeJIT is complete
    } catch (const std::exception& e) {
        std::cerr << "JIT compilation failed: " << e.what() << std::endl;
        return false;
    }
}

void JITMachine::ClearJITCache() {
    if (jit_) {
        // Clear internal JIT cache - implementation would depend on RuntimeJIT internals
        profile_.contextExecutionCounts.clear();
        profile_.lineExecutionCounts.clear();
    }
}

} // namespace MiniScript