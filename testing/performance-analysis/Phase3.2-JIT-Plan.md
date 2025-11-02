# Phase 3.2: Full LLVM JIT Integration Plan

Based on Phase 3.1A success (3.39x-6.14x speedups) and modern LLVM research, here is the comprehensive plan for full JIT integration into MiniScript.

## Executive Summary

**Objective**: Integrate LLVM ORC-based JIT compilation into MiniScript interpreter to achieve 3-6x performance improvements for arithmetic-intensive code while maintaining compatibility and correctness.

**Key Innovation**: Selective JIT compilation using runtime profiling to identify hot expressions combined with modern LLVM ORC v2 APIs for efficient code generation.

**Success Criteria**: 
- 50%+ performance improvement on arithmetic-heavy MiniScript programs
- Seamless fallback to interpreter for unsupported constructs
- Compilation overhead under 10ms for typical expressions
- Zero correctness regressions

## Architecture Overview

### Core Components

1. **Expression Profiler**: Runtime hotness detection and compilation decision engine
2. **LLVM IR Generator**: Converts MiniScript TAC to optimized LLVM IR
3. **JIT Runtime Manager**: Manages compiled code cache and execution
4. **Integration Layer**: Hooks into existing MiniScript interpreter

### Modern LLVM Stack (Based on Research)

- **LLJIT**: Primary JIT engine (replaces legacy MCJIT)
- **ORC v2**: On-Request Compilation framework for layered compilation
- **ExecutionSession**: Manages JIT compilation sessions and symbol resolution
- **IRCompileLayer + IRTransformLayer**: Handle IR optimization and machine code generation
- **ThreadSafeModule**: Thread-safe IR management for concurrent compilation

## Phase 3.2 Implementation Plan

### Phase 3.2A: LLVM Integration Foundation (Week 1)

**Objectives**: 
- Set up modern LLVM ORC v2 infrastructure
- Create basic JIT framework integrated with MiniScript

**Tasks**:
1. **Update CMakeLists.txt** with modern LLVM ORC components
   ```cmake
   # Required LLVM components for ORC v2
   LLVM_LINK_COMPONENTS
     Core
     ExecutionEngine
     OrcJIT
     Support
     Target
     native
   ```

2. **Create MiniScriptJIT class** using LLJIT
   ```cpp
   class MiniScriptJIT {
   private:
     std::unique_ptr<llvm::orc::LLJIT> JIT;
     llvm::orc::MangleAndInterner Mangle;
     
   public:
     MiniScriptJIT();
     llvm::Error compileExpression(const TACExpression& expr);
     CompiledFunction* lookupFunction(const std::string& name);
   };
   ```

3. **Basic LLVM IR generation** for simple arithmetic
   - Convert MiniScript `Value` operations to LLVM IR
   - Handle double arithmetic (`+`, `-`, `*`, `/`, `^`)
   - Support variable access and constant loading

**Deliverables**:
- Working LLJIT integration that can compile `a + b * 2.5`
- Basic error handling and fallback to interpreter
- Simple benchmark showing JIT vs interpreter timing

### Phase 3.2B: Expression Profiler (Week 2)

**Objectives**:
- Build runtime profiler to identify compilation candidates
- Implement smart compilation thresholds

**Research-Based Design**:
Based on LLVM documentation, modern JIT systems use:
- **Lazy compilation**: Only compile when execution frequency justifies overhead
- **Tiered compilation**: Start with interpreter, upgrade to JIT for hot code
- **Profile-guided optimization**: Use runtime data to guide compilation decisions

**Tasks**:
1. **Hotness Detection System**
   ```cpp
   class ExpressionProfiler {
   private:
     std::unordered_map<ExpressionID, ProfileData> hotness_map;
     
   public:
     bool shouldCompile(const TACExpression& expr);
     void recordExecution(ExpressionID id, double execution_time);
     CompilationPriority getPriority(const TACExpression& expr);
   };
   ```

2. **Compilation Thresholds**
   - **Frequency threshold**: Compile after 1000+ executions
   - **Time threshold**: Compile expressions taking >1ms to interpret
   - **Complexity threshold**: Prioritize expressions with 5+ operations

3. **Expression Fingerprinting**
   - Create stable IDs for expression patterns
   - Handle variable substitution and constant folding
   - Cache compiled versions across similar expressions

**Deliverables**:
- Runtime profiler integrated into interpreter loop
- Threshold tuning system with configurable parameters
- Metrics showing compilation candidate identification accuracy

### Phase 3.2C: Advanced IR Generation (Week 3)

**Objectives**:
- Implement comprehensive TAC-to-LLVM-IR conversion
- Add optimization passes for maximum performance

**Modern LLVM Approach**:
Research shows LLVM ORC v2 uses layered compilation:
- **IRTransformLayer**: Apply optimizations during compilation
- **Concurrent compilation**: Background JIT compilation while interpreting
- **Symbol resolution**: Handle MiniScript built-in functions and variables

**Tasks**:
1. **Complete TAC Translation**
   ```cpp
   class TACToLLVMConverter {
   public:
     llvm::Expected<llvm::orc::ThreadSafeModule> 
     convertExpression(const TACExpression& expr);
     
   private:
     llvm::Value* generateTACLine(const TACLine& line);
     llvm::Value* handleBuiltinCall(const std::string& func);
   };
   ```

2. **Optimization Pipeline**
   ```cpp
   // Based on LLVM research - modern optimization approach
   static llvm::Expected<llvm::orc::ThreadSafeModule>
   optimizeModule(llvm::orc::ThreadSafeModule M, 
                  const llvm::orc::MaterializationResponsibility &R) {
     auto FPM = std::make_unique<llvm::legacy::FunctionPassManager>(M.get());
     FPM->add(createInstructionCombiningPass());
     FPM->add(createReassociatePass());
     FPM->add(createGVNPass());
     FPM->add(createCFGSimplificationPass());
     // ... apply optimizations
   }
   ```

3. **Type System Integration**
   - Handle MiniScript's dynamic typing with LLVM's static types
   - Implement runtime type checking where needed
   - Optimize for known-type fast paths

**Deliverables**:
- Complete arithmetic expression compiler with optimizations
- Support for MiniScript built-in math functions
- Type-specialized compilation paths for performance

### Phase 3.2D: Runtime Integration (Week 4)

**Objectives**:
- Integrate JIT compilation into MiniScript execution flow
- Implement seamless fallback and error handling

**Integration Strategy** (Based on ORC v2 patterns):
```cpp
// In MiniScript interpreter main loop
Value Interpreter::executeExpression(const TACExpression& expr) {
  // Check if JIT compilation is beneficial
  if (profiler->shouldCompile(expr)) {
    // Attempt JIT compilation
    if (auto compiled_func = jit->compileExpression(expr)) {
      return executeCompiledExpression(*compiled_func, expr.variables);
    }
  }
  
  // Fallback to interpreter
  return interpretExpression(expr);
}
```

**Tasks**:
1. **Execution Engine Modification**
   - Hook JIT checks into TAC execution loop
   - Implement compiled function calling conventions
   - Handle MiniScript `Value` marshaling to/from LLVM types

2. **Cache Management**
   ```cpp
   class JITCache {
     std::unordered_map<ExpressionID, CompiledFunction> cache;
     std::mutex cache_mutex;
     
   public:
     CompiledFunction* lookup(ExpressionID id);
     void store(ExpressionID id, CompiledFunction func);
     void evictLRU();  // Memory pressure handling
   };
   ```

3. **Error Recovery**
   - Graceful fallback on compilation failures
   - Debugging support for JIT-compiled code (LLVM research shows GDB integration)
   - Performance monitoring and adaptive thresholds

**Deliverables**:
- Fully integrated JIT system with runtime compilation
- Comprehensive error handling and fallback mechanisms
- Cache management with memory pressure handling

## Technical Considerations

### Modern LLVM Best Practices (Research-Based)

1. **Use LLJIT over MCJIT**: Research shows MCJIT is deprecated, LLJIT is the modern approach
   ```cpp
   auto JIT = llvm::orc::LLJITBuilder().create();
   ```

2. **ThreadSafeModule Management**: Essential for concurrent compilation
   ```cpp
   llvm::orc::ThreadSafeContext TSCtx(std::make_unique<llvm::LLVMContext>());
   llvm::orc::ThreadSafeModule TSM(std::move(M), TSCtx);
   ```

3. **Symbol Resolution**: Handle external function calls properly
   ```cpp
   JIT->getMainJITDylib().addGenerator(
     cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
       DL.getGlobalPrefix())));
   ```

### Performance Optimization Strategy

1. **Compilation Overhead Minimization**
   - Background compilation while interpreting
   - Incremental compilation for expression variants
   - Lazy symbol resolution

2. **Memory Management**
   - JIT code cache with LRU eviction
   - Compilation artifact cleanup
   - Memory mapping optimization for code pages

3. **Runtime Profiling**
   - Low-overhead execution counting
   - Adaptive threshold adjustment
   - Performance regression detection

## Success Metrics

### Performance Targets
- **3-6x speedup** for arithmetic-heavy expressions (based on Phase 3.1A results)
- **<10ms compilation time** for typical expressions
- **<5% memory overhead** for JIT infrastructure
- **Zero correctness regressions** across test suite

### Quality Metrics
- **95%+ compilation success rate** for supported expressions
- **Seamless fallback** on compilation failures
- **Stable performance** across different program patterns
- **Maintainable code** with comprehensive testing

## Risk Mitigation

### Technical Risks
1. **LLVM API Changes**: Use stable ORC v2 APIs, avoid deprecated interfaces
2. **Type System Complexity**: Implement gradual typing with runtime checks
3. **Memory Usage**: Implement aggressive cache management and monitoring

### Integration Risks
1. **Correctness**: Extensive testing with existing MiniScript test suite
2. **Performance Regression**: Benchmark-driven development with fallback paths
3. **Complexity**: Modular design with clear separation of concerns

## Next Steps

1. **Start Phase 3.2A**: Set up modern LLVM ORC v2 infrastructure
2. **Validate Integration**: Ensure basic JIT compilation works with MiniScript
3. **Iterate on Performance**: Use benchmarks to guide optimization decisions
4. **Maintain Compatibility**: Preserve existing MiniScript behavior and APIs

This plan leverages modern LLVM ORC v2 capabilities while building on the proven performance gains from Phase 3.1A to deliver a production-ready JIT system for MiniScript.