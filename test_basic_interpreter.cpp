#include "MiniScript-cpp/src/MiniScript/MiniscriptInterpreter.h" 
#include <iostream>

using namespace MiniScript;

int main() {
    std::cout << "Testing basic interpreter functionality\n";
    
    try {
        String simpleCode = "print \"Hello World\"";
        
        Interpreter interp;
        interp.Reset(simpleCode);
        
        std::cout << "Code parsed successfully\n";
        
        interp.RunUntilDone();
        
        std::cout << "Execution completed\n";
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown exception occurred\n";
    }
    
    return 0;
}