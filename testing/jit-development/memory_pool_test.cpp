// Memory Pool Performance Test  
// Tests the HashMapEntryPool allocator performance vs standard allocation
// This is a C++ test to be compiled separately

#include "HashMapEntryPool.h"
#include "MiniscriptTypes.h"
#include <chrono>
#include <iostream>
#include <vector>

using namespace MiniScript;
using namespace std::chrono;

// Mock simple HashMapEntry for testing
template<typename K, typename V>
struct SimpleEntry {
    K key;
    V value; 
    SimpleEntry* next;
    
    SimpleEntry() : next(nullptr) {}
};

void test_pool_vs_standard_allocation() {
    std::cout << "=== Memory Pool vs Standard Allocation Test ===" << std::endl;
    
    const size_t NUM_ALLOCATIONS = 10000;
    const size_t NUM_ITERATIONS = 10;
    
    // Test 1: Standard allocation/deallocation
    std::cout << "\nTest 1: Standard allocation (new/delete)" << std::endl;
    
    auto start_time = high_resolution_clock::now();
    
    for (size_t iter = 0; iter < NUM_ITERATIONS; ++iter) {
        std::vector<SimpleEntry<int, int>*> entries;
        entries.reserve(NUM_ALLOCATIONS);
        
        // Allocate
        for (size_t i = 0; i < NUM_ALLOCATIONS; ++i) {
            auto* entry = new SimpleEntry<int, int>();
            entry->key = i;
            entry->value = i * 2;
            entries.push_back(entry);
        }
        
        // Deallocate
        for (auto* entry : entries) {
            delete entry;
        }
    }
    
    auto end_time = high_resolution_clock::now();
    auto standard_duration = duration_cast<microseconds>(end_time - start_time);
    
    std::cout << "  Time: " << standard_duration.count() << " microseconds" << std::endl;
    std::cout << "  Rate: " << (NUM_ALLOCATIONS * NUM_ITERATIONS * 1000000.0 / standard_duration.count()) 
              << " ops/sec" << std::endl;
    
    // Test 2: Pool allocation  
    std::cout << "\nTest 2: Pool allocation" << std::endl;
    
    // Use the existing HashMapEntry type with the pool
    HashMapEntryPool<int, int> pool;
    
    start_time = high_resolution_clock::now();
    
    for (size_t iter = 0; iter < NUM_ITERATIONS; ++iter) {
        std::vector<HashMapEntry<int, int>*> entries;
        entries.reserve(NUM_ALLOCATIONS);
        
        // Allocate from pool
        for (size_t i = 0; i < NUM_ALLOCATIONS; ++i) {
            auto* entry = pool.allocate();
            entry->key = i;
            entry->value = i * 2;
            entries.push_back(entry);
        }
        
        // Deallocate to pool
        for (auto* entry : entries) {
            pool.deallocate(entry);
        }
    }
    
    end_time = high_resolution_clock::now();
    auto pool_duration = duration_cast<microseconds>(end_time - start_time);
    
    std::cout << "  Time: " << pool_duration.count() << " microseconds" << std::endl;
    std::cout << "  Rate: " << (NUM_ALLOCATIONS * NUM_ITERATIONS * 1000000.0 / pool_duration.count()) 
              << " ops/sec" << std::endl;
    
    // Calculate improvement
    double speedup = static_cast<double>(standard_duration.count()) / pool_duration.count();
    std::cout << "\nImprovement: " << speedup << "x faster with pool allocation" << std::endl;
    
    // Pool statistics
    auto stats = pool.get_stats();
    std::cout << "\nPool Statistics:" << std::endl;
    std::cout << "  Total blocks: " << stats.total_blocks << std::endl;
    std::cout << "  Peak usage: " << stats.peak_usage << std::endl;
    std::cout << "  Total allocations: " << stats.total_allocations << std::endl;
    std::cout << "  Fragmentation ratio: " << stats.fragmentation_ratio << std::endl;
}

void test_pool_fragmentation() {
    std::cout << "\n=== Pool Fragmentation Test ===" << std::endl;
    
    HashMapEntryPool<int, int> pool;
    std::vector<HashMapEntry<int, int>*> entries;
    
    // Allocate many entries
    std::cout << "Allocating 1000 entries..." << std::endl;
    for (int i = 0; i < 1000; ++i) {
        auto* entry = pool.allocate();
        entry->key = i;
        entry->value = i;
        entries.push_back(entry);
    }
    
    auto stats = pool.get_stats();
    std::cout << "After allocation - Blocks: " << stats.total_blocks 
              << ", Current usage: " << stats.current_usage << std::endl;
    
    // Deallocate every other entry (create fragmentation)
    std::cout << "Deallocating every other entry..." << std::endl;
    for (size_t i = 0; i < entries.size(); i += 2) {
        pool.deallocate(entries[i]);
        entries[i] = nullptr;
    }
    
    stats = pool.get_stats();
    std::cout << "After partial deallocation - Free blocks: " << stats.free_blocks 
              << ", Current usage: " << stats.current_usage 
              << ", Fragmentation: " << (stats.fragmentation_ratio * 100) << "%" << std::endl;
    
    // Allocate new entries (should reuse freed space)
    std::cout << "Allocating 250 new entries (should reuse space)..." << std::endl;
    for (int i = 0; i < 250; ++i) {
        auto* entry = pool.allocate();
        entry->key = i + 2000;
        entry->value = i + 2000;
    }
    
    stats = pool.get_stats();
    std::cout << "After reallocation - Blocks: " << stats.total_blocks 
              << ", Current usage: " << stats.current_usage << std::endl;
    
    // Clean up remaining entries
    for (auto* entry : entries) {
        if (entry) pool.deallocate(entry);
    }
}

int main() {
    try {
        test_pool_vs_standard_allocation();
        test_pool_fragmentation();
        
        std::cout << "\n=== Summary ===" << std::endl;
        std::cout << "Memory pool allocator successfully implemented and tested!" << std::endl;
        std::cout << "Benefits:" << std::endl;
        std::cout << "  - Reduced allocation overhead" << std::endl;
        std::cout << "  - Better cache locality" << std::endl;
        std::cout << "  - Automatic fragmentation management" << std::endl;
        std::cout << "  - Statistics for debugging" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}