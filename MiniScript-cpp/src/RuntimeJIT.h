#pragma once

#include "AdvancedIRGenerator.h"
#include "../MiniScript/MiniscriptTAC.h"
#include "../MiniScript/MiniscriptInterpreter.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <atomic>
#include <mutex>

namespace MiniScript {

/// Runtime JIT Integration for MiniScript Interpreter
/// This class integrates JIT compilation directly into the MiniScript execution pipeline
class RuntimeJIT {
public:
    RuntimeJIT();
    ~RuntimeJIT();
    
    /// Initialize JIT system for the given interpreter
    void initialize(Interpreter* interpreter);
    
    /// Check if JIT compilation should be attempted for the current execution context
    bool shouldCompileContext(Context* context);
    
    /// Attempt to JIT compile a sequence of TAC instructions
    bool compileContextRange(Context* context, long startLine, long endLine);
    
    /// Execute JIT compiled code if available, fallback to interpreter otherwise
    bool executeJITOrFallback(Context* context, long& currentLine);
    
    /// Performance monitoring and statistics
    struct RuntimeStats {
        std::atomic<size_t> totalInstructions{0};
        std::atomic<size_t> jitCompiledInstructions{0};
        std::atomic<size_t> jitExecutions{0};
        std::atomic<size_t> interpreterExecutions{0};
        std::atomic<double> jitCompilationTime{0.0};
        std::atomic<double> jitExecutionTime{0.0};
        std::atomic<double> interpreterExecutionTime{0.0};
    };
    
    RuntimeStats getStats() const {
        RuntimeStats stats;
        stats.totalInstructions = totalInstructions.load();
        stats.jitExecutions = jitExecutions.load();
        stats.interpreterFallbacks = interpreterFallbacks.load();
        stats.compiledRegions = compiledRegions.load();
        return stats;
    }
    void resetStats();
    
    /// Configuration for JIT behavior
    struct JITConfig {
        size_t compilationThreshold = 100;      // Execute N times before JIT compilation
        size_t maxInstructionSequence = 50;     // Maximum TAC sequence to compile
        bool enableInlining = true;             // Enable function inlining
        bool enableOptimizations = true;        // Enable LLVM optimizations
        bool enableProfilingGuided = true;      // Enable profile-guided optimizations
        bool fallbackOnFailure = true;         // Fallback to interpreter on JIT failure
    };
    
    void updateConfig(const JITConfig& config) { 
        std::lock_guard<std::mutex> lock(mutex_);
        config_ = config; 
    }
    
private:
    // Core JIT infrastructure
    std::unique_ptr<llvm::LLVMContext> llvmContext_;
    std::unique_ptr<llvm::Module> llvmModule_;
    std::unique_ptr<MiniScriptJIT::AdvancedIRGenerator> irGenerator_;
    
    // Execution tracking and profiling
    struct ExecutionProfile {
        size_t executionCount = 0;
        double averageExecutionTime = 0.0;
        bool isHot = false;
        llvm::Function* compiledFunction = nullptr;
        std::vector<TACLine> cachedTAC;
    };
    
    std::unordered_map<std::string, ExecutionProfile> profiles_;
    std::unordered_map<Context*, std::unordered_map<long, size_t>> contextExecutionCounts_;
    
    // Compiled function cache
    struct CompiledRegion {
        llvm::Function* function;
        long startLine;
        long endLine;
        std::string fingerprint;
        double compilationTime;
        size_t executionCount = 0;
    };
    
    std::unordered_map<std::string, CompiledRegion> compiledRegions_;
    
    // Thread safety
    mutable std::mutex mutex_;
    
    // Configuration
    JITConfig config_;
    RuntimeStats stats_;
    Interpreter* interpreter_;
    
    // Helper methods
    std::string generateContextFingerprint(Context* context, long startLine, long endLine);
    std::vector<TACLine> extractTACSequence(Context* context, long startLine, long endLine);
    TACLine convertToStandardTAC(const MiniScript::TACLine& msLine);
    bool isCompilableSequence(const std::vector<MiniScript::TACLine>& sequence);
    void updateExecutionProfile(const std::string& fingerprint, double executionTime);
    bool executeCompiledFunction(llvm::Function* function, Context* context);
    void cleanupCompiledRegions();
    
    // Performance measurement
    class Timer {
    public:
        Timer() : start_(std::chrono::high_resolution_clock::now()) {}
        double elapsed() const {
            auto end = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<double, std::micro>(end - start_).count();
        }
    private:
        std::chrono::high_resolution_clock::time_point start_;
    };
};

/// Enhanced Machine class with integrated JIT compilation
class JITMachine : public Machine {
public:
    JITMachine(Context* context, TextOutputMethod standardOutput);
    ~JITMachine();
    
    /// Override Step() to include JIT compilation decision logic
    void Step();
    
    /// Enhanced DoOneLine with JIT execution path
    void DoOneLine(TACLine& line, Context* context);
    
    /// Get JIT runtime statistics
    RuntimeJIT::RuntimeStats getJITStats() const;
    
    /// Configure JIT behavior
    void configureJIT(const RuntimeJIT::JITConfig& config);
    
    /// Enable/disable JIT compilation
    void setJITEnabled(bool enabled) { jitEnabled_ = enabled; }
    bool isJITEnabled() const { return jitEnabled_; }

private:
    std::unique_ptr<RuntimeJIT> runtimeJIT_;
    bool jitEnabled_;
    
    // Track execution patterns for JIT decision making
    std::unordered_map<long, size_t> instructionCounts_;
    
    void updateInstructionStats(long lineNumber);
    bool shouldAttemptJIT(Context* context, long lineNumber);
};

/// Factory function to create JIT-enabled interpreter
std::unique_ptr<Interpreter> createJITInterpreter();

/// Integration utilities
namespace JITIntegration {
    /// Convert MiniScript TACLine::Op to our simplified Op enum
    Op convertOperation(TACLine::Op msOp);
    
    /// Convert MiniScript Value to string representation for JIT
    std::string valueToString(const Value& val);
    
    /// Check if a TAC operation is suitable for JIT compilation
    bool isJITCompilable(TACLine::Op op);
    
    /// Estimate execution cost of a TAC sequence
    double estimateExecutionCost(const std::vector<TACLine>& sequence);
    
    /// Check if a context contains loops or hot paths
    bool containsHotPaths(Context* context);
}

} // namespace MiniScript