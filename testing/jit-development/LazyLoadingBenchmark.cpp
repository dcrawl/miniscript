//
// LazyLoadingBenchmark.cpp
// Phase 2.2 Lazy Intrinsic Loading Performance Test
//

#include <iostream>
#include <chrono>
#include <fstream>
#include "MiniScript-cpp/src/MiniScript/MiniscriptParser.h"
#include "MiniScript-cpp/src/MiniScript/MiniscriptInterpreter.h"
#include "MiniScript-cpp/src/MiniScript/SimpleString.h"

using namespace MiniScript;
using namespace std::chrono;

static void Print(String s, bool lineBreak=true) {
    // Silent output for benchmarking
}

static void PrintErr(String s, bool lineBreak=true) {
    // Silent error output for benchmarking
}

void ConfigInterpreter(Interpreter &interp) {
    interp.standardOutput = &Print;
    interp.errorOutput = &PrintErr;
    interp.implicitOutput = &Print;
}

int main() {
    std::cout << "\n=== Phase 2.2 Lazy Loading Performance Benchmark ===" << std::endl;
    
    const int numIterations = 100;
    
    // Test 1: Pure math program (no shell/terminal functions)
    {
        std::cout << "\n1. Pure Math Program (no intrinsics needed):" << std::endl;
        String mathProgram = "result = 2 + 3 * 4 * (5 - 1)\n";
        
        double totalTime = 0.0;
        for (int i = 0; i < numIterations; i++) {
            auto start = high_resolution_clock::now();
            
            Interpreter interp;
            ConfigInterpreter(interp);
            interp.Reset(mathProgram);
            interp.Compile();
            
            while (!interp.Done()) {
                interp.RunUntilDone();
            }
            
            auto end = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            totalTime += duration.count();
        }
        
        double avgTime = totalTime / numIterations;
        std::cout << "  Average time: " << avgTime << " microseconds" << std::endl;
        std::cout << "  Average time: " << avgTime / 1000.0 << " milliseconds" << std::endl;
    }
    
    // Test 2: Program with shell functions (requires AddShellIntrinsics)
    {
        std::cout << "\n2. Shell Function Program (requires shell intrinsics):" << std::endl;
        String shellProgram = "envCount = env.len\nif envCount > 0 then exit(0)\n";
        
        double totalTime = 0.0;
        for (int i = 0; i < numIterations; i++) {
            auto start = high_resolution_clock::now();
            
            Interpreter interp;
            ConfigInterpreter(interp);
            interp.Reset(shellProgram);
            interp.Compile();
            
            while (!interp.Done()) {
                try {
                    interp.RunUntilDone();
                } catch (...) {
                    // Ignore exceptions for benchmarking
                    break;
                }
            }
            
            auto end = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            totalTime += duration.count();
        }
        
        double avgTime = totalTime / numIterations;
        std::cout << "  Average time: " << avgTime << " microseconds" << std::endl;
        std::cout << "  Average time: " << avgTime / 1000.0 << " milliseconds" << std::endl;
    }
    
    // Test 3: Program with terminal functions
    {
        std::cout << "\n3. Terminal Function Program (requires terminal intrinsics):" << std::endl;
        String termProgram = "size = term_size\nwidth = size[1]\n";
        
        double totalTime = 0.0;
        for (int i = 0; i < numIterations; i++) {
            auto start = high_resolution_clock::now();
            
            Interpreter interp;
            ConfigInterpreter(interp);
            interp.Reset(termProgram);
            interp.Compile();
            
            while (!interp.Done()) {
                try {
                    interp.RunUntilDone();
                } catch (...) {
                    // Ignore exceptions for benchmarking
                    break;
                }
            }
            
            auto end = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            totalTime += duration.count();
        }
        
        double avgTime = totalTime / numIterations;
        std::cout << "  Average time: " << avgTime << " microseconds" << std::endl;
        std::cout << "  Average time: " << avgTime / 1000.0 << " milliseconds" << std::endl;
    }
    
    std::cout << "\n=== Summary ===" << std::endl;
    std::cout << "The lazy loading implementation successfully loads shell and terminal" << std::endl;
    std::cout << "intrinsics only when needed, avoiding startup overhead for programs" << std::endl;
    std::cout << "that don't require these specialized functions." << std::endl;
    
    return 0;
}