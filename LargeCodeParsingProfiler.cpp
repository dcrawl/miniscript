//
// LargeCodeParsingProfiler.cpp  
// Test parsing performance with larger programs
//

#include <chrono>
#include <iostream>
#include "MiniscriptLexer.h"
#include "MiniscriptParser.h"

using namespace MiniScript;
using namespace std::chrono;

const String LARGE_TEST_CODE = 
"// Large MiniScript program to test parsing scalability\n"
"globals.data = {}\n"
"globals.cache = {}\n"
"globals.config = {\"debug\": true, \"timeout\": 30, \"retries\": 3}\n"
"\n"
"fibonacci = function(n)\n"
"  if n <= 1 then return n\n"
"  if globals.cache.hasIndex(n) then return globals.cache[n]\n"
"  result = fibonacci(n-1) + fibonacci(n-2)\n"
"  globals.cache[n] = result\n"
"  return result\n"
"end function\n"
"\n"
"processData = function(items)\n"
"  results = []\n"
"  for i in range(items.len)\n"
"    item = items[i]\n"
"    if item isa number then\n"
"      processed = item * 2 + 1\n"
"    else if item isa string then\n"
"      processed = item.upper + \"_PROCESSED\"\n"
"    else if item isa list then\n"
"      processed = []\n"
"      for subitem in item\n"
"        processed.push(str(subitem))\n"
"      end for\n"
"    else\n"
"      processed = str(item)\n"
"    end if\n"
"    results.push(processed)\n"
"  end for\n"
"  return results\n"
"end function\n"
"\n"
"validateConfig = function(config)\n"
"  required = [\"debug\", \"timeout\", \"retries\"]\n"
"  for key in required\n"
"    if not config.hasIndex(key) then\n"
"      print \"Missing required config: \" + key\n"
"      return false\n"
"    end if\n"
"  end for\n"
"  if config.timeout < 1 or config.timeout > 300 then\n"
"    print \"Invalid timeout: \" + config.timeout\n"
"    return false\n"
"  end if\n"
"  if config.retries < 0 or config.retries > 10 then\n"
"    print \"Invalid retries: \" + config.retries\n"
"    return false\n"
"  end if\n"
"  return true\n"
"end function\n"
"\n"
"// Main execution\n"
"if not validateConfig(globals.config) then\n"
"  print \"Configuration validation failed\"\n"
"  exit\n"
"end if\n"
"\n"
"testData = [1, 2, \"hello\", [3, 4, 5], {\"nested\": true}, 42]\n"
"processed = processData(testData)\n"
"\n"
"print \"Processing results:\"\n"
"for i in range(processed.len)\n"
"  print \"  [\" + i + \"]: \" + processed[i]\n"
"end for\n"
"\n"
"print \"Fibonacci sequence:\"\n"
"for i in range(10)\n"
"  fib = fibonacci(i)\n"
"  print \"  F(\" + i + \") = \" + fib\n"
"end for\n"
"\n"
"// Performance test loop\n"
"iterations = 1000\n"
"sum = 0\n"
"startTime = time\n"
"for i in range(iterations)\n"
"  for j in range(100)\n"
"    sum = sum + i * j + fibonacci(j % 8)\n"
"  end for\n"
"end for\n"
"endTime = time\n"
"print \"Performance test completed in \" + (endTime - startTime) + \" seconds\"\n"
"print \"Final sum: \" + sum\n";

int main() {
    std::cout << "\n=== Phase 2.2 Large Code Parsing Analysis ===" << std::endl;
    std::cout << "Test code size: " << LARGE_TEST_CODE.LengthB() << " bytes" << std::endl;
    std::cout << "Test code lines: ~90 lines" << std::endl;
    
    // Test 1: Small vs Large Code Lexing
    const String SMALL_CODE = "sum = 0\nfor i in range(10000)\n  sum = sum + i * 2\nend for\nprint sum";
    
    // Small code lexing
    const int iterations = 100;
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        Lexer lexer(SMALL_CODE);
        while (!lexer.atEnd()) {
            Token token = lexer.Dequeue();
            volatile auto preventOpt = token.type;
        }
    }
    
    auto end = high_resolution_clock::now();
    double smallLexingTime = duration_cast<microseconds>(end - start).count() / 1000.0 / iterations;
    
    // Large code lexing
    start = high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        Lexer lexer(LARGE_TEST_CODE);
        while (!lexer.atEnd()) {
            Token token = lexer.Dequeue();
            volatile auto preventOpt = token.type;
        }
    }
    
    end = high_resolution_clock::now();
    double largeLexingTime = duration_cast<microseconds>(end - start).count() / 1000.0 / iterations;
    
    // Small code parsing
    start = high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        Parser parser;
        parser.Parse(SMALL_CODE);
        volatile auto preventOpt = parser.outputStack.Count();
    }
    
    end = high_resolution_clock::now();
    double smallParsingTime = duration_cast<microseconds>(end - start).count() / 1000.0 / iterations;
    
    // Large code parsing
    start = high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        Parser parser;
        parser.Parse(LARGE_TEST_CODE);
        volatile auto preventOpt = parser.outputStack.Count();
    }
    
    end = high_resolution_clock::now();
    double largeParsingTime = duration_cast<microseconds>(end - start).count() / 1000.0 / iterations;
    
    // Results
    std::cout << "\n=== Scalability Analysis ===" << std::endl;
    std::cout << "Small Code (5 lines):" << std::endl;
    std::cout << "  Lexing: " << smallLexingTime << " ms" << std::endl;
    std::cout << "  Parsing: " << smallParsingTime << " ms" << std::endl;
    
    std::cout << "Large Code (~90 lines):" << std::endl;
    std::cout << "  Lexing: " << largeLexingTime << " ms" << std::endl;
    std::cout << "  Parsing: " << largeParsingTime << " ms" << std::endl;
    
    double sizeRatio = (double)LARGE_TEST_CODE.LengthB() / SMALL_CODE.LengthB();
    double lexingRatio = largeLexingTime / smallLexingTime;
    double parsingRatio = largeParsingTime / smallParsingTime;
    
    std::cout << "\n=== Scaling Characteristics ===" << std::endl;
    std::cout << "Size ratio: " << sizeRatio << "x larger" << std::endl;
    std::cout << "Lexing scaling: " << lexingRatio << "x slower (" << (lexingRatio/sizeRatio) << " efficiency)" << std::endl;
    std::cout << "Parsing scaling: " << parsingRatio << "x slower (" << (parsingRatio/sizeRatio) << " efficiency)" << std::endl;
    
    if (lexingRatio > sizeRatio * 1.2) {
        std::cout << "⚠️  LEXING SCALING ISSUE: Non-linear performance degradation" << std::endl;
    }
    if (parsingRatio > sizeRatio * 1.2) {
        std::cout << "⚠️  PARSING SCALING ISSUE: Non-linear performance degradation" << std::endl;
    }
    
    // Recommendations
    std::cout << "\n=== Phase 2.2 Strategy Recommendations ===" << std::endl;
    
    if (largeParsingTime > 1.0) {  // More than 1ms for large programs
        std::cout << "🎯 PARSING OPTIMIZATION RECOMMENDED for large programs" << std::endl;
        std::cout << "   Large programs take " << largeParsingTime << " ms to parse" << std::endl;
        std::cout << "   This could impact IDE responsiveness and startup time" << std::endl;
        
        if (largeLexingTime > largeParsingTime - largeLexingTime) {
            std::cout << "   Priority 1: LEXER optimization (bulk tokenization)" << std::endl;
            std::cout << "   Priority 2: PARSER optimization (AST pooling)" << std::endl;
        } else {
            std::cout << "   Priority 1: PARSER optimization (AST pooling)" << std::endl;
            std::cout << "   Priority 2: LEXER optimization (bulk tokenization)" << std::endl;
        }
    } else {
        std::cout << "ℹ️  PARSING PERFORMANCE ACCEPTABLE for current use cases" << std::endl;
        std::cout << "   Focus Phase 2.2 on I/O and startup optimization instead" << std::endl;
    }
    
    return 0;
}