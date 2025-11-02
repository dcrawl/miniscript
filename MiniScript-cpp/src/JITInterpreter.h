#ifndef JITINTERPRETER_H
#define JITINTERPRETER_H

#include "MiniScript/MiniscriptInterpreter.h"
#include "JITMachine.h"

namespace MiniScript {

/// JIT-enabled Interpreter that uses JITMachine for enhanced performance
/// while maintaining full compatibility with the standard MiniScript Interpreter
class JITInterpreter : public Interpreter {
public:
    /// Constructors - same interface as standard Interpreter
    JITInterpreter();
    JITInterpreter(String source);
    JITInterpreter(List<String> source);
    
    /// Destructor
    ~JITInterpreter();
    
    /// JIT-specific configuration methods
    void SetJITEnabled(bool enabled);
    bool IsJITEnabled() const;
    
    void SetJITConfig(const JITConfig& config);
    RuntimeStats GetJITStats() const;
    
    /// Force compilation of current context
    bool CompileCurrentContext();
    
    /// Clear JIT compilation cache
    void ClearJITCache();
    
    /// Access to the JIT-enabled machine (for advanced usage)
    JITMachine* GetJITMachine() { return jitMachine_; }

protected:
    /// Override the VM creation to use JITMachine
    void CreateVM(Context* globalContext);
    
private:
    JITMachine* jitMachine_;  // Our JIT-enhanced machine
};

} // namespace MiniScript

#endif // JITINTERPRETER_H