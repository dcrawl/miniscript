# TAC Operation Categorization & Frequency Analysis

## Priority-Based Categorization

### 🔥 **CRITICAL PRIORITY** (Optimize First)

#### Arithmetic Operations (Very High Frequency)
```cpp
// Number arithmetic - EXTREMELY common in loops, calculations
Op::APlusB, Op::AMinusB, Op::ATimesB, Op::ADividedByB, Op::AModB, Op::APowB

// String concatenation - VERY common in string building
Op::APlusB (when operands are strings)
```

#### Assignment Operations (Very High Frequency) 
```cpp 
// Already fast-pathed but could be extracted
Op::AssignA, Op::AssignImplicit, Op::ReturnA, Op::CopyA, Op::BindAssignA
```

#### Container Access (High Frequency)
```cpp
// Map/List lookups - benefits from our Phase 1 map optimizations!
Op::ElemBofA, Op::ElemBofIterA, Op::LengthOfA
```

### 🎯 **HIGH PRIORITY** (Second Wave)

#### Comparison Operations (High Frequency)
```cpp
// Used extensively in conditionals, loops, sorting
Op::AEqualB, Op::ANotEqualB, Op::AGreaterThanB, Op::AGreatOrEqualB, 
Op::ALessThanB, Op::ALessOrEqualB, Op::AisaB
```

#### Control Flow (Medium Frequency, High Impact)
```cpp
// Jumps are expensive, optimization has high impact
Op::GotoA, Op::GotoAifB, Op::GotoAifTrulyB, Op::GotoAifNotB
```

### ⚡ **MEDIUM PRIORITY** (Polish Phase)

#### Logical Operations (Medium Frequency)
```cpp
Op::AAndB, Op::AOrB, Op::NotA
```

#### Function Operations (Lower Frequency, Variable Impact)
```cpp
Op::CallFunctionA, Op::CallIntrinsicA, Op::PushParam
```

### 🔧 **LOW PRIORITY** (Future Work)

#### Object Creation (Low Frequency)
```cpp
Op::NewA, Op::Noop
```

## Type-Operation Matrix Analysis

### Most Common Combinations (Hot Paths):

1. **Number × Number → Arithmetic** 
   - `APlusB`, `AMinusB`, `ATimesB`, `ADividedByB`
   - Current: Multiple type checks + switch
   - Optimization: Direct numeric operations

2. **String × String/Any → Concatenation**
   - `APlusB` with string operands
   - Current: Scattered in 3 places  
   - Optimization: Dedicated string concatenation

3. **Map/List × Any → Element Access**
   - `ElemBofA` for container[key/index]
   - Current: Type-specific branches
   - Optimization: Container-specialized access (leverages Phase 1!)

4. **Any × Any → Equality**
   - `AEqualB`, `ANotEqualB`
   - Current: Type-specific equality branches
   - Optimization: Type-specific equality functions

5. **Number → Control Flow**
   - `GotoA`, `GotoAifB` variants  
   - Current: Mixed with arithmetic
   - Optimization: Dedicated control flow handler

## Operation Frequency Estimation

Based on typical MiniScript code patterns:

### Very High (>20% of operations):
- **Assignment**: `AssignA`, `AssignImplicit` 
- **Arithmetic**: `APlusB`, `AMinusB` (numeric)
- **Element Access**: `ElemBofA` (map/list lookups)

### High (10-20% of operations):
- **Comparison**: `AEqualB`, `ANotEqualB`, greater/less than
- **String Concat**: `APlusB` (string operands)
- **Length**: `LengthOfA`

### Medium (5-10% of operations):  
- **Control Flow**: `GotoAifB` variants
- **Multiplication/Division**: `ATimesB`, `ADividedByB`
- **Logical**: `AAndB`, `AOrB`, `NotA`

### Lower (<5% of operations):
- **Function Calls**: `CallFunctionA`, `CallIntrinsicA` 
- **Object Creation**: `NewA`
- **Advanced Arithmetic**: `AModB`, `APowB`
- **Type Testing**: `AisaB`

## Optimization Strategy Ranking

### Phase 2.1a: **Critical Operations** (Week 1)
1. **ArithmeticEvaluator**: Handle numeric arithmetic hot path
2. **StringEvaluator**: Optimize string concatenation 
3. **ContainerEvaluator**: Leverage Phase 1 map optimizations

**Expected Impact**: 15-25% improvement (highest frequency operations)

### Phase 2.1b: **High Impact Operations** (Week 1-2)  
4. **ComparisonEvaluator**: Type-specific comparison optimization
5. **ControlFlowEvaluator**: Optimize branching overhead

**Expected Impact**: Additional 5-15% improvement

### Phase 2.1c: **Polish & Integration** (Week 2)
6. **LogicalEvaluator**: Logical operation optimization  
7. **FunctionEvaluator**: Complex function call handling
8. **Integration & Benchmarking**: Measure total improvements

**Expected Impact**: Additional 5-10% improvement + validation

## Key Insight: Synergy with Phase 1

Our Phase 1 map optimizations directly benefit the **Container Access** operations:

- `ElemBofA` (map[key] lookups) now use optimized hash tables
- Better load factors (0.75 vs 7.97) = faster element access
- This creates a **multiplicative benefit** - faster maps × specialized evaluators

**Combined Impact Potential**: Phase 1 + Phase 2.1 could yield 40-60% improvement on map-heavy workloads!

## Implementation Priority Order

1. **ArithmeticEvaluator** (numeric operations - highest frequency)
2. **StringEvaluator** (string concatenation - very common)  
3. **ContainerEvaluator** (leverages Phase 1 improvements)
4. **ComparisonEvaluator** (conditional logic optimization)
5. **ControlFlowEvaluator** (branching optimization)

This categorization provides the roadmap for our specialized evaluator implementation.