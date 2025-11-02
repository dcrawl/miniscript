/*
 * Phase 3.2B: MiniScript Expression Profiler
 * 
 * This implements intelligent profiling to identify arithmetic expressions
 * that would benefit from JIT compilation based on:
 * - Execution frequency (hotness detection) 
 * - Expression complexity (operation count)
 * - Compilation overhead analysis
 * - Adaptive threshold adjustment
 */

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <iomanip>
#include <functional>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <sstream>

// Expression profiling data
struct ExpressionProfile {
    // Execution statistics
    std::atomic<uint64_t> execution_count{0};
    std::atomic<uint64_t> total_execution_time_ns{0};
    std::atomic<uint64_t> last_execution_time{0};
    
    // Expression characteristics
    uint32_t operation_count = 0;
    uint32_t depth = 0;
    bool has_complex_ops = false;  // pow, function calls, etc.
    
    // JIT compilation status
    enum CompilationStatus {
        NOT_ANALYZED,     // Never profiled
        INTERPRETER_ONLY, // Not worth JIT compilation
        JIT_CANDIDATE,    // Should be JIT compiled
        JIT_COMPILED,     // Already compiled
        JIT_FAILED        // Compilation failed, use interpreter
    };
    
    std::atomic<CompilationStatus> status{NOT_ANALYZED};
    std::atomic<uint64_t> compilation_time_ns{0};
    
    // Performance tracking
    std::atomic<uint64_t> jit_execution_count{0};
    std::atomic<uint64_t> jit_total_time_ns{0};
    
    // Calculate average execution time
    double getAverageExecutionTime() const {
        uint64_t count = execution_count.load();
        if (count == 0) return 0.0;
        return static_cast<double>(total_execution_time_ns.load()) / count;
    }
    
    // Calculate execution frequency (executions per second)
    double getExecutionFrequency() const {
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        uint64_t last_exec = last_execution_time.load();
        if (last_exec == 0) return 0.0;
        
        uint64_t time_window = now - last_exec;
        if (time_window == 0) return 0.0;
        
        // Convert to executions per second
        return static_cast<double>(execution_count.load()) * 1e9 / time_window;
    }
};

// Expression fingerprinting for stable identification
class ExpressionFingerprinter {
public:
    // Generate a stable hash for an expression pattern
    static uint64_t fingerprint(const std::vector<SimpleTACLine>& instructions) {
        std::hash<std::string> hasher;
        std::stringstream ss;
        
        // Create canonical representation
        for (const auto& instr : instructions) {
            ss << static_cast<int>(instr.operation) << ":";
            
            // Normalize variable names to positions
            if (!instr.operandA.empty()) {
                ss << normalizeVariable(instr.operandA) << ",";
            }
            if (!instr.operandB.empty()) {
                ss << normalizeVariable(instr.operandB) << ",";
            }
            if (instr.operation == SimpleTACLine::LOAD_CONST) {
                ss << instr.constant_value << ",";
            }
            ss << ";";
        }
        
        return hasher(ss.str());
    }
    
    // Calculate expression complexity metrics
    static void analyzeComplexity(const std::vector<SimpleTACLine>& instructions,
                                 uint32_t& operation_count, uint32_t& depth, bool& has_complex_ops) {
        operation_count = instructions.size();
        depth = calculateDepth(instructions);
        has_complex_ops = false;
        
        // Check for complex operations
        for (const auto& instr : instructions) {
            if (instr.operation == SimpleTACLine::POW) {
                has_complex_ops = true;
                break;
            }
        }
    }
    
private:
    static std::string normalizeVariable(const std::string& var) {
        // Simple variable normalization - map common patterns
        static std::unordered_map<std::string, std::string> var_map;
        static int next_var_id = 0;
        
        auto it = var_map.find(var);
        if (it != var_map.end()) {
            return it->second;
        }
        
        std::string normalized = "v" + std::to_string(next_var_id++);
        var_map[var] = normalized;
        return normalized;
    }
    
    static uint32_t calculateDepth(const std::vector<SimpleTACLine>& instructions) {
        // Simple depth calculation - count max dependency chain
        std::unordered_map<std::string, uint32_t> var_depths;
        uint32_t max_depth = 0;
        
        for (const auto& instr : instructions) {
            uint32_t current_depth = 1;
            
            // Check operand depths
            if (!instr.operandA.empty()) {
                auto it = var_depths.find(instr.operandA);
                if (it != var_depths.end()) {
                    current_depth = std::max(current_depth, it->second + 1);
                }
            }
            if (!instr.operandB.empty()) {
                auto it = var_depths.find(instr.operandB);
                if (it != var_depths.end()) {
                    current_depth = std::max(current_depth, it->second + 1);
                }
            }
            
            var_depths[instr.result] = current_depth;
            max_depth = std::max(max_depth, current_depth);
        }
        
        return max_depth;
    }
};

// Adaptive compilation thresholds
class CompilationThresholds {
public:
    // Base thresholds (can be adjusted based on performance)
    uint64_t min_execution_count = 1000;        // Minimum executions before considering JIT
    double min_execution_frequency = 100.0;     // Minimum executions per second
    double min_avg_execution_time_ns = 10000.0; // Minimum 10μs average execution time
    uint32_t max_complexity = 50;               // Maximum operation count to JIT
    
    // Compilation overhead thresholds
    uint64_t max_compilation_time_ns = 50000000; // Max 50ms compilation time
    double min_speedup_factor = 1.5;            // Minimum 1.5x speedup required
    
    // Adaptive adjustment factors
    double success_rate = 0.0;                  // JIT compilation success rate
    double average_speedup = 0.0;               // Average measured speedup
    
    // Adjust thresholds based on performance history
    void adjustThresholds(double recent_success_rate, double recent_speedup) {
        const double alpha = 0.1; // Smoothing factor
        
        success_rate = alpha * recent_success_rate + (1.0 - alpha) * success_rate;
        average_speedup = alpha * recent_speedup + (1.0 - alpha) * average_speedup;
        
        // Adjust thresholds based on performance
        if (success_rate > 0.8 && average_speedup > 2.0) {
            // High success rate and good speedup - lower thresholds to compile more
            min_execution_count = std::max(static_cast<uint64_t>(500), static_cast<uint64_t>(min_execution_count * 0.9));
            min_execution_frequency = std::max(50.0, min_execution_frequency * 0.9);
        } else if (success_rate < 0.5 || average_speedup < 1.2) {
            // Low success rate or poor speedup - raise thresholds to compile less
            min_execution_count = std::min(static_cast<uint64_t>(5000), static_cast<uint64_t>(min_execution_count * 1.1));
            min_execution_frequency = std::min(500.0, min_execution_frequency * 1.1);
        }
    }
    
    void printThresholds() const {
        std::cout << "Compilation Thresholds:" << std::endl;
        std::cout << "  Min execution count: " << min_execution_count << std::endl;
        std::cout << "  Min execution frequency: " << min_execution_frequency << " Hz" << std::endl;
        std::cout << "  Min avg execution time: " << min_avg_execution_time_ns << " ns" << std::endl;
        std::cout << "  Success rate: " << (success_rate * 100.0) << "%" << std::endl;
        std::cout << "  Average speedup: " << average_speedup << "x" << std::endl;
    }
};

// Main expression profiler class
class ExpressionProfiler {
private:
    // Profile storage
    std::unordered_map<uint64_t, std::unique_ptr<ExpressionProfile>> profiles;
    mutable std::mutex profiles_mutex;
    
    // Adaptive thresholds
    CompilationThresholds thresholds;
    
    // Performance tracking
    std::atomic<uint64_t> total_profiles{0};
    std::atomic<uint64_t> jit_candidates{0};
    std::atomic<uint64_t> successful_compilations{0};
    std::atomic<uint64_t> failed_compilations{0};
    
public:
    ExpressionProfiler() {
        std::cout << "ExpressionProfiler initialized with adaptive thresholds" << std::endl;
        thresholds.printThresholds();
    }
    
    // Record execution of an expression
    void recordExecution(const SimpleExpression& expr, uint64_t execution_time_ns) {
        uint64_t fingerprint = ExpressionFingerprinter::fingerprint(expr.instructions);
        
        std::lock_guard<std::mutex> lock(profiles_mutex);
        auto profile = getOrCreateProfile(fingerprint, expr);
        
        // Update execution statistics
        profile->execution_count.fetch_add(1);
        profile->total_execution_time_ns.fetch_add(execution_time_ns);
        
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        profile->last_execution_time.store(now);
        
        // Check if this expression should be considered for JIT compilation
        if (profile->status.load() == ExpressionProfile::NOT_ANALYZED) {
            analyzeForCompilation(*profile, fingerprint);
        }
    }
    
    // Check if an expression should be JIT compiled
    bool shouldCompile(const SimpleExpression& expr) {
        uint64_t fingerprint = ExpressionFingerprinter::fingerprint(expr.instructions);
        
        std::lock_guard<std::mutex> lock(profiles_mutex);
        auto it = profiles.find(fingerprint);
        if (it == profiles.end()) {
            return false; // No profile yet
        }
        
        return it->second->status.load() == ExpressionProfile::JIT_CANDIDATE;
    }
    
    // Record JIT compilation result
    void recordCompilation(const SimpleExpression& expr, bool success, uint64_t compilation_time_ns) {
        uint64_t fingerprint = ExpressionFingerprinter::fingerprint(expr.instructions);
        
        std::lock_guard<std::mutex> lock(profiles_mutex);
        auto it = profiles.find(fingerprint);
        if (it == profiles.end()) return;
        
        auto& profile = it->second;
        profile->compilation_time_ns.store(compilation_time_ns);
        
        if (success) {
            profile->status.store(ExpressionProfile::JIT_COMPILED);
            successful_compilations.fetch_add(1);
        } else {
            profile->status.store(ExpressionProfile::JIT_FAILED);
            failed_compilations.fetch_add(1);
        }
    }
    
    // Record JIT execution performance
    void recordJITExecution(const SimpleExpression& expr, uint64_t execution_time_ns) {
        uint64_t fingerprint = ExpressionFingerprinter::fingerprint(expr.instructions);
        
        std::lock_guard<std::mutex> lock(profiles_mutex);
        auto it = profiles.find(fingerprint);
        if (it == profiles.end()) return;
        
        auto& profile = it->second;
        profile->jit_execution_count.fetch_add(1);
        profile->jit_total_time_ns.fetch_add(execution_time_ns);
    }
    
    // Get compilation priority for an expression
    enum CompilationPriority {
        VERY_LOW = 0,
        LOW = 1, 
        MEDIUM = 2,
        HIGH = 3,
        VERY_HIGH = 4
    };
    
    CompilationPriority getPriority(const SimpleExpression& expr) {
        uint64_t fingerprint = ExpressionFingerprinter::fingerprint(expr.instructions);
        
        std::lock_guard<std::mutex> lock(profiles_mutex);
        auto it = profiles.find(fingerprint);
        if (it == profiles.end()) return VERY_LOW;
        
        auto& profile = it->second;
        
        // Calculate priority based on multiple factors
        double score = 0.0;
        
        // Factor 1: Execution frequency
        double freq = profile->getExecutionFrequency();
        if (freq > 1000.0) score += 2.0;
        else if (freq > 500.0) score += 1.5;
        else if (freq > 100.0) score += 1.0;
        
        // Factor 2: Average execution time
        double avg_time = profile->getAverageExecutionTime();
        if (avg_time > 100000.0) score += 2.0;      // > 100μs
        else if (avg_time > 50000.0) score += 1.5;  // > 50μs
        else if (avg_time > 10000.0) score += 1.0;  // > 10μs
        
        // Factor 3: Total execution count
        uint64_t count = profile->execution_count.load();
        if (count > 10000) score += 2.0;
        else if (count > 5000) score += 1.5;
        else if (count > 1000) score += 1.0;
        
        // Factor 4: Expression complexity (sweet spot)
        if (profile->operation_count >= 3 && profile->operation_count <= 20) {
            score += 1.0;
        }
        if (profile->has_complex_ops) {
            score += 0.5; // Complex ops benefit more from JIT
        }
        
        // Convert score to priority
        if (score >= 6.0) return VERY_HIGH;
        if (score >= 4.0) return HIGH;
        if (score >= 2.0) return MEDIUM;
        if (score >= 1.0) return LOW;
        return VERY_LOW;
    }
    
    // Update adaptive thresholds based on recent performance
    void updateThresholds() {
        uint64_t total_attempts = successful_compilations.load() + failed_compilations.load();
        if (total_attempts == 0) return;
        
        double success_rate = static_cast<double>(successful_compilations.load()) / total_attempts;
        
        // Calculate average speedup from JIT vs interpreter performance
        double total_speedup = 0.0;
        int speedup_samples = 0;
        
        std::lock_guard<std::mutex> lock(profiles_mutex);
        for (const auto& pair : profiles) {
            const auto& profile = pair.second;
            if (profile->status.load() == ExpressionProfile::JIT_COMPILED &&
                profile->jit_execution_count.load() > 0 && profile->execution_count.load() > 0) {
                
                double interpreter_avg = profile->getAverageExecutionTime();
                double jit_avg = static_cast<double>(profile->jit_total_time_ns.load()) / 
                               profile->jit_execution_count.load();
                
                if (jit_avg > 0.0) {
                    total_speedup += interpreter_avg / jit_avg;
                    speedup_samples++;
                }
            }
        }
        
        double avg_speedup = speedup_samples > 0 ? total_speedup / speedup_samples : 1.0;
        
        thresholds.adjustThresholds(success_rate, avg_speedup);
    }
    
    // Print profiling statistics
    void printStatistics() const {
        std::cout << "\n=== Expression Profiler Statistics ===" << std::endl;
        std::cout << "Total profiles: " << total_profiles.load() << std::endl;
        std::cout << "JIT candidates: " << jit_candidates.load() << std::endl;
        std::cout << "Successful compilations: " << successful_compilations.load() << std::endl;
        std::cout << "Failed compilations: " << failed_compilations.load() << std::endl;
        
        uint64_t total_attempts = successful_compilations.load() + failed_compilations.load();
        if (total_attempts > 0) {
            double success_rate = static_cast<double>(successful_compilations.load()) / total_attempts * 100.0;
            std::cout << "Compilation success rate: " << std::fixed << std::setprecision(1) 
                      << success_rate << "%" << std::endl;
        }
        
        std::cout << std::endl;
        thresholds.printThresholds();
        
        // Print top expressions by execution count
        std::cout << "\nTop Expressions:" << std::endl;
        std::vector<std::pair<uint64_t, const ExpressionProfile*>> sorted_profiles;
        
        {
            std::lock_guard<std::mutex> lock(profiles_mutex);
            for (const auto& pair : profiles) {
                sorted_profiles.emplace_back(pair.second->execution_count.load(), pair.second.get());
            }
        }
        
        std::sort(sorted_profiles.begin(), sorted_profiles.end(), 
                 [](const auto& a, const auto& b) { return a.first > b.first; });
        
        for (size_t i = 0; i < std::min(size_t(5), sorted_profiles.size()); ++i) {
            const auto& profile = sorted_profiles[i].second;
            std::cout << "  #" << (i+1) << ": " << profile->execution_count.load() 
                      << " executions, avg: " << std::fixed << std::setprecision(1)
                      << profile->getAverageExecutionTime() << " ns, status: ";
            
            switch (profile->status.load()) {
                case ExpressionProfile::NOT_ANALYZED: std::cout << "Not Analyzed"; break;
                case ExpressionProfile::INTERPRETER_ONLY: std::cout << "Interpreter Only"; break;
                case ExpressionProfile::JIT_CANDIDATE: std::cout << "JIT Candidate"; break;
                case ExpressionProfile::JIT_COMPILED: std::cout << "JIT Compiled"; break;
                case ExpressionProfile::JIT_FAILED: std::cout << "JIT Failed"; break;
            }
            std::cout << std::endl;
        }
    }
    
private:
    ExpressionProfile* getOrCreateProfile(uint64_t fingerprint, const SimpleExpression& expr) {
        auto it = profiles.find(fingerprint);
        if (it != profiles.end()) {
            return it->second.get();
        }
        
        // Create new profile
        auto profile = std::make_unique<ExpressionProfile>();
        
        // Analyze expression characteristics
        ExpressionFingerprinter::analyzeComplexity(expr.instructions, 
                                                  profile->operation_count,
                                                  profile->depth, 
                                                  profile->has_complex_ops);
        
        ExpressionProfile* profile_ptr = profile.get();
        profiles[fingerprint] = std::move(profile);
        total_profiles.fetch_add(1);
        
        return profile_ptr;
    }
    
    void analyzeForCompilation(ExpressionProfile& profile, uint64_t fingerprint) {
        // Check compilation thresholds
        uint64_t count = profile.execution_count.load();
        double freq = profile.getExecutionFrequency();
        double avg_time = profile.getAverageExecutionTime();
        
        // Basic threshold checks
        if (count < thresholds.min_execution_count) {
            return; // Not enough executions yet
        }
        
        if (freq < thresholds.min_execution_frequency) {
            profile.status.store(ExpressionProfile::INTERPRETER_ONLY);
            return;
        }
        
        if (avg_time < thresholds.min_avg_execution_time_ns) {
            profile.status.store(ExpressionProfile::INTERPRETER_ONLY);
            return;
        }
        
        if (profile.operation_count > thresholds.max_complexity) {
            profile.status.store(ExpressionProfile::INTERPRETER_ONLY);
            return;
        }
        
        // Expression is a good JIT candidate
        profile.status.store(ExpressionProfile::JIT_CANDIDATE);
        jit_candidates.fetch_add(1);
        
        std::cout << "Expression " << std::hex << fingerprint << std::dec 
                  << " identified as JIT candidate (count=" << count 
                  << ", freq=" << std::fixed << std::setprecision(1) << freq << " Hz"
                  << ", avg_time=" << std::setprecision(0) << avg_time << " ns)" << std::endl;
    }
};

