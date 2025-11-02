#include "MiniScript-cpp/src/MiniScript/ContextPool.h"
#include <iostream>
#include <chrono>

using namespace MiniScript;

int main() {
    std::cout << "Simple Context Pooling Test\n";
    std::cout << "===========================\n";
    
    auto& pool = ContextPool::instance();
    
    // Test 1: Basic acquire/release
    std::cout << "\n1. Basic Acquire/Release Test:\n";
    auto stats = pool.getStatistics();
    std::cout << "Initial - Pool size: " << stats.pool_size 
              << ", Created: " << stats.total_created << std::endl;
    
    // Acquire some contexts
    Context* ctx1 = pool.acquire();
    Context* ctx2 = pool.acquire();
    Context* ctx3 = pool.acquire();
    
    // Set some values to test isolation
    ctx1->lineNum = 100;
    ctx2->lineNum = 200;
    ctx3->lineNum = 300;
    
    stats = pool.getStatistics();
    std::cout << "After acquire - Pool size: " << stats.pool_size 
              << ", Created: " << stats.total_created 
              << ", Hit rate: " << stats.hit_rate << "%\n";
    
    // Release them back
    pool.release(ctx1);
    pool.release(ctx2);
    pool.release(ctx3);
    
    stats = pool.getStatistics();
    std::cout << "After release - Pool size: " << stats.pool_size 
              << ", Hit rate: " << stats.hit_rate << "%\n";
    
    // Test 2: Reuse verification
    std::cout << "\n2. Reuse Verification:\n";
    Context* ctx4 = pool.acquire();
    std::cout << "Acquired context lineNum: " << ctx4->lineNum << " (should be 0 due to reset)\n";
    
    ctx4->lineNum = 999;
    pool.release(ctx4);
    
    Context* ctx5 = pool.acquire();
    std::cout << "Re-acquired context lineNum: " << ctx5->lineNum << " (should be 0 due to reset)\n";
    pool.release(ctx5);
    
    // Test 3: Performance measurement
    std::cout << "\n3. Performance Test:\n";
    pool.resetStatistics();
    
    const int iterations = 10000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        Context* ctx = pool.acquire();
        ctx->lineNum = i;  // Simulate some work
        pool.release(ctx);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    stats = pool.getStatistics();
    std::cout << "Pool operations (" << iterations << " acquire/release pairs): " 
              << duration.count() << " microseconds\n";
    std::cout << "Average per operation: " << (duration.count() / (double)iterations) << " μs\n";
    std::cout << "Final stats - Pool size: " << stats.pool_size 
              << ", Created: " << stats.total_created 
              << ", Hit rate: " << stats.hit_rate << "%\n";
    
    std::cout << "\nSimple Context Pooling Test Complete!\n";
    
    return 0;
}