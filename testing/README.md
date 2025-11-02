# Testing Directory

This directory contains all development, testing, and analysis files organized by purpose.

## Directory Structure

### `/jit-development/`

Core JIT development files and experimental components:

- `AdvancedJIT.cpp` - Advanced JIT compilation experiments
- `ExpressionJIT.cpp` - Expression-level JIT compilation
- `MiniScriptJIT.cpp` - Main JIT compiler implementation
- `SimpleExpressionJIT.cpp` - Simplified JIT for expressions
- `*Profiler.cpp` - Various performance profilers
- `memory_pool_test.cpp` - Memory management experiments

### `/performance-analysis/`

Documentation and analysis of performance optimizations:

- `PERFORMANCE_ANALYSIS.md` - Main performance analysis document
- `Phase*.md` - Development phase completion reports
- `MAP_OPTIMIZATION_ANALYSIS.md` - Hash map optimization analysis
- `JIT-*.md` - JIT compilation strategy documents
- `TAC_Operation_Categories.md` - TAC instruction analysis

### `/miniscript-programs/`

Test MiniScript programs used for validation and benchmarking:

- `*_test.ms` - Unit tests for specific features
- `benchmark.ms` - Performance benchmark programs
- `env_*.ms` - Environment and system tests
- `realworld_*.ms` - Real-world application simulations

### `/validation/`

Comprehensive validation and testing frameworks:

- `AccuracyAndPerformanceTest.cpp` - Main validation suite
- `JITProjectionTest.cpp` - JIT performance projections
- `Phase3_*.cpp` - JIT development phase tests
- `VALIDATION_REPORT.md` - Complete validation results

## Usage

### Running Validation Tests

```bash
# Build validation targets
make accuracy-performance-test
make jit-projection-test

# Run accuracy and performance validation
./accuracy-performance-test

# Run JIT performance projections
./jit-projection-test
```

### Running JIT Development Tests

```bash
# Build JIT development targets
make runtime-integration-test
make production-jit-test

# Test runtime integration
./runtime-integration-test

# Test production interpreter integration  
./production-jit-test
```

### Performance Analysis

The `/performance-analysis/` directory contains comprehensive documentation of:

- Performance optimization strategies
- JIT compilation approach and results
- Phase-by-phase development progress
- Benchmark results and analysis

### Test Programs

The `/miniscript-programs/` directory contains MiniScript test cases:

- Unit tests for language features
- Performance benchmarks
- Real-world application examples
- Environment validation scripts

## Development Workflow

1. **Development**: Create new JIT components in `/jit-development/`
2. **Testing**: Add validation tests to `/validation/`
3. **Benchmarking**: Create test programs in `/miniscript-programs/`
4. **Analysis**: Document results in `/performance-analysis/`

This organization keeps the main project directory clean while preserving all development artifacts and making them easily accessible for future reference.
