# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

MiniScript is a high-level object-oriented scripting language with implementations in both C# and C++. The project provides:
- Core language implementations (C# and C++)
- Command-line interpreter (`minicmd`)
- Shared libraries for embedding MiniScript in other applications
- Comprehensive test suite and integration tests

## Build System

The project uses CMake for cross-platform building.

### Basic Build Commands

```bash
# Create and enter build directory
mkdir -p build && cd build

# Configure with CMake
cmake ../..

# Build release version
cmake --build . --config Release

# Build with testing enabled
cmake -DMINISCRIPT_BUILD_TESTING=ON ../..
cmake --build . --config Release
```

### CMake Options

- `MINISCRIPT_BUILD_TESTING=ON` - Enable unit tests and integration tests
- `MINISCRIPT_BUILD_CSHARP=ON` - Build C# binaries (requires .NET)
- `MINISCRIPT_CMD_NAME` - Customize executable name (default: "miniscript")

## Testing

### Running Tests

```bash
# Run all tests (after building with MINISCRIPT_BUILD_TESTING=ON)
ctest

# For multi-config builds (Visual Studio, Xcode)
ctest -C Release

# Run specific test types
ctest -R "UnitTests"
ctest -R "Integration"

# Run manual integration test
./minicmd --itest ../TestSuite.txt
```

### Test Structure

- **Unit Tests**: C++ unit tests in `UnitTest.cpp` with `UNIT_TEST_MAIN` define
- **Integration Tests**: MiniScript files executed through the interpreter
- **TestSuite.txt**: Comprehensive integration test suite with expected outputs

## Architecture

### Core Components

**Language Engine** (`MiniScript-cpp/src/MiniScript/`):
- `MiniscriptInterpreter`: Main execution engine
- `MiniscriptLexer`: Tokenization
- `MiniscriptParser`: AST generation  
- `MiniscriptTAC`: Three-Address Code intermediate representation
- `MiniscriptTypes`: Core data types (Value, ValueDict, ValueList)
- `MiniscriptIntrinsics`: Built-in functions and operators

**Command-Line Host** (`MiniScript-cpp/src/`):
- `main.cpp`: Entry point for command-line interpreter
- `ShellIntrinsics.cpp`: File I/O, system calls, and shell-specific functions
- `DateTimeUtils`: Date/time functionality
- `Key.cpp`: Keyboard input handling
- `editline/`: Command-line editing library

**Data Structures**:
- `SimpleString`: Custom string implementation
- `SimpleVector`: Dynamic array implementation  
- `Dictionary`: Hash table for maps/objects
- `List`: Dynamic list implementation
- `RefCountedStorage`: Memory management

### Language Features

- **Dynamic typing** with Value wrapper class
- **Object orientation** via prototype-based inheritance (`__isa` chain)
- **Function references** using `@` prefix (e.g., `@functionName`)
- **Inclusive ranges** - `range(start, stop)` includes both endpoints
- **Map/dictionary syntax** with both `map["key"]` and `map.key` access
- **Built-in intrinsics** for strings, lists, math, I/O

### Memory Management

Uses reference counting via `RefCountedStorage` for automatic memory management of:
- Strings (when longer than inline threshold)
- Lists and dictionaries
- User-defined objects

## Development Workflow

### Adding New Intrinsic Functions

1. Declare in `MiniscriptIntrinsics.h`
2. Implement in `MiniscriptIntrinsics.cpp` 
3. Register in appropriate intrinsic setup function
4. Add tests to verify functionality

### Modifying Language Grammar

1. Update lexer tokens in `MiniscriptLexer.cpp`
2. Modify parser rules in `MiniscriptParser.cpp`
3. Update TAC generation in `MiniscriptTAC.cpp`
4. Handle execution in `MiniscriptInterpreter.cpp`

### Cross-Platform Considerations

- Use CMake feature detection for platform-specific code
- Windows builds exclude editline (command-line editing)
- File paths use forward slashes internally, converted as needed
- Environment variable access varies by platform

## Important Files

- `TestSuite.txt`: Comprehensive test cases with expected outputs
- `QuickRef.md`: Language reference documentation
- `lib/`: Importable MiniScript modules (found via `MS_IMPORT_PATH`)
- `MiniScript-cpp/README.md`: C++ specific build instructions

## Installation

After building, the interpreter can be installed by:
1. Copying/linking `miniscript` executable to PATH
2. Placing `lib/` folder next to executable or setting `MS_IMPORT_PATH`

The `lib` folder contains standard library modules accessible via `import` statement.