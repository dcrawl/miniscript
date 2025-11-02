# Specialized Evaluators Architecture Design

## Overview

This design splits the monolithic `TACLine::Evaluate` function into specialized, optimized evaluators for different operation categories. Each evaluator focuses on specific operation types and value type combinations for maximum performance.

## Core Architecture

### Main Dispatcher (FastPath Router)

```cpp
// In MiniscriptTAC.cpp - New optimized entry point
namespace OptimizedTAC {
    
    Value EvaluateFast(const TACLine* line, Context* context) {
        // Fast path for most common operations
        const Op op = line->op;
        
        // Critical Path 1: Assignment operations (pre-optimized)
        if (op == Op::AssignA || op == Op::ReturnA || op == Op::AssignImplicit) {
            return AssignmentEvaluator::Evaluate(line, context);
        }
        
        // Get operands once (avoid repeated evaluations)
        Value opA = line->rhsA.type == ValueType::Null ? line->rhsA : line->rhsA.Val(context);
        Value opB = line->rhsB.type == ValueType::Null ? line->rhsB : line->rhsB.Val(context);
        
        // Critical Path 2: Arithmetic hot paths
        if (IsArithmeticOp(op)) {
            return ArithmeticEvaluator::Evaluate(op, opA, opB, context);
        }
        
        // Critical Path 3: Container access (leverages Phase 1 optimizations)
        if (IsContainerOp(op)) {
            return ContainerEvaluator::Evaluate(op, opA, opB, context);
        }
        
        // Critical Path 4: Comparison operations
        if (IsComparisonOp(op)) {
            return ComparisonEvaluator::Evaluate(op, opA, opB, context);
        }
        
        // Less frequent operations
        if (IsControlFlowOp(op)) {
            return ControlFlowEvaluator::Evaluate(op, opA, opB, context);
        }
        
        if (IsLogicalOp(op)) {
            return LogicalEvaluator::Evaluate(op, opA, opB, context);
        }
        
        if (IsFunctionOp(op)) {
            return FunctionEvaluator::Evaluate(line, context);
        }
        
        // Fallback to original implementation for edge cases
        return line->EvaluateOriginal(context);
    }
    
    // Fast operation type checks (bit operations)
    inline bool IsArithmeticOp(Op op) {
        return (op >= Op::APlusB && op <= Op::APowB);
    }
    
    inline bool IsComparisonOp(Op op) {
        return (op >= Op::AEqualB && op <= Op::AisaB) || 
               (op >= Op::AGreaterThanB && op <= Op::ALessOrEqualB);
    }
    
    inline bool IsContainerOp(Op op) {
        return (op == Op::ElemBofA || op == Op::ElemBofIterA || op == Op::LengthOfA);
    }
    
    inline bool IsControlFlowOp(Op op) {
        return (op >= Op::GotoA && op <= Op::GotoAifNotB);
    }
    
    inline bool IsLogicalOp(Op op) {
        return (op == Op::AAndB || op == Op::AOrB || op == Op::NotA);
    }
    
    inline bool IsFunctionOp(Op op) {
        return (op == Op::CallFunctionA || op == Op::CallIntrinsicA || op == Op::PushParam);
    }
}
```

## Specialized Evaluator Classes

### 1. ArithmeticEvaluator (Highest Priority)

```cpp
class ArithmeticEvaluator {
public:
    static Value Evaluate(Op op, const Value& opA, const Value& opB, Context* context) {
        // Fast type-based dispatch
        if (opA.type == ValueType::Number) {
            if (opB.type == ValueType::Number || opB.IsNull()) {
                return EvaluateNumeric(op, opA.data.number, opB.IsNull() ? 0.0 : opB.data.number);
            }
            // Number + String = String concatenation (common case)
            if (op == Op::APlusB && opB.type == ValueType::String) {
                return EvaluateNumberStringConcat(opA.data.number, opB.GetString());
            }
        }
        else if (opA.type == ValueType::String) {
            return StringEvaluator::EvaluateStringArithmetic(op, opA.GetString(), opB);
        }
        
        // Fallback for mixed types
        return EvaluateMixed(op, opA, opB);
    }

private:
    // Optimized numeric arithmetic (no type checking overhead)
    static Value EvaluateNumeric(Op op, double a, double b) {
        switch (op) {
            case Op::APlusB:    return Value(a + b);
            case Op::AMinusB:   return Value(a - b);
            case Op::ATimesB:   return Value(a * b);
            case Op::ADividedByB: return Value(a / b);
            case Op::AModB:     return Value(fmod(a, b));
            case Op::APowB:     return Value(pow(a, b));
            default:            return Value::null; // Should not happen
        }
    }
    
    // Optimized number-to-string concatenation
    static Value EvaluateNumberStringConcat(double num, const String& str) {
        String numStr = Value(num).ToString();
        if (numStr.LengthB() + str.LengthB() > Value::maxStringSize) {
            LimitExceededException("string too large").raise();
        }
        return Value(numStr + str);
    }
    
    static Value EvaluateMixed(Op op, const Value& opA, const Value& opB);
};
```

### 2. StringEvaluator (High Priority)

```cpp
class StringEvaluator {
public:
    static Value EvaluateStringArithmetic(Op op, const String& sA, const Value& opB) {
        if (op == Op::APlusB) {
            // String concatenation - very common operation
            if (opB.IsNull()) return Value(sA);
            String sB = opB.ToString();
            if (sA.LengthB() + sB.LengthB() > Value::maxStringSize) {
                LimitExceededException("string too large").raise();
            }
            return Value(sA + sB);
        }
        else if (op == Op::AMinusB) {
            // String suffix removal
            if (opB.IsNull()) return Value(sA);
            String sB = opB.ToString();
            if (sA.EndsWith(sB)) {
                return Value(sA.SubstringB(0, sA.LengthB() - sB.LengthB()));
            }
            return Value(sA);
        }
        
        // Other string operations fallback
        return EvaluateStringFallback(op, sA, opB);
    }

private:
    static Value EvaluateStringFallback(Op op, const String& sA, const Value& opB);
};
```

### 3. ContainerEvaluator (Leverages Phase 1 Optimizations)

```cpp
class ContainerEvaluator {
public:
    static Value Evaluate(Op op, const Value& opA, const Value& opB, Context* context) {
        switch (op) {
            case Op::ElemBofA:
            case Op::ElemBofIterA:
                return EvaluateElementAccess(opA, opB);
                
            case Op::LengthOfA:
                return EvaluateLength(opA);
                
            default:
                return Value::null; // Should not happen
        }
    }

private:
    // Optimized element access (benefits from Phase 1 map optimizations!)
    static Value EvaluateElementAccess(const Value& container, const Value& index) {
        if (container.type == ValueType::List) {
            return container.GetElem(index);
        }
        else if (container.type == ValueType::Map) {
            // This now uses our optimized dynamic hash tables from Phase 1!
            return container.GetElem(index);
        }
        else if (container.type == ValueType::String && index.type == ValueType::String) {
            // String lookup optimization
            return Value::Resolve(container, index.ToString(), nullptr, nullptr);
        }
        
        // Fallback
        return container.GetElem(index);
    }
    
    static Value EvaluateLength(const Value& container) {
        switch (container.type) {
            case ValueType::List:
                return Value(container.GetList().Count());
            case ValueType::Map:
                return Value(container.GetDict().Count());
            case ValueType::String:
                return Value(container.GetString().Length());
            default:
                RuntimeException("length requires a string, list, or map").raise();
                return Value::null;
        }
    }
};
```

### 4. ComparisonEvaluator (High Priority)

```cpp
class ComparisonEvaluator {
public:
    static Value Evaluate(Op op, const Value& opA, const Value& opB, Context* context) {
        // Special cases for null comparisons (very common)
        if (op == Op::AEqualB || op == Op::ANotEqualB) {
            if (opA.IsNull() || opB.IsNull()) {
                bool equal = (opA == opB);
                return Value::Truth(op == Op::AEqualB ? equal : !equal);
            }
        }
        
        // Type-specific optimizations
        if (opA.type == opB.type) {
            switch (opA.type) {
                case ValueType::Number:
                    return EvaluateNumericComparison(op, opA.data.number, opB.data.number);
                case ValueType::String:
                    return EvaluateStringComparison(op, opA.GetString(), opB.GetString());
                case ValueType::List:
                case ValueType::Map:
                    return EvaluateContainerComparison(op, opA, opB);
            }
        }
        
        // Mixed type comparisons
        return EvaluateMixedComparison(op, opA, opB, context);
    }

private:
    static Value EvaluateNumericComparison(Op op, double a, double b) {
        switch (op) {
            case Op::AEqualB:        return Value::Truth(a == b);
            case Op::ANotEqualB:     return Value::Truth(a != b);
            case Op::AGreaterThanB:  return Value::Truth(a > b);
            case Op::AGreatOrEqualB: return Value::Truth(a >= b);
            case Op::ALessThanB:     return Value::Truth(a < b);
            case Op::ALessOrEqualB:  return Value::Truth(a <= b);
            default:                 return Value::null;
        }
    }
    
    static Value EvaluateStringComparison(Op op, const String& sA, const String& sB);
    static Value EvaluateContainerComparison(Op op, const Value& opA, const Value& opB);
    static Value EvaluateMixedComparison(Op op, const Value& opA, const Value& opB, Context* context);
};
```

### 5. ControlFlowEvaluator 

```cpp
class ControlFlowEvaluator {
public:
    static Value Evaluate(Op op, const Value& opA, const Value& opB, Context* context) {
        if (opA.type != ValueType::Number) {
            RuntimeException("control flow target must be a number").raise();
        }
        
        int target = (int)opA.data.number;
        
        switch (op) {
            case Op::GotoA:
                context->lineNum = target;
                break;
                
            case Op::GotoAifB:
                if (!opB.IsNull() && opB.BoolValue()) {
                    context->lineNum = target;
                }
                break;
                
            case Op::GotoAifTrulyB: {
                // Truly true (integer != 0)
                long i = opB.IsNull() ? 0 : opB.IntValue();
                if (i != 0) context->lineNum = target;
                break;
            }
            
            case Op::GotoAifNotB:
                if (opB.IsNull() || !opB.BoolValue()) {
                    context->lineNum = target;
                }
                break;
        }
        
        return Value::null;
    }
};
```

### 6. AssignmentEvaluator (Already Fast-Pathed)

```cpp
class AssignmentEvaluator {
public:
    static Value Evaluate(const TACLine* line, Context* context) {
        const Op op = line->op;
        
        if (op == Op::AssignA || op == Op::ReturnA || op == Op::AssignImplicit) {
            // Existing optimized assignment logic
            const Value& rhsA = line->rhsA;
            if (rhsA.type == ValueType::List || rhsA.type == ValueType::Map) {
                return rhsA.FullEval(context);
            } else if (rhsA.IsNull()) {
                return Value::null;
            } else {
                return rhsA.Val(context);
            }
        }
        else if (op == Op::CopyA) {
            // Literal copy optimization
            return line->rhsA.EvalCopy(context);
        }
        
        return Value::null;
    }
};
```

## Integration Strategy

### Phase 2.1a: Core Implementation (Week 1)
1. **ArithmeticEvaluator**: Implement numeric operations optimization
2. **StringEvaluator**: Implement string concatenation optimization  
3. **Basic Dispatcher**: Route to specialized evaluators

### Phase 2.1b: Container & Comparison (Week 1-2)
4. **ContainerEvaluator**: Leverage Phase 1 map optimizations
5. **ComparisonEvaluator**: Type-specific comparison optimization
6. **Enhanced Dispatcher**: Add remaining operation routing

### Phase 2.1c: Polish & Validation (Week 2)
7. **ControlFlowEvaluator**: Control flow optimization
8. **Performance Benchmarking**: Measure improvements
9. **Fallback Safety**: Ensure original behavior for edge cases

## Expected Performance Benefits

### Micro-optimizations:
- **Reduced Type Checking**: 10-15% (eliminate redundant type checks)
- **Better Branch Prediction**: 5-10% (simpler control flow)
- **Cache Optimization**: 5-10% (related operations grouped)

### Macro-optimizations:
- **Arithmetic Hot Path**: 15-25% (direct numeric operations)
- **Container Access**: 10-20% (synergy with Phase 1 improvements)
- **String Operations**: 10-15% (optimized concatenation)

### Total Expected Impact: 20-35% interpreter performance improvement

This architecture provides a clean, maintainable foundation for our optimization work while preserving backward compatibility through fallback mechanisms.