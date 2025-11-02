#include "MiniScript-cpp/src/MiniScript/ContextPool.h"
#include "MiniScript-cpp/src/MiniScript/MiniscriptInterpreter.h"
#include <iostream>
#include <chrono>

using namespace MiniScript;

int main() {
    std::cout << "Testing Context Pooling Implementation\n";
    std::cout << "=====================================\n";
    
    // Test 1: Basic pooling functionality
    std::cout << "\n1. Basic Pool Functionality:\n";
    auto& pool = ContextPool::instance();
    
    // Show initial statistics
    auto stats = pool.getStatistics();
    std::cout << "Initial - Pool size: " << stats.pool_size 
              << ", Hits: " << stats.pool_hits << ", Misses: " << stats.pool_misses << std::endl;
    
    // Acquire some contexts
    Context* ctx1 = pool.acquire();
    Context* ctx2 = pool.acquire();
    Context* ctx3 = pool.acquire();
    
    std::cout << "Acquired 3 contexts\n";
    stats = pool.getStatistics();
    std::cout << "After acquire - Pool size: " << stats.pool_size 
              << ", Hits: " << stats.pool_hits << ", Misses: " << stats.pool_misses << std::endl;
    
    // Release them back
    pool.release(ctx1);
    pool.release(ctx2);
    pool.release(ctx3);
    
    std::cout << "Released 3 contexts\n";
    stats = pool.getStatistics();
    std::cout << "After release - Pool size: " << stats.pool_size 
              << ", Hits: " << stats.pool_hits << ", Misses: " << stats.pool_misses << std::endl;
    
    // Test 2: Function-heavy code performance
    std::cout << "\n2. Performance Test - Function Heavy Code:\n";
    
    String testCode = R"(
        factorial = function(n)
            if n <= 1 then return 1
            return n * factorial(n - 1)
        end function
        
        sum = 0
        for i in range(1, 100)
            sum = sum + factorial(5)
        end for
        print sum
    )";
    
    // Time without clearing statistics (to see total allocations)
    pool.resetStatistics();
    
    auto start = std::chrono::high_resolution_clock::now();
    
    Interpreter interp;
    interp.Reset(testCode);
    interp.RunUntilDone();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Execution time: " << duration.count() << " microseconds\n";
    stats = pool.getStatistics();
    std::cout << "After function test - Pool size: " << stats.pool_size 
              << ", Hits: " << stats.pool_hits << ", Misses: " << stats.pool_misses 
              << ", Hit rate: " << stats.hit_rate << "%\n";
    
    // Test 3: Pool efficiency under stress
    std::cout << "\n3. Pool Stress Test:\n";
    pool.resetStatistics();
    
    const int iterations = 1000;
    start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        Context* ctx = pool.acquire();
        // Simulate some work by setting values
        ctx->lineNum = i;
        ctx->resultStorage = Value(i);
        pool.release(ctx);
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Pool operations (" << iterations << " acquire/release pairs): " 
              << duration.count() << " microseconds\n";
    std::cout << "Average per operation: " << (duration.count() / (double)iterations) << " μs\n";
    stats = pool.getStatistics();
    std::cout << "Final - Pool size: " << stats.pool_size 
              << ", Hits: " << stats.pool_hits << ", Misses: " << stats.pool_misses 
              << ", Hit rate: " << stats.hit_rate << "%\n";
    
    std::cout << "\nContext Pooling Test Complete!\n";
    
    return 0;
}