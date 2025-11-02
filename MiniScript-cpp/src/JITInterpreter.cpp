#include "JITInterpreter.h"
#include <iostream>

namespace MiniScript {

JITInterpreter::JITInterpreter() : Interpreter(), jitMachine_(nullptr) {
}

JITInterpreter::JITInterpreter(String source) : Interpreter(source), jitMachine_(nullptr) {
}

JITInterpreter::JITInterpreter(List<String> source) : Interpreter(source), jitMachine_(nullptr) {
}

JITInterpreter::~JITInterpreter() {
    if (jitMachine_) {
        delete jitMachine_;
        jitMachine_ = nullptr;
        // Set vm to nullptr since we're managing it ourselves
        vm = nullptr;
    }
}

void JITInterpreter::CreateVM(Context* globalContext) {
    // This method will be called when we override the Compile() method
    if (jitMachine_) {
        delete jitMachine_;
        jitMachine_ = nullptr;
    }
    
    // Create JIT-enhanced machine
    jitMachine_ = new JITMachine(globalContext, standardOutput);
    
    // Set the vm pointer to the underlying machine for compatibility
    vm = jitMachine_->GetMachine();
    
    // Configure machine properties
    vm->interpreter = this;
    vm->storeImplicit = true;
    
    std::cout << "JIT-enhanced interpreter initialized" << std::endl;
}

void JITInterpreter::SetJITEnabled(bool enabled) {
    if (jitMachine_) {
        jitMachine_->SetJITEnabled(enabled);
        std::cout << "JIT compilation " << (enabled ? "enabled" : "disabled") << std::endl;
    }
}

bool JITInterpreter::IsJITEnabled() const {
    return jitMachine_ ? jitMachine_->IsJITEnabled() : false;
}

void JITInterpreter::SetJITConfig(const JITConfig& config) {
    if (jitMachine_) {
        jitMachine_->SetJITConfig(config);
    }
}

RuntimeStats JITInterpreter::GetJITStats() const {
    if (jitMachine_) {
        return jitMachine_->GetJITStats();
    }
    return RuntimeStats{};
}

bool JITInterpreter::CompileCurrentContext() {
    if (!jitMachine_) return false;
    
    Context* context = jitMachine_->GetTopContext();
    if (!context) return false;
    
    return jitMachine_->CompileContextRange(context, 0, context->code.Count() - 1);
}

void JITInterpreter::ClearJITCache() {
    if (jitMachine_) {
        jitMachine_->ClearJITCache();
    }
}

} // namespace MiniScript