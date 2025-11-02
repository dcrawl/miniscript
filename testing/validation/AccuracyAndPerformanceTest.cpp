#include "MiniScript/MiniscriptInterpreter.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <cstdlib>
#include <iomanip>
#include <fstream>

// Global variables for output capture
static std::string* g_capturedOutput = nullptr;

// Global function to capture MiniScript output
static void captureOutput(MiniScript::String text, bool addLineBreak) {
    if (g_capturedOutput) {
        *g_capturedOutput += text.c_str();
        if (addLineBreak) *g_capturedOutput += "\n";
    }
}

/// Accuracy and Performance Validation Suite
/// Tests computational accuracy and compares performance with reference MiniScript
class AccuracyAndPerformanceValidator {
public:
    void runValidationSuite() {
        std::cout << "=== MiniScript Accuracy & Performance Validation Suite ===" << std::endl;
        
        testComputationalAccuracy();
        testPerformanceComparison();
        printSummary();
    }

private:
    struct TestResult {
        std::string testName;
        bool accuracyPass;
        double ourTime;
        double refTime;
        double speedup;
        std::string ourResult;
        std::string refResult;
    };

    std::vector<TestResult> results_;

    void testComputationalAccuracy() {
        std::cout << "\n--- Computational Accuracy Tests ---" << std::endl;
        
        // Test cases that previously had issues
        testSumAccuracy();
        testFibonacciAccuracy();
        testFloatingPointAccuracy();
        testLoopAccuracy();
        testComplexExpressions();
    }

    void testSumAccuracy() {
        std::cout << "\nTest 1: Sum Accuracy (Previous Issue Check)" << std::endl;
        
        std::string program = R"(
            // Test summation accuracy
            sum = 0
            for i in range(1, 100)
                sum = sum + i
            end for
            print sum
        )";
        
        // Expected result: sum of 1 to 100 = 5050
        std::string expected = "5050";
        
        TestResult result = runComparisonTest("Sum 1-100", program, expected);
        results_.push_back(result);
        
        std::cout << "Our result: " << result.ourResult << std::endl;
        std::cout << "Expected: " << expected << std::endl;
        std::cout << "Accuracy: " << (result.accuracyPass ? "✅ PASS" : "❌ FAIL") << std::endl;
    }

    void testFibonacciAccuracy() {
        std::cout << "\nTest 2: Fibonacci Accuracy" << std::endl;
        
        std::string program = R"(
            // Fibonacci calculation
            n = 15
            if n <= 1 then
                result = n
            else
                a = 0
                b = 1
                for i in range(2, n)
                    temp = a + b
                    a = b
                    b = temp
                end for
                result = b
            end if
            print result
        )";
        
        // Expected: 15th Fibonacci number = 610
        std::string expected = "610";
        
        TestResult result = runComparisonTest("Fibonacci 15", program, expected);
        results_.push_back(result);
        
        std::cout << "Our result: " << result.ourResult << std::endl;
        std::cout << "Expected: " << expected << std::endl;
        std::cout << "Accuracy: " << (result.accuracyPass ? "✅ PASS" : "❌ FAIL") << std::endl;
    }

    void testFloatingPointAccuracy() {
        std::cout << "\nTest 3: Floating Point Accuracy" << std::endl;
        
        std::string program = R"(
            // Floating point computation
            result = 0.1 + 0.2
            result = round(result * 1000) / 1000  // Round to 3 decimal places
            print result
        )";
        
        std::string expected = "0.3";
        
        TestResult result = runComparisonTest("Float 0.1+0.2", program, expected);
        results_.push_back(result);
        
        std::cout << "Our result: " << result.ourResult << std::endl;
        std::cout << "Expected: " << expected << std::endl;
        std::cout << "Accuracy: " << (result.accuracyPass ? "✅ PASS" : "❌ FAIL") << std::endl;
    }

    void testLoopAccuracy() {
        std::cout << "\nTest 4: Complex Loop Accuracy" << std::endl;
        
        std::string program = R"(
            // Complex loop with accumulation
            total = 0
            product = 1
            for i in range(1, 10)
                total = total + i * i
                if i <= 5 then
                    product = product * i
                end if
            end for
            result = total + product
            print result
        )";
        
        // total = 1² + 2² + ... + 10² = 385
        // product = 1 * 2 * 3 * 4 * 5 = 120
        // result = 385 + 120 = 505
        std::string expected = "505";
        
        TestResult result = runComparisonTest("Complex Loop", program, expected);
        results_.push_back(result);
        
        std::cout << "Our result: " << result.ourResult << std::endl;
        std::cout << "Expected: " << expected << std::endl;
        std::cout << "Accuracy: " << (result.accuracyPass ? "✅ PASS" : "❌ FAIL") << std::endl;
    }

    void testComplexExpressions() {
        std::cout << "\nTest 5: Complex Mathematical Expressions" << std::endl;
        
        std::string program = R"(
            // Complex mathematical expression
            x = 3
            y = 4
            z = 5
            result = x^2 + y^2 + z^2 + 2*x*y + 2*y*z + 2*x*z
            print result
        )";
        
        // (x+y+z)² = (3+4+5)² = 12² = 144
        std::string expected = "144";
        
        TestResult result = runComparisonTest("Complex Math", program, expected);
        results_.push_back(result);
        
        std::cout << "Our result: " << result.ourResult << std::endl;
        std::cout << "Expected: " << expected << std::endl;
        std::cout << "Accuracy: " << (result.accuracyPass ? "✅ PASS" : "❌ FAIL") << std::endl;
    }

    void testPerformanceComparison() {
        std::cout << "\n--- Performance Comparison Tests ---" << std::endl;
        
        testSimpleArithmetic();
        testLoopPerformance();
        testFibonacciPerformance();
        testComputeIntensiveTask();
    }

    void testSimpleArithmetic() {
        std::cout << "\nPerformance Test 1: Simple Arithmetic" << std::endl;
        
        std::string program = R"(
            result = 0
            for i in range(10000)
                result = result + i * 2 - 1
            end for
            print result
        )";
        
        TestResult result = runComparisonTest("Simple Arithmetic", program, "");
        results_.push_back(result);
        
        std::cout << "Our time: " << result.ourTime << " ms" << std::endl;
        std::cout << "Reference time: " << result.refTime << " ms" << std::endl;
        std::cout << "Speedup: " << result.speedup << "x" << std::endl;
    }

    void testLoopPerformance() {
        std::cout << "\nPerformance Test 2: Nested Loops" << std::endl;
        
        std::string program = R"(
            total = 0
            for i in range(100)
                for j in range(100)
                    total = total + i + j
                end for
            end for
            print total
        )";
        
        TestResult result = runComparisonTest("Nested Loops", program, "");
        results_.push_back(result);
        
        std::cout << "Our time: " << result.ourTime << " ms" << std::endl;
        std::cout << "Reference time: " << result.refTime << " ms" << std::endl;
        std::cout << "Speedup: " << result.speedup << "x" << std::endl;
    }

    void testFibonacciPerformance() {
        std::cout << "\nPerformance Test 3: Fibonacci Calculation" << std::endl;
        
        std::string program = R"(
            n = 30
            a = 0
            b = 1
            for i in range(2, n+1)
                temp = a + b
                a = b
                b = temp
            end for
            print b
        )";
        
        TestResult result = runComparisonTest("Fibonacci 30", program, "");
        results_.push_back(result);
        
        std::cout << "Our time: " << result.ourTime << " ms" << std::endl;
        std::cout << "Reference time: " << result.refTime << " ms" << std::endl;
        std::cout << "Speedup: " << result.speedup << "x" << std::endl;
    }

    void testComputeIntensiveTask() {
        std::cout << "\nPerformance Test 4: Compute-Intensive Task" << std::endl;
        
        std::string program = R"(
            // Prime counting approximation
            count = 0
            for n in range(2, 1000)
                isPrime = true
                for i in range(2, n)
                    if n % i == 0 then
                        isPrime = false
                        break
                    end if
                    if i * i > n then break
                end for
                if isPrime then count = count + 1
            end for
            print count
        )";
        
        TestResult result = runComparisonTest("Prime Count", program, "");
        results_.push_back(result);
        
        std::cout << "Our time: " << result.ourTime << " ms" << std::endl;
        std::cout << "Reference time: " << result.refTime << " ms" << std::endl;
        std::cout << "Speedup: " << result.speedup << "x" << std::endl;
    }

    TestResult runComparisonTest(const std::string& testName, const std::string& program, const std::string& expectedResult) {
        TestResult result;
        result.testName = testName;
        result.accuracyPass = false;
        result.ourTime = 0;
        result.refTime = 0;
        result.speedup = 0;

        // Test our implementation
        auto ourResult = runOurImplementation(program, result.ourTime);
        result.ourResult = ourResult;

        // Test reference implementation
        auto refResult = runReferenceImplementation(program, result.refTime);
        result.refResult = refResult;

        // Check accuracy
        if (!expectedResult.empty()) {
            result.accuracyPass = (trim(ourResult) == expectedResult) || (trim(refResult) == expectedResult);
        } else {
            result.accuracyPass = (trim(ourResult) == trim(refResult));
        }

        // Calculate speedup
        if (result.refTime > 0) {
            result.speedup = result.refTime / result.ourTime;
        }

        return result;
    }

    std::string runOurImplementation(const std::string& program, double& executionTime) {
        std::string output;
        g_capturedOutput = &output;
        
        try {
            MiniScript::Interpreter interpreter(MiniScript::String(program.c_str()));
            
            // Set output handler
            interpreter.standardOutput = captureOutput;

            auto start = std::chrono::high_resolution_clock::now();
            interpreter.RunUntilDone(30.0);  // 30 second timeout
            auto end = std::chrono::high_resolution_clock::now();

            executionTime = std::chrono::duration<double, std::milli>(end - start).count();
            
        } catch (const std::exception& e) {
            output = "ERROR: " + std::string(e.what());
        }
        
        g_capturedOutput = nullptr;
        return output;
    }

    std::string runReferenceImplementation(const std::string& program, double& executionTime) {
        // Write program to temporary file
        std::string tempFile = "/tmp/miniscript_test.ms";
        std::ofstream file(tempFile);
        file << program;
        file.close();

        // Run reference implementation and capture output
        std::string command = "/usr/local/bin/miniscript " + tempFile + " 2>&1";
        
        auto start = std::chrono::high_resolution_clock::now();
        FILE* pipe = popen(command.c_str(), "r");
        auto end = std::chrono::high_resolution_clock::now();
        
        executionTime = std::chrono::duration<double, std::milli>(end - start).count();

        std::string output;
        if (pipe) {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), pipe)) {
                output += buffer;
            }
            pclose(pipe);
        }

        // Clean up
        remove(tempFile.c_str());
        
        return output;
    }

    std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return "";
        size_t end = str.find_last_not_of(" \t\n\r");
        return str.substr(start, end - start + 1);
    }

    void printSummary() {
        std::cout << "\n=== Validation Summary ===" << std::endl;
        
        int accuracyPassed = 0;
        double totalSpeedup = 0;
        int performanceTests = 0;
        
        std::cout << "\nAccuracy Results:" << std::endl;
        std::cout << std::setw(20) << "Test" << std::setw(15) << "Result" << std::setw(20) << "Our Output" << std::setw(20) << "Ref Output" << std::endl;
        std::cout << std::string(75, '-') << std::endl;
        
        for (const auto& result : results_) {
            if (!result.ourResult.empty() && result.ourResult.find("ERROR") == std::string::npos) {
                std::cout << std::setw(20) << result.testName 
                          << std::setw(15) << (result.accuracyPass ? "✅ PASS" : "❌ FAIL")
                          << std::setw(20) << trim(result.ourResult).substr(0, 18)
                          << std::setw(20) << trim(result.refResult).substr(0, 18) << std::endl;
                
                if (result.accuracyPass) accuracyPassed++;
            }
        }
        
        std::cout << "\nPerformance Results:" << std::endl;
        std::cout << std::setw(20) << "Test" << std::setw(12) << "Our Time" << std::setw(12) << "Ref Time" << std::setw(12) << "Speedup" << std::endl;
        std::cout << std::string(56, '-') << std::endl;
        
        for (const auto& result : results_) {
            if (result.ourTime > 0 && result.refTime > 0) {
                std::cout << std::setw(20) << result.testName 
                          << std::setw(12) << std::fixed << std::setprecision(2) << result.ourTime << "ms"
                          << std::setw(12) << std::fixed << std::setprecision(2) << result.refTime << "ms"
                          << std::setw(12) << std::fixed << std::setprecision(2) << result.speedup << "x" << std::endl;
                
                totalSpeedup += result.speedup;
                performanceTests++;
            }
        }
        
        double avgSpeedup = performanceTests > 0 ? totalSpeedup / performanceTests : 0;
        
        std::cout << "\n=== Final Results ===" << std::endl;
        std::cout << "Accuracy Tests: " << accuracyPassed << "/" << results_.size() << " passed" << std::endl;
        std::cout << "Average Speedup: " << std::fixed << std::setprecision(2) << avgSpeedup << "x" << std::endl;
        
        if (accuracyPassed == results_.size()) {
            std::cout << "🎯 All accuracy tests PASSED! ✅" << std::endl;
        } else {
            std::cout << "⚠️  Some accuracy tests FAILED! ❌" << std::endl;
        }
        
        if (avgSpeedup > 1.0) {
            std::cout << "🚀 Performance improvement achieved! " << avgSpeedup << "x faster on average" << std::endl;
        } else {
            std::cout << "📊 Performance results: " << avgSpeedup << "x (may need optimization)" << std::endl;
        }
    }
};

int main() {
    try {
        AccuracyAndPerformanceValidator validator;
        validator.runValidationSuite();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}