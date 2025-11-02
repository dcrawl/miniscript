cmake_minimum_required(VERSION 3.12)
project(Miniscript VERSION 1.6.2 LANGUAGES C CXX)

option(MINISCRIPT_BUILD_TESTING "Build unit test executable" OFF)
option(MINISCRIPT_BUILD_CSHARP "Build CSharp binaries" OFF)
set(MINISCRIPT_CMD_NAME "miniscript" CACHE STRING
	"Specifies the command-line MiniScript executable filename")

if(MINISCRIPT_BUILD_CSHARP)
	enable_language(CSharp)
	set(CMAKE_CSharp_FLAGS "/langversion:7.3")
	set(CMAKE_DOTNET_TARGET_FRAMEWORK "net45")
	set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.5")
	set(DOTNET_REFS "System;System.Core")
	add_library(miniscript-cs SHARED
		MiniScript-cs/Miniscript.cs
		MiniScript-cs/MiniscriptErrors.cs
		MiniScript-cs/MiniscriptInterpreter.cs
		MiniScript-cs/MiniscriptIntrinsics.cs
		MiniScript-cs/MiniscriptKeywords.cs
		MiniScript-cs/MiniscriptLexer.cs
		MiniScript-cs/MiniscriptParser.cs
		MiniScript-cs/MiniscriptTAC.cs
		MiniScript-cs/MiniscriptTypes.cs
		MiniScript-cs/MiniscriptUnitTest.cs
	)
	set_target_properties(miniscript-cs PROPERTIES
		VS_GLOBAL_ROOTNAMESPACE "Miniscript"
		VS_DOTNET_REFERENCES "${DOTNET_REFS}"
	)
endif()

set(MINISCRIPT_HEADERS
	MiniScript-cpp/src/MiniScript/Dictionary.h
	MiniScript-cpp/src/MiniScript/HashMapEntryPool.h
	MiniScript-cpp/src/MiniScript/ImprovedHash.h
	MiniScript-cpp/src/MiniScript/List.h
	MiniScript-cpp/src/MiniScript/MiniscriptErrors.h
	MiniScript-cpp/src/MiniScript/MiniscriptInterpreter.h
	MiniScript-cpp/src/MiniScript/MiniscriptIntrinsics.h
	MiniScript-cpp/src/MiniScript/MiniscriptKeywords.h
	MiniScript-cpp/src/MiniScript/MiniscriptLexer.h
	MiniScript-cpp/src/MiniScript/MiniscriptParser.h
	MiniScript-cpp/src/MiniScript/MiniscriptTAC.h
	MiniScript-cpp/src/MiniScript/MiniscriptTypes.h
	MiniScript-cpp/src/MiniScript/OptimizedEvaluators.h
	MiniScript-cpp/src/MiniScript/QA.h
	MiniScript-cpp/src/MiniScript/RefCountedStorage.h
	MiniScript-cpp/src/MiniScript/SimpleString.h
	MiniScript-cpp/src/MiniScript/SimpleVector.h
	MiniScript-cpp/src/MiniScript/SplitJoin.h
	MiniScript-cpp/src/MiniScript/UnicodeUtil.h
	MiniScript-cpp/src/MiniScript/UnitTest.h
)

set(MINICMD_HEADERS
	MiniScript-cpp/src/DateTimeUtils.h
	MiniScript-cpp/src/Key.h
	MiniScript-cpp/src/OstreamSupport.h
	MiniScript-cpp/src/ShellExec.h
	MiniScript-cpp/src/ShellIntrinsics.h
	MiniScript-cpp/src/editline/editline.h
	MiniScript-cpp/src/editline/unix.h
	MiniScript-cpp/src/whereami/whereami.h
	MiniScript-cpp/src/TermIntrinsics.h
	MiniScript-cpp/src/TermIO.h
)

add_library(miniscript-cpp
	MiniScript-cpp/src/MiniScript/Dictionary.cpp
	MiniScript-cpp/src/MiniScript/HashMapEntryPool.cpp
	MiniScript-cpp/src/MiniScript/List.cpp
	MiniScript-cpp/src/MiniScript/MiniscriptInterpreter.cpp
	MiniScript-cpp/src/MiniScript/MiniscriptIntrinsics.cpp
	MiniScript-cpp/src/MiniScript/MiniscriptKeywords.cpp
	MiniScript-cpp/src/MiniScript/MiniscriptLexer.cpp
	MiniScript-cpp/src/MiniScript/MiniscriptParser.cpp
	MiniScript-cpp/src/MiniScript/MiniscriptTAC.cpp
	MiniScript-cpp/src/MiniScript/MiniscriptTypes.cpp
	MiniScript-cpp/src/MiniScript/OptimizedEvaluators.cpp
	MiniScript-cpp/src/MiniScript/QA.cpp
	MiniScript-cpp/src/MiniScript/SimpleString.cpp
	MiniScript-cpp/src/MiniScript/SimpleVector.cpp
	MiniScript-cpp/src/MiniScript/SplitJoin.cpp
	MiniScript-cpp/src/MiniScript/UnicodeUtil.cpp
	MiniScript-cpp/src/MiniScript/UnitTest.cpp
	${MINISCRIPT_HEADERS}
)

target_include_directories(miniscript-cpp PUBLIC MiniScript-cpp/src/MiniScript)

if(NOT WIN32)
	set(EDITLINE_SRC
		MiniScript-cpp/src/editline/complete.c
		MiniScript-cpp/src/editline/editline.c
		MiniScript-cpp/src/editline/sysunix.c
	)
endif()

add_executable(minicmd
	MiniScript-cpp/src/main.cpp
	MiniScript-cpp/src/DateTimeUtils.cpp
	MiniScript-cpp/src/Key.cpp
	MiniScript-cpp/src/OstreamSupport.cpp
	MiniScript-cpp/src/ShellIntrinsics.cpp
	MiniScript-cpp/src/ShellExec.cpp
	MiniScript-cpp/src/whereami/whereami.c
	MiniScript-cpp/src/TermIntrinsics.cpp
	MiniScript-cpp/src/TermIO.cpp
	${EDITLINE_SRC}
	${MINICMD_HEADERS}
)
target_include_directories(minicmd PRIVATE MiniScript-cpp/src/editline)
target_link_libraries(minicmd PRIVATE miniscript-cpp)

set_target_properties(miniscript-cpp minicmd PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)
set_target_properties(minicmd PROPERTIES OUTPUT_NAME ${MINISCRIPT_CMD_NAME})

if(MINISCRIPT_BUILD_TESTING)
	enable_testing()
	add_custom_target(TestSuite SOURCES TestSuite.txt)
	add_executable(tests-cpp MiniScript-cpp/src/MiniScript/UnitTest.cpp)
	target_compile_definitions(tests-cpp PRIVATE UNIT_TEST_MAIN)
	target_link_libraries(tests-cpp PRIVATE miniscript-cpp)
	add_test(NAME Miniscript.cpp.UnitTests COMMAND tests-cpp)
	add_test(NAME Miniscript.cpp.Integration COMMAND minicmd --itest ${CMAKE_SOURCE_DIR}/TestSuite.txt)
	set_tests_properties(Miniscript.cpp.UnitTests Miniscript.cpp.Integration PROPERTIES FAIL_REGULAR_EXPRESSION "FAIL|Error")
	if(MINISCRIPT_BUILD_CSHARP)
		add_executable(tests-cs MiniScript-cs/Program.cs)
		target_link_libraries(tests-cs PRIVATE miniscript-cs)
		set_target_properties(tests-cs PROPERTIES
			VS_GLOBAL_ROOTNAMESPACE "Miniscript"
			VS_DOTNET_REFERENCES "${DOTNET_REFS}"
			CXX_STANDARD 14
			CXX_STANDARD_REQUIRED ON
		)
		add_test(NAME Miniscript.cs.UnitTests COMMAND tests-cs --test)
		add_test(NAME Miniscript.cs.Integration COMMAND tests-cs --test --integration ${CMAKE_SOURCE_DIR}/TestSuite.txt)
		set_tests_properties(Miniscript.cs.UnitTests Miniscript.cs.Integration PROPERTIES FAIL_REGULAR_EXPRESSION "FAIL|Error")
	endif()
endif()

# Phase 2.1 Performance Benchmark
add_executable(phase21-benchmark
	MiniScript-cpp/src/MiniScript/Phase2_1_Benchmark.cpp
)
target_include_directories(phase21-benchmark PRIVATE MiniScript-cpp/src/MiniScript)
target_link_libraries(phase21-benchmark PRIVATE miniscript-cpp)
set_target_properties(phase21-benchmark PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

# Phase 2.1 Fast Path Validator
add_executable(fastpath-validator
	MiniScript-cpp/src/MiniScript/FastPathValidator.cpp
)
target_include_directories(fastpath-validator PRIVATE MiniScript-cpp/src/MiniScript)
target_link_libraries(fastpath-validator PRIVATE miniscript-cpp)
set_target_properties(fastpath-validator PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

# Phase 2.1 Final Validation
add_executable(phase21-final-validation
	MiniScript-cpp/src/MiniScript/Phase21FinalValidation.cpp
)
target_include_directories(phase21-final-validation PRIVATE MiniScript-cpp/src/MiniScript)
target_link_libraries(phase21-final-validation PRIVATE miniscript-cpp)
set_target_properties(phase21-final-validation PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

# Real-World Performance Comparison
add_executable(realworld-comparison
	MiniScript-cpp/src/MiniScript/RealWorldComparison.cpp
)
target_include_directories(realworld-comparison PRIVATE MiniScript-cpp/src/MiniScript)
target_link_libraries(realworld-comparison PRIVATE miniscript-cpp)
set_target_properties(realworld-comparison PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

# Pure Evaluation Benchmark
add_executable(pure-evaluation-benchmark
	MiniScript-cpp/src/MiniScript/PureEvaluationBenchmark.cpp
)
target_include_directories(pure-evaluation-benchmark PRIVATE MiniScript-cpp/src/MiniScript)
target_link_libraries(pure-evaluation-benchmark PRIVATE miniscript-cpp)
set_target_properties(pure-evaluation-benchmark PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

# Phase 2.2 Parsing Performance Profiler
add_executable(parsing-profiler
	ParsingProfiler.cpp
)
target_include_directories(parsing-profiler PRIVATE MiniScript-cpp/src/MiniScript)
target_link_libraries(parsing-profiler PRIVATE miniscript-cpp)
set_target_properties(parsing-profiler PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

# Phase 2.2 Simple Parsing Profiler (no segfault issues)
add_executable(simple-parsing-profiler
	SimpleParsingProfiler.cpp
)
target_include_directories(simple-parsing-profiler PRIVATE MiniScript-cpp/src/MiniScript)
target_link_libraries(simple-parsing-profiler PRIVATE miniscript-cpp)
set_target_properties(simple-parsing-profiler PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

# Phase 2.2 Large Code Parsing Profiler  
add_executable(large-code-profiler
	LargeCodeParsingProfiler.cpp
)
target_include_directories(large-code-profiler PRIVATE MiniScript-cpp/src/MiniScript)
target_link_libraries(large-code-profiler PRIVATE miniscript-cpp)
set_target_properties(large-code-profiler PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

add_executable(lazy-loading-benchmark LazyLoadingBenchmark.cpp)
target_link_libraries(lazy-loading-benchmark PRIVATE miniscript-cpp)
set_target_properties(lazy-loading-benchmark PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

add_executable(operation-profiler OperationProfiler.cpp)
target_link_libraries(operation-profiler PRIVATE miniscript-cpp)
set_target_properties(operation-profiler PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

add_executable(specialized-instructions SpecializedInstructions.cpp)
target_link_libraries(specialized-instructions PRIVATE miniscript-cpp)
set_target_properties(specialized-instructions PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

# Phase 3.1A: Expression JIT Compiler
find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config PATHS /opt/homebrew/Cellar/llvm@17/17.0.6/bin NO_DEFAULT_PATH)
if(LLVM_CONFIG_EXECUTABLE)
    execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --cppflags OUTPUT_VARIABLE LLVM_CPPFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs core executionengine mcjit native OUTPUT_VARIABLE LLVM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
    
    add_executable(expression-jit ExpressionJIT.cpp)
    target_link_libraries(expression-jit PRIVATE miniscript-cpp)
    
    # Add LLVM flags
    separate_arguments(LLVM_CPPFLAGS_LIST UNIX_COMMAND "${LLVM_CPPFLAGS}")
    separate_arguments(LLVM_LDFLAGS_LIST UNIX_COMMAND "${LLVM_LDFLAGS}")
    separate_arguments(LLVM_LIBS_LIST UNIX_COMMAND "${LLVM_LIBS}")
    
    target_compile_options(expression-jit PRIVATE ${LLVM_CPPFLAGS_LIST})
    target_link_options(expression-jit PRIVATE ${LLVM_LDFLAGS_LIST})
    target_link_libraries(expression-jit PRIVATE ${LLVM_LIBS_LIST})
    
    set_target_properties(expression-jit PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)
else()
    message(WARNING "LLVM not found - Expression JIT will not be built")
endif()

# Phase 3.1A: Simple Expression JIT (no LLVM dependencies)
add_executable(simple-jit SimpleExpressionJIT.cpp)
set_target_properties(simple-jit PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON)

# Phase 3.1A: Advanced Expression JIT (optimized proof of concept)
add_executable(advanced-jit AdvancedJIT.cpp)
set_target_properties(advanced-jit PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON)

# Phase 3.2A: Modern LLVM ORC v2 JIT Integration
if(LLVM_CONFIG_EXECUTABLE)
    # Get modern LLVM ORC components
    execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs core orcjit support target native OUTPUT_VARIABLE LLVM_ORC_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
    
    add_executable(miniscript-jit MiniScriptJIT.cpp)
    target_link_libraries(miniscript-jit PRIVATE miniscript-cpp)
    
    # Add LLVM ORC flags
    separate_arguments(LLVM_ORC_LIBS_LIST UNIX_COMMAND "${LLVM_ORC_LIBS}")
    target_compile_options(miniscript-jit PRIVATE ${LLVM_CPPFLAGS_LIST})
    target_link_options(miniscript-jit PRIVATE ${LLVM_LDFLAGS_LIST})
    target_link_libraries(miniscript-jit PRIVATE ${LLVM_ORC_LIBS_LIST})
    
    set_target_properties(miniscript-jit PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)
    
    message(STATUS "Phase 3.2A Modern LLVM ORC JIT target enabled")
else()
    message(WARNING "Phase 3.2A LLVM ORC JIT disabled (LLVM not found)")
endif()

# Phase 3.2B: Expression Profiler (standalone test)
add_executable(profiler-test ProfilerTest.cpp)
set_target_properties(profiler-test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON)
message(STATUS "Phase 3.2B Expression Profiler test enabled")

# Phase 3.2C: Advanced IR Generation (comprehensive TAC-to-LLVM-IR)
if(LLVM_CONFIG_EXECUTABLE)
    add_executable(advanced-ir-test MiniScript-cpp/src/Phase3_2C_Test.cpp
                                   MiniScript-cpp/src/AdvancedIRGenerator.cpp)
    target_include_directories(advanced-ir-test PRIVATE MiniScript-cpp/src/MiniScript)
    target_link_libraries(advanced-ir-test PRIVATE miniscript-cpp)
    
    # Add LLVM flags for advanced IR generation
    target_compile_options(advanced-ir-test PRIVATE ${LLVM_CPPFLAGS_LIST})
    target_link_options(advanced-ir-test PRIVATE ${LLVM_LDFLAGS_LIST})
    target_link_libraries(advanced-ir-test PRIVATE ${LLVM_ORC_LIBS_LIST})
    
    set_target_properties(advanced-ir-test PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)
    
    message(STATUS "Phase 3.2C Advanced IR Generation target enabled")
else()
    message(WARNING "Phase 3.2C Advanced IR Generation disabled (LLVM not found)")
endif()

# Phase 3.2D: Runtime Integration (production JIT system)
if(LLVM_CONFIG_EXECUTABLE)
    add_executable(runtime-integration-test MiniScript-cpp/src/Phase3_2D_Standalone.cpp)
    target_include_directories(runtime-integration-test PRIVATE MiniScript-cpp/src/MiniScript)
    target_link_libraries(runtime-integration-test PRIVATE miniscript-cpp)
    
    # Add LLVM flags for runtime integration
    target_compile_options(runtime-integration-test PRIVATE ${LLVM_CPPFLAGS_LIST})
    target_link_options(runtime-integration-test PRIVATE ${LLVM_LDFLAGS_LIST})
    target_link_libraries(runtime-integration-test PRIVATE ${LLVM_ORC_LIBS_LIST})
    
    set_target_properties(runtime-integration-test PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)
    
    message(STATUS "Phase 3.2D Runtime Integration target enabled")
else()
    message(WARNING "LLVM not found. Phase 3.2D Runtime Integration target disabled.")
endif()

# Phase 3.3: Production Interpreter Integration  
add_executable(production-jit-test MiniScript-cpp/src/Phase3_3_Standalone.cpp)
target_include_directories(production-jit-test PRIVATE MiniScript-cpp/src/MiniScript)
target_link_libraries(production-jit-test PRIVATE miniscript-cpp)

set_target_properties(production-jit-test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON)

message(STATUS "Phase 3.3 Production Interpreter Integration target enabled")

# Accuracy and Performance Validation Suite
add_executable(accuracy-performance-test MiniScript-cpp/src/AccuracyAndPerformanceTest.cpp)
target_include_directories(accuracy-performance-test PRIVATE MiniScript-cpp/src/MiniScript)
target_link_libraries(accuracy-performance-test PRIVATE miniscript-cpp)

set_target_properties(accuracy-performance-test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON)

message(STATUS "Accuracy and Performance Validation Suite enabled")

# JIT Performance Projection Test
add_executable(jit-projection-test MiniScript-cpp/src/JITProjectionTest.cpp)
target_include_directories(jit-projection-test PRIVATE MiniScript-cpp/src/MiniScript)
target_link_libraries(jit-projection-test PRIVATE miniscript-cpp)

set_target_properties(jit-projection-test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON)

message(STATUS "JIT Performance Projection Test enabled")

message(STATUS "Phase 3.1A Expression JIT targets enabled")

install(TARGETS miniscript-cpp minicmd)
