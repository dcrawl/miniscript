#ifndef JITMACHINE_H
#define JITMACHINE_H

#include "MiniScript/MiniscriptTAC.h"
#include "RuntimeJIT.h"
#include <memory>
#include <unordered_map>
#include <chrono>

namespace MiniScript {

/// JIT-enhanced Machine wrapper that provides seamless JIT compilation integration
/// with the existing MiniScript interpreter execution flow using composition
class JITMachine {
public:
    JITMachine(Context *context, TextOutputMethod standardOutput);
    ~JITMachine();

    /// Enhanced Step() method with JIT integration
    void Step();
    
    /// Machine interface delegation
    bool Done() { return machine_->Done(); }
    void Stop() { machine_->Stop(); }
    void Reset() { machine_->Reset(); }
    void ManuallyPushCall(FunctionStorage* func, Value resultStorage = Value::null) {
        machine_->ManuallyPushCall(func, resultStorage);
    }
    
    Context* GetGlobalContext() { return machine_->GetGlobalContext(); }
    Context* GetTopContext() { return machine_->GetTopContext(); }
    String FindShortName(const Value& val) { return machine_->FindShortName(val); }
    double RunTime() { return machine_->RunTime(); }
    List<SourceLoc> GetStack() { return machine_->GetStack(); }
    
    /// Direct access to underlying machine for compatibility
    Machine* GetMachine() { return machine_.get(); }
    
    /// Enable/disable JIT compilation
    void SetJITEnabled(bool enabled) { jitEnabled_ = enabled; }
    bool IsJITEnabled() const { return jitEnabled_; }
    
    /// JIT configuration and statistics
    void SetJITConfig(const JITConfig& config);
    RuntimeStats GetJITStats() const;
    
    /// Force compilation of a specific context range
    bool CompileContextRange(Context* context, long startLine, long endLine);
    
    /// Clear JIT compilation cache
    void ClearJITCache();

protected:
    /// Determine if current execution context should be JIT compiled
    bool ShouldConsiderJIT(Context* context);
    
    /// Execute with JIT or fallback to interpreter
    void ExecuteWithJITOrFallback(Context* context);

private:
    std::unique_ptr<Machine> machine_;  // Composition instead of inheritance
    std::unique_ptr<RuntimeJIT> jit_;
    bool jitEnabled_;
    
    // JIT execution profiling
    struct ExecutionProfile {
        std::unordered_map<Context*, size_t> contextExecutionCounts;
        std::unordered_map<long, size_t> lineExecutionCounts;
        std::chrono::steady_clock::time_point lastProfileUpdate;
    };
    
    ExecutionProfile profile_;
    
    /// Update execution profiling data
    void UpdateExecutionProfile(Context* context, long lineNum);
    
    /// Check if context has hot paths worth JIT compiling
    bool HasHotPaths(Context* context);
};

} // namespace MiniScript

#endif // JITMACHINE_H